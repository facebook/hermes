/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use super::{Node, NodeLabel, NodeList, NodePtr, StringLiteral};

/// Trait implemented by those who call the visit functionality.
pub trait Visitor {
    /// Visit the Node `node` with the given `parent`.
    fn call(&mut self, node: &Node, parent: Option<&Node>);
}

/// Trait implemented by possible child types of `NodeKind`.
pub trait Visitable {
    /// Visit this child of the given `node`.
    /// Should be no-op for any type that doesn't contain pointers to other
    /// `Node`s.
    fn visit<V: Visitor>(&self, visitor: &mut V, node: &Node);
}

impl Visitable for f64 {
    fn visit<V: Visitor>(&self, _visitor: &mut V, _node: &Node) {}
}

impl Visitable for bool {
    fn visit<V: Visitor>(&self, _visitor: &mut V, _node: &Node) {}
}

impl Visitable for NodeLabel {
    fn visit<V: Visitor>(&self, _visitor: &mut V, _node: &Node) {}
}

impl Visitable for StringLiteral {
    fn visit<V: Visitor>(&self, _visitor: &mut V, _node: &Node) {}
}

impl<T: Visitable> Visitable for Option<T> {
    fn visit<V: Visitor>(&self, visitor: &mut V, node: &Node) {
        if let Some(t) = self {
            t.visit(visitor, node);
        }
    }
}

impl Visitable for NodePtr {
    fn visit<V: Visitor>(&self, visitor: &mut V, node: &Node) {
        visitor.call(self, Some(node));
    }
}

impl Visitable for NodeList {
    fn visit<V: Visitor>(&self, visitor: &mut V, node: &Node) {
        for child in self {
            visitor.call(child, Some(node));
        }
    }
}
