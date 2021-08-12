/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use std::marker::PhantomData;

#[repr(C)]
pub struct SMLoc {
    ptr: *const u8,
}

#[repr(C)]
pub struct SMRange {
    start: SMLoc,
    end: SMLoc,
}

#[repr(C)]
struct StringRef {
    data: *const u8,
    length: usize,
}

impl StringRef {
    fn as_slice(&self) -> &[u8] {
        unsafe { std::slice::from_raw_parts(self.data, self.length) }
    }
}

#[repr(C)]
pub struct UniqueString {
    str: StringRef,
}

#[repr(transparent)]
#[derive(Copy, Clone)]
pub struct NodeLabel {
    ptr: *const UniqueString,
}

#[repr(transparent)]
#[derive(Copy, Clone)]
pub struct NodeLabelOpt {
    ptr: *const UniqueString,
}

#[repr(transparent)]
#[derive(Copy, Clone)]
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
        if self.ptr.is_null() {
            panic!("null NodeLabel");
        }
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
        if self.ptr.is_null() {
            panic!("null NodeLabel");
        }
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
    ptr: *const Node,
}

#[repr(transparent)]
#[derive(Copy, Clone)]
pub struct NodePtrOpt {
    ptr: *const Node,
}

impl NodePtr {
    pub fn new(n: &Node) -> NodePtr {
        NodePtr { ptr: n }
    }

    pub fn as_ref(&self) -> &Node {
        if self.ptr.is_null() {
            panic!("null NodePtr");
        }
        unsafe { &*self.ptr }
    }
}

impl NodePtrOpt {
    pub fn as_node_ptr(&self) -> Option<NodePtr> {
        if self.ptr.is_null() {
            None
        } else {
            Some(NodePtr { ptr: self.ptr })
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
    pub fn as_ref(&self) -> &NodeList {
        if self.ptr.is_null() {
            panic!("NodeListRef is null")
        }
        unsafe { &*self.ptr }
    }
    pub fn iter(&self) -> NodeIterator {
        NodeIterator {
            head: self.ptr,
            cur: self.ptr,
            ph: PhantomData,
        }
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

/// Just a temporary placeholder.
#[repr(transparent)]
pub struct NodeKind(u32);

#[repr(C)]
pub struct Node {
    pub link: NodeList,
    pub kind: NodeKind,
    pub parens: u32,
    pub source_range: SMRange,
    pub debug_loc: SMLoc,
}
