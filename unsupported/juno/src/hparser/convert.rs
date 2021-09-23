/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use super::generated_cvt::cvt_node_ptr;
use crate::ast;
use hermes::parser::{
    DataRef, HermesParser, NodeLabel, NodeLabelOpt, NodeListOptRef, NodeListRef, NodePtr,
    NodePtrOpt, NodeString, NodeStringOpt, SMLoc, SMRange,
};
use hermes::utf::{
    is_utf8_continuation, utf8_with_surrogates_to_string, utf8_with_surrogates_to_utf16,
};
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
pub struct Converter<'a> {
    pub hparser: &'a HermesParser<'a>,
    /// The file id to use for the converted coordinates.
    pub file_id: u32,

    /// A cache to speed up finding locations.
    line_cache: FindLineCache<'a>,
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

impl Converter<'_> {
    pub fn new<'a>(hparser: &'a HermesParser<'a>, file_id: u32) -> Converter<'a> {
        Converter {
            hparser,
            file_id,
            line_cache: Default::default(),
        }
    }

    pub fn smrange(&mut self, smr: SMRange) -> ast::SourceRange {
        ast::SourceRange {
            file: self.file_id,
            start: self.cvt_smloc(smr.start),
            end: self.cvt_smloc(smr.end.pred()),
        }
    }

    /// Convert a SMLoc to ast::SourceLoc using the line_cache. Best cache
    /// utilization is achieved if locations are always increasing. In this way
    /// the next location will almost always be in the current line or the next
    /// line.
    fn cvt_smloc(&mut self, loc: SMLoc) -> ast::SourceLoc {
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
}

/// # Safety
/// `n` must be valid.
pub unsafe fn cvt_node_ptr_opt(cvt: &mut Converter, n: NodePtrOpt) -> Option<ast::NodePtr> {
    n.as_node_ptr().map(|n| unsafe { cvt_node_ptr(cvt, n) })
}

pub unsafe fn cvt_node_list(cvt: &mut Converter, n: NodeListRef) -> ast::NodeList {
    let mut res = Vec::<ast::NodePtr>::new();
    for node in n.iter() {
        res.push(cvt_node_ptr(cvt, NodePtr::new(node)));
    }
    res
}

pub unsafe fn cvt_node_list_opt(cvt: &mut Converter, n: NodeListOptRef) -> Option<ast::NodeList> {
    n.as_node_list_ref()
        .map(|n| unsafe { cvt_node_list(cvt, n) })
}

pub fn cvt_string(l: NodeString) -> ast::StringLiteral {
    ast::StringLiteral {
        str: utf8_with_surrogates_to_utf16(l.as_slice()).unwrap(),
    }
}

pub fn cvt_string_opt(l: NodeStringOpt) -> Option<ast::StringLiteral> {
    l.as_node_string().map(cvt_string)
}

pub fn cvt_label(u: NodeLabel) -> ast::NodeLabel {
    ast::NodeLabel {
        str: utf8_with_surrogates_to_string(u.as_slice()).unwrap(),
    }
}

pub fn cvt_label_opt(u: NodeLabelOpt) -> Option<ast::NodeLabel> {
    u.as_node_label().map(cvt_label)
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
