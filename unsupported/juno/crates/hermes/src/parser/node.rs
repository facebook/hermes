/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use std::convert::AsRef;
use std::marker::PhantomData;
use std::ptr::NonNull;

use super::generated_ffi::NodeKind;

#[repr(C)]
#[derive(Copy, Clone)]
pub struct SMLoc {
    ptr: *const u8,
}

impl SMLoc {
    pub fn is_valid(self) -> bool {
        !self.ptr.is_null()
    }

    pub fn pred(self) -> SMLoc {
        SMLoc {
            ptr: self.ptr.wrapping_sub(1),
        }
    }

    pub fn as_ptr(self) -> *const u8 {
        self.ptr
    }
}

#[repr(C)]
#[derive(Copy, Clone)]
pub struct SMRange {
    pub start: SMLoc,
    pub end: SMLoc,
}

impl SMRange {
    pub fn is_empty(self) -> bool {
        self.start.ptr == self.end.ptr
    }
}

#[repr(C)]
#[derive(Copy, Clone)]
pub struct StringRef {
    data: *const u8,
    length: usize,
}

impl StringRef {
    pub fn as_slice(&self) -> &[u8] {
        // Rust doesn't allow null pointer in a slice.
        if self.data.is_null() {
            &[]
        } else {
            unsafe { std::slice::from_raw_parts(self.data, self.length) }
        }
    }
}

#[repr(C)]
#[derive(Copy, Clone)]
pub struct UniqueString {
    str: StringRef,
}

#[repr(transparent)]
#[derive(Copy, Clone, Eq, PartialEq, Hash)]
pub struct NodeLabel {
    ptr: *const UniqueString,
}

#[repr(transparent)]
#[derive(Copy, Clone)]
pub struct NodeLabelOpt {
    ptr: *const UniqueString,
}

#[repr(transparent)]
#[derive(Copy, Clone, Eq, PartialEq, Hash)]
pub struct NodeString {
    ptr: *const UniqueString,
}

#[repr(transparent)]
#[derive(Copy, Clone)]
pub struct NodeStringOpt {
    ptr: *const UniqueString,
}

impl NodeLabel {
    pub fn as_slice(&self) -> &[u8] {
        debug_assert!(!self.ptr.is_null(), "null NodeLabel");
        unsafe { (*self.ptr).str.as_slice() }
    }
}

impl NodeLabelOpt {
    pub fn as_node_label(&self) -> Option<NodeLabel> {
        if self.ptr.is_null() {
            None
        } else {
            Some(NodeLabel { ptr: self.ptr })
        }
    }
}

impl NodeString {
    pub fn as_slice(&self) -> &[u8] {
        debug_assert!(!self.ptr.is_null(), "null NodeLabel");
        unsafe { (*self.ptr).str.as_slice() }
    }
}

impl NodeStringOpt {
    pub fn as_node_string(&self) -> Option<NodeString> {
        if self.ptr.is_null() {
            None
        } else {
            Some(NodeString { ptr: self.ptr })
        }
    }
}

#[repr(transparent)]
#[derive(Copy, Clone)]
pub struct NodePtr {
    ptr: NonNull<Node>,
}

#[repr(transparent)]
#[derive(Copy, Clone)]
pub struct NodePtrOpt {
    ptr: *const Node,
}

impl NodePtr {
    pub fn new(n: &Node) -> NodePtr {
        NodePtr {
            ptr: NonNull::from(n),
        }
    }
}

impl AsRef<Node> for NodePtr {
    fn as_ref(&self) -> &Node {
        debug_assert!(!self.ptr.as_ptr().is_null(), "null NodePtr");
        unsafe { self.ptr.as_ref() }
    }
}

impl NodePtrOpt {
    pub fn as_node_ptr(&self) -> Option<NodePtr> {
        if self.ptr.is_null() {
            None
        } else {
            Some(NodePtr {
                ptr: unsafe { NonNull::new_unchecked(self.ptr as *mut Node) },
            })
        }
    }
}

#[repr(C)]
pub struct NodeList {
    pub prev: *const NodeList,
    pub next: *const NodeList,
}

#[repr(transparent)]
pub struct NodeListRef {
    ptr: *const NodeList,
}

#[repr(transparent)]
pub struct NodeListOptRef {
    ptr: *const NodeList,
}

impl NodeListRef {
    pub fn iter(&self) -> NodeIterator {
        NodeIterator {
            head: self.ptr,
            cur: self.ptr,
            ph: PhantomData,
        }
    }
}

impl AsRef<NodeList> for NodeListRef {
    fn as_ref(&self) -> &NodeList {
        debug_assert!(!self.ptr.is_null(), "null NodeListRef");
        unsafe { &*self.ptr }
    }
}

impl NodeListOptRef {
    pub fn as_node_list_ref(&self) -> Option<NodeListRef> {
        if self.ptr.is_null() {
            None
        } else {
            Some(NodeListRef { ptr: self.ptr })
        }
    }
}

pub struct NodeIterator<'a> {
    head: *const NodeList,
    cur: *const NodeList,
    ph: PhantomData<&'a Node>,
}

impl<'a> std::iter::Iterator for NodeIterator<'a> {
    type Item = &'a Node;

    fn next(&mut self) -> Option<Self::Item> {
        self.cur = unsafe { (*self.cur).next };
        if self.cur == self.head {
            None
        } else {
            Some(unsafe { &*(self.cur as *const Node) })
        }
    }
}

#[repr(C)]
pub struct Node {
    pub link: NodeList,
    pub kind: NodeKind,
    pub parens: u32,
    pub source_range: SMRange,
    pub debug_loc: SMLoc,
}
