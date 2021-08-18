/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use std::fmt;

#[macro_use]
mod def;
mod dump;
mod kind;
mod validate;

pub use kind::NodeKind;
use kind::NodeVariant;

pub use dump::{dump_json, Pretty};

/// A JavaScript AST node.
#[derive(Debug)]
pub struct Node {
    /// Location of the node in the JS file.
    pub range: SourceRange,

    /// Actual kind of the node.
    pub kind: NodeKind,
}

impl Node {
    /// Call the `visitor` on this node with a given `parent`.
    pub fn visit<'a, V: Visitor<'a>>(&'a self, visitor: &mut V, parent: Option<&'a Node>) {
        visitor.call(self, parent);
    }

    /// Check whether the node is valid.
    pub fn validate(&self) -> bool {
        validate::validate_node(self)
    }

    /// Call the `visitor` on only this node's children.
    pub fn visit_children<'a, V: Visitor<'a>>(&'a self, visitor: &mut V) {
        self.kind.visit_children(visitor, self);
    }
}

/// Trait implemented by those who call the visit functionality.
pub trait Visitor<'a> {
    /// Visit the Node `node` with the given `parent`.
    fn call(&mut self, node: &'a Node, parent: Option<&'a Node>);
}

/// A source range within a single JS file.
/// Represented as a closed interval: [start, end].
#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub struct SourceRange {
    /// Index of the file this range is in.
    pub file: u32,

    /// Start of the source range, inclusive.
    pub start: SourceLoc,

    /// End of the source range, inclusive.
    pub end: SourceLoc,
}

/// Line and column of a file.
#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub struct SourceLoc {
    /// 1-based line number.
    pub line: u32,

    /// 1-based column number.
    pub col: u32,
}

/// JS identifier represented as valid UTF-8.
#[derive(Debug)]
pub struct NodeLabel {
    pub str: String,
}

/// A single node child owned by a parent.
pub type NodePtr = Box<Node>;

/// A list of nodes owned by a parent.
pub type NodeList = Vec<NodePtr>;

/// JS string literals don't have to contain valid UTF-8,
/// so we wrap a `Vec<u16>`, which allows us to represent UTF-16 characters
/// without being subject to Rust's restrictions on [`String`].
pub struct StringLiteral {
    pub str: Vec<u16>,
}

impl fmt::Debug for StringLiteral {
    /// Format the StringLiteral as a `u""` string to make it more readable
    /// when debugging.
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> Result<(), fmt::Error> {
        write!(f, "u{:?}", String::from_utf16_lossy(&self.str))
    }
}

/// Trait implemented by possible child types of `NodeKind`.
trait NodeChild {
    /// Visit this child of the given `node`.
    /// Should be no-op for any type that doesn't contain pointers to other
    /// `Node`s.
    fn visit<'a, V: Visitor<'a>>(&'a self, visitor: &mut V, node: &'a Node);
}

impl NodeChild for f64 {
    fn visit<'a, V: Visitor<'a>>(&self, _visitor: &mut V, _node: &Node) {}
}

impl NodeChild for bool {
    fn visit<'a, V: Visitor<'a>>(&self, _visitor: &mut V, _node: &Node) {}
}

impl NodeChild for NodeLabel {
    fn visit<'a, V: Visitor<'a>>(&self, _visitor: &mut V, _node: &Node) {}
}

impl NodeChild for StringLiteral {
    fn visit<'a, V: Visitor<'a>>(&self, _visitor: &mut V, _node: &Node) {}
}

impl<T: NodeChild> NodeChild for Option<T> {
    fn visit<'a, V: Visitor<'a>>(&'a self, visitor: &mut V, node: &'a Node) {
        if let Some(t) = self {
            t.visit(visitor, node);
        }
    }
}

impl NodeChild for NodePtr {
    fn visit<'a, V: Visitor<'a>>(&'a self, visitor: &mut V, node: &'a Node) {
        visitor.call(self, Some(node));
    }
}

impl NodeChild for NodeList {
    fn visit<'a, V: Visitor<'a>>(&'a self, visitor: &mut V, node: &'a Node) {
        for child in self {
            visitor.call(child, Some(node));
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_string_literal() {
        assert_eq!(
            "u\"ABC\"",
            format!(
                "{:?}",
                StringLiteral {
                    str: vec!['A' as u16, 'B' as u16, 'C' as u16],
                }
            )
        );
    }
}
