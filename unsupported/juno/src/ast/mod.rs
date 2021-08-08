/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use std::fmt;

mod kind;
mod visit;

pub use kind::NodeKind;
pub use visit::{Visitable, Visitor};

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
    pub fn visit<V: Visitor>(&self, visitor: &mut V, parent: Option<&Node>) {
        visitor.call(self, parent);
    }

    /// Call the `visitor` on only this node's children.
    pub fn visit_children<V: Visitor>(&self, visitor: &mut V) {
        self.kind.visit_children(visitor, self);
    }
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
