/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use super::generated_ffi::cvt_node_ptr;
use super::hermes_parser::HermesParser;
use super::node::*;
use crate::ast;
use crate::hermes_utf::{utf8_with_surrogates_to_string, utf8_with_surrogates_to_utf16};
use std::os::raw::c_int;
use std::str::FromStr;

/// A trait converting an AST location, presumably using a source map. The input file_id is
/// expected to be ignored, but it could be used to indicate something, as it is passed
/// unmodified.
pub trait CvtLoc {
    fn convert_loc(&self, file_id: u32, line: u32, column: u32) -> (u32, u32, u32);
}

/// Converts from Hermes AST to Juno AST
pub struct Converter<'a> {
    pub hparser: &'a HermesParser<'a>,
    /// The file id to use for the converted coordinates.
    pub file_id: u32,
    /// Optional location converter.
    pub cvt_loc: Option<&'a dyn CvtLoc>,
}

impl Converter<'_> {
    pub fn smrange(&self, smr: SMRange) -> ast::SourceRange {
        assert!(
            smr.start.is_valid() && smr.end.is_valid(),
            "All source range from Hermes parser must be valid"
        );

        let mut start = self
            .hparser
            .find_coord(smr.start)
            .expect("Location from Hermes parser cannot be found");
        let mut end = self
            .hparser
            .find_coord(smr.end.pred())
            .expect("Location from Hermes parser cannot be found");
        let mut file_id = self.file_id;

        if let Some(cvt_loc) = self.cvt_loc {
            let res_st = cvt_loc.convert_loc(self.file_id, start.line as u32, start.column as u32);
            file_id = res_st.0;
            start.line = res_st.1 as c_int;
            start.column = res_st.2 as c_int;

            let res_end = cvt_loc.convert_loc(self.file_id, end.line as u32, end.column as u32);
            end.line = res_end.1 as c_int;
            end.column = res_end.2 as c_int;

            debug_assert!(
                res_end.0 == res_st.0,
                "End location has a different file_id"
            );
        }

        ast::SourceRange {
            file: file_id,
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
