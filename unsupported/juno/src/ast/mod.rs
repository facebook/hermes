/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use std::fmt;

#[macro_use]
mod def;
mod kind;

pub use kind::NodeKind;
use kind::NodeVariant;

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

    /// Check whether the node is valid.
    pub fn validate(&self) -> bool {
        self.kind.validate(self)
    }

    /// Call the `visitor` on only this node's children.
    pub fn visit_children<V: Visitor>(&self, visitor: &mut V) {
        self.kind.visit_children(visitor, self);
    }
}

/// Trait implemented by those who call the visit functionality.
pub trait Visitor {
    /// Visit the Node `node` with the given `parent`.
    fn call(&mut self, node: &Node, parent: Option<&Node>);
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
    fn visit<V: Visitor>(&self, visitor: &mut V, node: &Node);

    /// Check whether this is a valid child of `node` given the constraints.
    fn validate(&self, node: &Node, constraints: &[NodeVariant]) -> bool;
}

impl NodeChild for f64 {
    fn visit<V: Visitor>(&self, _visitor: &mut V, _node: &Node) {}

    fn validate(&self, _node: &Node, _constraints: &[NodeVariant]) -> bool {
        true
    }
}

impl NodeChild for bool {
    fn visit<V: Visitor>(&self, _visitor: &mut V, _node: &Node) {}

    fn validate(&self, _node: &Node, _constraints: &[NodeVariant]) -> bool {
        true
    }
}

impl NodeChild for NodeLabel {
    fn visit<V: Visitor>(&self, _visitor: &mut V, _node: &Node) {}

    fn validate(&self, _node: &Node, _constraints: &[NodeVariant]) -> bool {
        true
    }
}

impl NodeChild for StringLiteral {
    fn visit<V: Visitor>(&self, _visitor: &mut V, _node: &Node) {}

    fn validate(&self, _node: &Node, _constraints: &[NodeVariant]) -> bool {
        true
    }
}

impl<T: NodeChild> NodeChild for Option<T> {
    fn visit<V: Visitor>(&self, visitor: &mut V, node: &Node) {
        if let Some(t) = self {
            t.visit(visitor, node);
        }
    }

    fn validate(&self, node: &Node, constraints: &[NodeVariant]) -> bool {
        match self {
            None => true,
            Some(t) => t.validate(node, constraints),
        }
    }
}

impl NodeChild for NodePtr {
    fn visit<V: Visitor>(&self, visitor: &mut V, node: &Node) {
        visitor.call(self, Some(node));
    }

    fn validate(&self, _node: &Node, constraints: &[NodeVariant]) -> bool {
        for &constraint in constraints {
            if instanceof(self.kind.variant(), constraint) {
                return true;
            }
        }
        false
    }
}

impl NodeChild for NodeList {
    fn visit<V: Visitor>(&self, visitor: &mut V, node: &Node) {
        for child in self {
            visitor.call(child, Some(node));
        }
    }

    fn validate(&self, _node: &Node, constraints: &[NodeVariant]) -> bool {
        'elems: for elem in self {
            for &constraint in constraints {
                if instanceof(elem.kind.variant(), constraint) {
                    // Found a valid constraint for this element,
                    // move on to the next element.
                    continue 'elems;
                }
            }
            // Failed to find a constraint that matched, early return.
            return false;
        }
        true
    }
}

/// Return whether `subtype` contains `supertype` in its parent chain.
fn instanceof(subtype: NodeVariant, supertype: NodeVariant) -> bool {
    let mut cur = subtype;
    loop {
        if cur == supertype {
            return true;
        }
        match cur.parent() {
            None => return false,
            Some(next) => {
                cur = next;
            }
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
