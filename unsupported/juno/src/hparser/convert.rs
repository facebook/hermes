/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use super::generated_ffi::cvt_node_ptr;
use super::node::*;
use super::utf::*;
use crate::ast;

/// # Safety
/// `n` must be valid.
pub unsafe fn cvt_node_ptr_opt(n: NodePtrOpt) -> Option<ast::NodePtr> {
    n.as_node_ptr().map(|n| unsafe { cvt_node_ptr(n) })
}

pub unsafe fn cvt_node_list(n: NodeListRef) -> ast::NodeList {
    let mut res = Vec::<ast::NodePtr>::new();
    for node in n.iter() {
        res.push(cvt_node_ptr(NodePtr::new(node)));
    }
    res
}

pub unsafe fn cvt_node_list_opt(n: NodeListOptRef) -> Option<ast::NodeList> {
    n.as_node_list_ref().map(|n| unsafe { cvt_node_list(n) })
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
