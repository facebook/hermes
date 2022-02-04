/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use super::generated_cvt::cvt_node_ptr;
use crate::ast;
use crate::ast::SourceId;
use hermes::parser::{
    DataRef, HermesParser, NodeLabel, NodeLabelOpt, NodeListOptRef, NodeListRef, NodePtr,
    NodePtrOpt, NodeString, NodeStringOpt, SMLoc,
};
use hermes::utf::{
    is_utf8_continuation, utf8_with_surrogates_to_string, utf8_with_surrogates_to_utf16,
};
use juno_support::atom_table;
use std::collections::HashMap;
use std::convert::TryFrom;
use std::str::FromStr;

/// A cache to speed up finding locations. The assumption is that most lookups
/// happen either in the current or the next source line, which would happen
/// naturally if we are scanning the source left to right.
/// If there is a cache hit in the current line, there is no lookup at all -
/// just quick arithmetic to calculate the column offset. If the hit is in
/// the next line, we "slide" the cache - the next line becomes the current
/// one, and we fetch a reference to the next line, which is also an O(1)
/// operation.
#[derive(Debug, Default)]
struct FindLineCache<'a> {
    /// 1-based line number.
    line_no: u32,
    /// The last found line.
    line_ref: DataRef<'a, u8>,
    /// The following line.
    next_line_ref: DataRef<'a, u8>,
}

/// Converts from Hermes AST to Juno AST
pub struct Converter<'parser> {
    pub hparser: &'parser HermesParser<'parser>,
    /// The file id to use for the converted coordinates.
    pub file_id: SourceId,

    /// A cache to speed up finding locations.
    line_cache: FindLineCache<'parser>,

    /// Map from NodeLabel, which has been uniqued in Hermes, to an
    /// ast::Identifier. This allows us to avoid repeated conversion of the same
    /// NodeLabel.
    atom_tab: HashMap<NodeLabel, atom_table::Atom>,
}

/// Adjust the source location backwards making sure it doesn't point to \r or
/// in the middle of a utf-8 sequence.
#[inline]
fn adjust_source_location(buf: &[u8], mut index: usize) -> usize {
    // In the very unlikely case that `index` points to a '\r', we skip backwards
    // until we find another character, while being careful not to fall off the
    // beginning of the buffer.
    if buf[index] == b'\r' || is_utf8_continuation(buf[index]) {
        loop {
            if index == 0 {
                // This is highly unlikely but theoretically possible. There were only
                // '\r' between `index` and the start of the buffer.
                break;
            }
            index -= 1;
            if !(buf[index] == b'\r' || is_utf8_continuation(buf[index])) {
                break;
            }
        }
    }

    index
}

impl FindLineCache<'_> {
    fn make_source_loc(&self, mut index: usize) -> ast::SourceLoc {
        index = adjust_source_location(self.line_ref.as_slice(), index);
        ast::SourceLoc {
            line: self.line_no,
            col: u32::try_from(index + 1).unwrap(),
        }
    }
}

impl<'parser> Converter<'parser> {
    pub fn new(hparser: &'parser HermesParser<'parser>, file_id: SourceId) -> Self {
        Converter {
            hparser,
            file_id,
            line_cache: Default::default(),
            atom_tab: Default::default(),
        }
    }

    /// Convert a SMLoc to ast::SourceLoc using the line_cache. Best cache
    /// utilization is achieved if locations are always increasing. In this way
    /// the next location will almost always be in the current line or the next
    /// line.
    pub fn cvt_smloc(&mut self, loc: SMLoc) -> ast::SourceLoc {
        assert!(
            loc.is_valid(),
            "All source locations from Hermes parser must be valid"
        );

        // Check the cache with the hope that the lookup is within the last line or
        // the next line.
        if let Some(index) = self.line_cache.line_ref.try_offset_from(loc.as_ptr()) {
            return self.line_cache.make_source_loc(index);
        }
        if let Some(index) = self.line_cache.next_line_ref.try_offset_from(loc.as_ptr()) {
            self.line_cache.line_no += 1;
            self.line_cache.line_ref = self.line_cache.next_line_ref;
            self.line_cache.next_line_ref = self.hparser.get_line_ref(self.line_cache.line_no + 1);
            return self.line_cache.make_source_loc(index);
        }

        let line_coord = self
            .hparser
            .find_line(loc)
            .expect("Location from Hermes parser cannot be found");

        // Populate the cache.
        self.line_cache = FindLineCache {
            line_no: line_coord.line_no,
            line_ref: line_coord.line_ref,
            next_line_ref: self.hparser.get_line_ref(line_coord.line_no + 1),
        };

        self.line_cache
            .make_source_loc(line_coord.line_ref.try_offset_from(loc.as_ptr()).unwrap())
    }

    pub fn cvt_label(&mut self, ctx: &ast::GCLock<'_, '_>, u: NodeLabel) -> ast::NodeLabel {
        *self
            .atom_tab
            .entry(u)
            .or_insert_with(move || ctx.atom(utf8_with_surrogates_to_string(u.as_slice()).unwrap()))
    }

    pub fn cvt_label_opt(
        &mut self,
        ctx: &ast::GCLock<'_, '_>,
        u: NodeLabelOpt,
    ) -> Option<ast::NodeLabel> {
        u.as_node_label().map(|u| self.cvt_label(ctx, u))
    }

    /// Report an invalid node kind for conversion via the SourceManager.
    pub fn report_invalid_node(&self, lock: &ast::GCLock, node: NodePtr, range: ast::SourceRange) {
        use hermes::parser::NodeKind::*;
        let node = node.as_ref();
        match node.kind {
            CoverEmptyArgs => {
                lock.sm().error(range, "invalid empty parentheses '( )'");
            }
            CoverTrailingComma => {
                lock.sm().error(range, "expression expected after ','");
            }
            CoverInitializer => {
                lock.sm()
                    .error(range, "':' expected in property initialization");
            }
            CoverRestElement => {
                lock.sm().error(range, "'...' not allowed in this context");
            }
            _ => {
                lock.sm()
                    .error(range, format!("unsupported syntax: {:?}", node.kind));
            }
        }
    }
}

/// # Safety
/// `n` must be valid.
pub unsafe fn cvt_node_ptr_opt<'gc, 'ast: 'gc>(
    cvt: &mut Converter<'_>,
    ctx: &'gc ast::GCLock<'ast, '_>,
    n: NodePtrOpt,
) -> Option<&'gc ast::Node<'gc>> {
    n.as_node_ptr()
        .map(|n| unsafe { cvt_node_ptr(cvt, ctx, n) })
}

pub unsafe fn cvt_node_list<'gc, 'ast: 'gc>(
    cvt: &mut Converter<'_>,
    ctx: &'gc ast::GCLock<'ast, '_>,
    n: NodeListRef,
) -> ast::NodeList<'gc> {
    let mut res = ast::NodeList::new();
    for node in n.iter() {
        res.push(cvt_node_ptr(cvt, ctx, NodePtr::new(node)));
    }
    res
}

pub unsafe fn cvt_node_list_opt<'gc, 'ast: 'gc>(
    cvt: &mut Converter<'_>,
    ctx: &'gc ast::GCLock<'ast, '_>,
    n: NodeListOptRef,
) -> Option<ast::NodeList<'gc>> {
    n.as_node_list_ref()
        .map(|n| unsafe { cvt_node_list(cvt, ctx, n) })
}

pub fn cvt_string(l: NodeString) -> ast::NodeString {
    ast::NodeString {
        str: utf8_with_surrogates_to_utf16(l.as_slice()).unwrap(),
    }
}

pub fn cvt_string_opt(l: NodeStringOpt) -> Option<ast::NodeString> {
    l.as_node_string().map(cvt_string)
}

/// Convert any of the enum `NodeLabel`s into their respective Rust enum types.
/// Use `unwrap` because the Hermes parser won't produce invalid strings in
/// those property positions.
/// Restrict the `Err` type on `T` to allow use of `unwrap` after `parse`.
pub fn cvt_enum<T: FromStr>(u: NodeLabel) -> T
where
    <T as FromStr>::Err: std::fmt::Debug,
{
    unsafe { std::str::from_utf8_unchecked(u.as_slice()) }
        .parse()
        .unwrap()
}
