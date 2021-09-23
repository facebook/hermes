/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use std::fmt;
use support::define_str_enum;
use thiserror::Error;

#[macro_use]
mod def;
mod dump;
mod kind;
mod validate;

pub use kind::NodeKind;
use kind::NodeVariant;

pub use dump::{dump_json, Pretty};
pub use validate::{validate_tree, ValidationError};

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

impl SourceLoc {
    /// Return an instance of SourceLoc initialized to an invalid value.
    pub fn invalid() -> SourceLoc {
        SourceLoc { line: 0, col: 0 }
    }
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

#[derive(Debug, Copy, Clone, Error)]
#[error("Invalid string property for AST node")]
pub struct TryFromStringError;

define_str_enum!(
    UnaryExpressionOperator,
    TryFromStringError,
    (Delete, "delete"),
    (Void, "void"),
    (Typeof, "typeof"),
    (Plus, "+"),
    (Minus, "-"),
    (BitNot, "~"),
    (Not, "!"),
);

define_str_enum!(
    BinaryExpressionOperator,
    TryFromStringError,
    (LooseEquals, "=="),
    (LooseNotEquals, "!="),
    (StrictEquals, "==="),
    (StrictNotEquals, "!=="),
    (Less, "<"),
    (LessEquals, "<="),
    (Greater, ">"),
    (GreaterEquals, ">="),
    (LShift, "<<"),
    (RShift, ">>"),
    (RShift3, ">>>"),
    (Plus, "+"),
    (Minus, "-"),
    (Mult, "*"),
    (Div, "/"),
    (Mod, "%"),
    (BitOr, "|"),
    (BitXor, "^"),
    (BitAnd, "&"),
    (Exp, "**"),
    (In, "in"),
    (Instanceof, "instanceof"),
);

define_str_enum!(
    LogicalExpressionOperator,
    TryFromStringError,
    (And, "&&"),
    (Or, "||"),
    (NullishCoalesce, "??"),
);

define_str_enum!(
    UpdateExpressionOperator,
    TryFromStringError,
    (Increment, "++"),
    (Decrement, "--"),
);

define_str_enum!(
    AssignmentExpressionOperator,
    TryFromStringError,
    (Assign, "="),
    (LShiftAssign, "<<="),
    (RShiftAssign, ">>="),
    (RShift3Assign, ">>>="),
    (PlusAssign, "+="),
    (MinusAssign, "-="),
    (MultAssign, "*="),
    (DivAssign, "/="),
    (ModAssign, "%="),
    (BitOrAssign, "|="),
    (BitXorAssign, "^="),
    (BitAndAssign, "&="),
    (ExpAssign, "**="),
    (LogicalOrAssign, "||="),
    (LogicalAndAssign, "&&="),
    (NullishCoalesceAssign, "??="),
);

define_str_enum!(
    VariableDeclarationKind,
    TryFromStringError,
    (Var, "var"),
    (Let, "let"),
    (Const, "const"),
);

define_str_enum!(
    PropertyKind,
    TryFromStringError,
    (Init, "init"),
    (Get, "get"),
    (Set, "set"),
);

define_str_enum!(
    MethodDefinitionKind,
    TryFromStringError,
    (Method, "method"),
    (Constructor, "constructor"),
    (Get, "get"),
    (Set, "set"),
);

define_str_enum!(
    ImportKind,
    TryFromStringError,
    (Value, "value"),
    (Type, "type"),
    (Typeof, "typeof"),
);

define_str_enum!(
    ExportKind,
    TryFromStringError,
    (Value, "value"),
    (Type, "type"),
);

/// Trait implemented by possible child types of `NodeKind`.
trait NodeChild {
    /// Visit this child of the given `node`.
    /// Should be no-op for any type that doesn't contain pointers to other
    /// `Node`s.
    fn visit<'a, V: Visitor<'a>>(&'a self, _visitor: &mut V, _node: &'a Node) {}
}

impl NodeChild for f64 {}

impl NodeChild for bool {}

impl NodeChild for NodeLabel {}
impl NodeChild for UnaryExpressionOperator {}
impl NodeChild for BinaryExpressionOperator {}
impl NodeChild for LogicalExpressionOperator {}
impl NodeChild for UpdateExpressionOperator {}
impl NodeChild for AssignmentExpressionOperator {}
impl NodeChild for VariableDeclarationKind {}
impl NodeChild for PropertyKind {}
impl NodeChild for MethodDefinitionKind {}
impl NodeChild for ImportKind {}
impl NodeChild for ExportKind {}

impl NodeChild for StringLiteral {}

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
