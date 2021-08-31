/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use super::generated_cvt::cvt_node_ptr;
use crate::ast;
use hermes::parser::{
    HermesParser, NodeLabel, NodeLabelOpt, NodeListOptRef, NodeListRef, NodePtr, NodePtrOpt,
    NodeString, NodeStringOpt, SMRange,
};
use hermes::utf::{utf8_with_surrogates_to_string, utf8_with_surrogates_to_utf16};
use std::str::FromStr;

/// Converts from Hermes AST to Juno AST
pub struct Converter<'a> {
    pub hparser: &'a HermesParser<'a>,
    /// The file id to use for the converted coordinates.
    pub file_id: u32,
}

impl Converter<'_> {
    pub fn smrange(&self, smr: SMRange) -> ast::SourceRange {
        assert!(
            smr.start.is_valid() && smr.end.is_valid(),
            "All source range from Hermes parser must be valid"
        );

        let start = self
            .hparser
            .find_coord(smr.start)
            .expect("Location from Hermes parser cannot be found");
        let end = self
            .hparser
            .find_coord(smr.end.pred())
            .expect("Location from Hermes parser cannot be found");

        ast::SourceRange {
            file: self.file_id,
            start: ast::SourceLoc {
                line: start.line as u32,
                col: start.column as u32,
            },
            end: ast::SourceLoc {
                line: end.line as u32,
                col: end.column as u32,
            },
        }
    }
}

/// # Safety
/// `n` must be valid.
pub unsafe fn cvt_node_ptr_opt(cvt: &Converter, n: NodePtrOpt) -> Option<ast::NodePtr> {
    n.as_node_ptr().map(|n| unsafe { cvt_node_ptr(cvt, n) })
}

pub unsafe fn cvt_node_list(cvt: &Converter, n: NodeListRef) -> ast::NodeList {
    let mut res = Vec::<ast::NodePtr>::new();
    for node in n.iter() {
        res.push(cvt_node_ptr(cvt, NodePtr::new(node)));
    }
    res
}

pub unsafe fn cvt_node_list_opt(cvt: &Converter, n: NodeListOptRef) -> Option<ast::NodeList> {
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
