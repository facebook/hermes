/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//! Visitor structures and helpers for the AST.

use crate::GCLock;
use crate::Node;
use crate::NodeField;

/// Indicates the path to the current node.
#[derive(Debug, Copy, Clone)]
pub struct Path<'a> {
    /// Parent node.
    pub parent: &'a Node<'a>,

    /// Field name in the path node.
    pub field: NodeField,
}

impl<'a> Path<'a> {
    pub fn new(parent: &'a Node<'a>, field: NodeField) -> Path<'a> {
        Path { parent, field }
    }
}

/// Trait implemented by those who call the visit functionality.
pub trait Visitor<'gc> {
    /// Visit the Node `node` with the given `path`.
    fn call(&mut self, ctx: &'gc GCLock, node: &'gc Node<'gc>, path: Option<Path<'gc>>);
}

/// Indicates what mutation occurred to an element of the AST during [`VisitorMut`] use.
#[derive(Debug)]
pub enum TransformResult<T> {
    /// No change to the element.
    Unchanged,

    /// Element must be removed, if possible.
    /// If the element cannot be removed, it will be replaced with EmptyStatement.
    /// **Only statements may be removed from non-optional fields,
    /// as removing expressions would result in an invalid AST.**
    Removed,

    /// Element should be swapped out for the wrapped element.
    Changed(T),

    /// Element should be swapped out for multiple wrapped elements.
    Expanded(Vec<T>),
}

/// Trait implemented by those who call the visit functionality.
pub trait VisitorMut<'gc> {
    /// Visit the Node `node` with the given `path`.
    fn call(
        &mut self,
        ctx: &'gc GCLock,
        node: &'gc Node<'gc>,
        path: Option<Path<'gc>>,
    ) -> TransformResult<&'gc Node<'gc>>;
}
