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
mod atom_table;
mod dump;
mod kind;
mod validate;

use kind::NodeVariant;

pub use dump::{dump_json, Pretty};
pub use kind::*;
pub use validate::{validate_tree, ValidationError};

pub use atom_table::{Atom, AtomTable, INVALID_ATOM};

/// The storage for AST nodes.
/// Can be used to allocate and free nodes.
#[derive(Debug, Default)]
pub struct Context {
    /// List of all the nodes stored in this context.
    storage: Vec<StorageEntry>,

    /// First element of the free list if there is one.
    free_list_head: Option<usize>,

    /// All identifiers are kept here.
    atom_tab: AtomTable,
}

#[derive(Debug)]
enum StorageEntry {
    /// Allocated node.
    Used(Node),

    /// Free list entry, with the index of the next entry in the free list.
    Free(Option<usize>),
}

impl Context {
    pub fn new() -> Context {
        Default::default()
    }

    #[inline]
    pub fn node(&self, ptr: NodePtr) -> &Node {
        match &self.storage[ptr.0] {
            StorageEntry::Used(node) => node,
            StorageEntry::Free(_) => {
                panic!("Attempt to get freed node");
            }
        }
    }

    #[inline]
    pub fn node_mut(&mut self, ptr: NodePtr) -> &mut Node {
        match &mut self.storage[ptr.0] {
            StorageEntry::Used(node) => node,
            StorageEntry::Free(_) => {
                panic!("Attempt to get freed node");
            }
        }
    }

    pub fn alloc(&mut self, node: Node) -> NodePtr {
        match self.free_list_head {
            None => {
                self.storage.push(StorageEntry::Used(node));
                NodePtr(self.storage.len() - 1)
            }
            Some(idx) => match self.storage[idx] {
                StorageEntry::Free(next) => {
                    self.storage[idx] = StorageEntry::Used(node);
                    self.free_list_head = next;
                    NodePtr(idx)
                }
                StorageEntry::Used(_) => {
                    panic!("Invalid entry in free list at {}", idx)
                }
            },
        }
    }

    pub fn free(&mut self, ptr: NodePtr) {
        let idx = ptr.0;
        self.storage[idx] = StorageEntry::Free(self.free_list_head);
        self.free_list_head = Some(idx);
    }

    /// Add a string to the identifier table.
    #[inline]
    pub fn add_atom<V: Into<String> + AsRef<str>>(&mut self, value: V) -> Atom {
        self.atom_tab.add_atom(value)
    }

    /// Obtain the contents of an atom from the atom table.
    #[inline]
    pub fn str(&self, index: Atom) -> &str {
        self.atom_tab.str(index)
    }
}

// /// A JavaScript AST node.
// #[derive(Debug)]
// pub struct Node {
//     /// Location of the node in the JS file.
//     pub range: SourceRange,

//     /// Actual kind of the node.
//     pub kind: NodeKind,
// }

impl NodePtr {
    /// Call the `visitor` on this node with a given `parent`.
    pub fn visit<V: Visitor>(self, ctx: &Context, visitor: &mut V, parent: Option<NodePtr>) {
        visitor.call(ctx, self, parent);
    }

    /// Call the `visitor` on only this node's children.
    pub fn visit_children<V: Visitor>(self, ctx: &Context, visitor: &mut V) {
        let node = self.get(ctx);
        node.visit_children(ctx, visitor, self);
    }
}

/// Trait implemented by those who call the visit functionality.
pub trait Visitor {
    /// Visit the Node `node` with the given `parent`.
    fn call(&mut self, ctx: &Context, node: NodePtr, parent: Option<NodePtr>);
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
pub type NodeLabel = Atom;

/// A single node child owned by a parent.
#[derive(Debug, Copy, Clone)]
pub struct NodePtr(usize);

impl NodePtr {
    #[inline]
    pub fn get<'a>(&self, ctx: &'a Context) -> &'a Node {
        ctx.node(*self)
    }

    #[inline]
    pub fn get_mut<'a>(&self, ctx: &'a mut Context) -> &'a mut Node {
        ctx.node_mut(*self)
    }
}

/// A list of nodes owned by a parent.
pub type NodeList = Vec<NodePtr>;

/// JS string literals don't have to contain valid UTF-8,
/// so we wrap a `Vec<u16>`, which allows us to represent UTF-16 characters
/// without being subject to Rust's restrictions on [`String`].
pub struct NodeString {
    pub str: Vec<u16>,
}

impl fmt::Debug for NodeString {
    /// Format the NodeString as a `u""` string to make it more readable
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
    fn visit<V: Visitor>(&self, _ctx: &Context, _visitor: &mut V, _node: NodePtr) {}
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

impl NodeChild for NodeString {}

impl<T: NodeChild> NodeChild for Option<T> {
    fn visit<V: Visitor>(&self, ctx: &Context, visitor: &mut V, node: NodePtr) {
        if let Some(t) = self {
            t.visit(ctx, visitor, node);
        }
    }
}

impl NodeChild for NodePtr {
    fn visit<V: Visitor>(&self, ctx: &Context, visitor: &mut V, node: NodePtr) {
        visitor.call(ctx, *self, Some(node));
    }
}

impl NodeChild for NodeList {
    fn visit<V: Visitor>(&self, ctx: &Context, visitor: &mut V, node: NodePtr) {
        for child in self {
            visitor.call(ctx, *child, Some(node));
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
                NodeString {
                    str: vec!['A' as u16, 'B' as u16, 'C' as u16],
                }
            )
        );
    }
}
