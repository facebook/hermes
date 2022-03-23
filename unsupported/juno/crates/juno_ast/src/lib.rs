/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//! Juno abstract syntax tree.
//!
//! Provides a transformable AST which is stored in a garbage-collected heap.
//! All nodes are stored in [`Context`], which handles memory management of the nodes
//! and exposes a safe API.
//!
//! Allocation and viewing of nodes must be done via the use of a [`GCLock`],
//! **of which there must be only one active per thread at any time**,
//! to avoid accidentally mixing `Node`s between `Context`.
//! The `GCLock` will provide `&'gc Node<'gc>`,
//! i.e. a `Node` that does not outlive the `GCLock` and which references other `Node`s which
//! also do not outlive the `GCLock`.
//!
//! Nodes are allocated and cloned/modified by using the various `Builder` structs,
//! for example [`NumericLiteralBuilder`].
//! Builder structs have `build_template` functions that take "template" structs,
//! which have the same general structure as the various node kinds, but are only used
//! for building/allocating nodes in the `Context`.
//!
//! Visitor patterns are provided by [`Visitor`] and [`VisitorMut`].

use juno_support::atom_table::Atom;
use juno_support::define_str_enum;
use std::{fmt, marker::PhantomData};
use thiserror::Error;

#[macro_use]
mod def;

mod context;
mod dump;
mod field;
mod kind;
mod validate;

pub use juno_support::source_manager::{SourceId, SourceLoc, SourceManager, SourceRange};

pub use field::NodeField;
pub use kind::NodeVariant;

pub use context::{Context, GCLock, NodePtr, NodeRc};
pub use dump::{dump_json, Pretty};
pub use kind::*;
pub use validate::{validate_tree, validate_tree_pure, TreeValidationError, ValidationError};

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

/// Metadata common to all AST nodes.
///
/// Stored inside [`Node`] and must not be constructed directly by users.
/// Instead it should be allocated in the GC by using the `Builder`.
/// Must not derive `Clone` or `Copy` in order to avoid `Node` being allocated
/// by callers outside this module.
#[derive(Debug)]
pub struct NodeMetadata<'a> {
    phantom: PhantomData<&'a Node<'a>>,
    pub range: SourceRange,
}

impl<'a> NodeMetadata<'a> {
    fn build_template(template: TemplateMetadata<'a>) -> NodeMetadata<'a> {
        NodeMetadata {
            phantom: template.phantom,
            range: template.range,
        }
    }
}

/// Metadata common to all AST nodes used in templates.
/// Stored inside template notes and may be constructed directly by users.
#[derive(Debug, Clone)]
pub struct TemplateMetadata<'a> {
    pub phantom: PhantomData<&'a Node<'a>>,
    pub range: SourceRange,
}

impl Default for TemplateMetadata<'_> {
    fn default() -> Self {
        Self {
            phantom: Default::default(),
            range: SourceRange {
                file: SourceId::INVALID,
                start: SourceLoc::invalid(),
                end: SourceLoc::invalid(),
            },
        }
    }
}

/// JS identifier represented as valid UTF-8.
pub type NodeLabel = Atom;

/// A list of nodes owned by a path.
pub type NodeList<'a> = Vec<&'a Node<'a>>;

/// JS string literals don't have to contain valid UTF-8,
/// so we wrap a `Vec<u16>`, which allows us to represent UTF-16 characters
/// without being subject to Rust's restrictions on [`String`].
#[derive(Clone)]
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

impl<'gc> Node<'gc> {
    pub fn visit<V: Visitor<'gc>>(
        &'gc self,
        ctx: &'gc GCLock,
        visitor: &mut V,
        path: Option<Path<'gc>>,
    ) {
        visitor.call(ctx, self, path);
    }

    /// Visit this node with `visitor` and return the modified root node.
    /// If the root node is to be removed, return `None`.
    pub fn visit_mut<V: VisitorMut<'gc>>(
        &'gc self,
        ctx: &'gc GCLock,
        visitor: &mut V,
        path: Option<Path<'gc>>,
    ) -> Option<&'gc Node<'gc>> {
        match visitor.call(ctx, self, path) {
            TransformResult::Unchanged => Some(self),
            TransformResult::Removed => None,
            TransformResult::Changed(new_node) => Some(new_node),
            TransformResult::Expanded(_) => {
                panic!("Attempt to replace a single node with multiple");
            }
        }
    }
}

/// Trait implemented by possible child types of `NodeKind`.
trait NodeChild<'gc>
where
    Self: std::marker::Sized,
{
    type Out;

    /// Visit this child of the given `node`.
    /// Should be no-op for any type that doesn't contain pointers to other
    /// `Node`s.
    fn visit_child<V: Visitor<'gc>>(self, _ctx: &'gc GCLock, _visitor: &mut V, _path: Path<'gc>) {}

    /// Visit this child of the given `node`.
    /// Should be no-op for any type that doesn't contain pointers to other
    /// `Node`s.
    fn visit_child_mut<V: VisitorMut<'gc>>(
        self,
        _ctx: &'gc GCLock,
        _visitor: &mut V,
        _path: Path<'gc>,
    ) -> TransformResult<Self::Out> {
        TransformResult::Unchanged
    }

    /// A way to "clone" children without actually implementing Clone.
    /// Not implementing Clone prevents accidental creation of `Node` references
    /// on the stack.
    /// Can't provide a default implementation here because associated type default definitions
    /// (for `Out`) are unstable.
    fn duplicate(self) -> Self::Out;
}

impl NodeChild<'_> for f64 {
    type Out = Self;
    fn duplicate(self) -> Self::Out {
        self
    }
}
impl NodeChild<'_> for bool {
    type Out = Self;
    fn duplicate(self) -> Self::Out {
        self
    }
}
impl NodeChild<'_> for NodeLabel {
    type Out = Self;
    fn duplicate(self) -> Self::Out {
        self
    }
}
impl NodeChild<'_> for UnaryExpressionOperator {
    type Out = Self;
    fn duplicate(self) -> Self::Out {
        self
    }
}
impl NodeChild<'_> for BinaryExpressionOperator {
    type Out = Self;
    fn duplicate(self) -> Self::Out {
        self
    }
}
impl NodeChild<'_> for LogicalExpressionOperator {
    type Out = Self;
    fn duplicate(self) -> Self::Out {
        self
    }
}
impl NodeChild<'_> for UpdateExpressionOperator {
    type Out = Self;
    fn duplicate(self) -> Self::Out {
        self
    }
}
impl NodeChild<'_> for AssignmentExpressionOperator {
    type Out = Self;
    fn duplicate(self) -> Self::Out {
        self
    }
}
impl NodeChild<'_> for VariableDeclarationKind {
    type Out = Self;
    fn duplicate(self) -> Self::Out {
        self
    }
}
impl NodeChild<'_> for PropertyKind {
    type Out = Self;
    fn duplicate(self) -> Self::Out {
        self
    }
}
impl NodeChild<'_> for MethodDefinitionKind {
    type Out = Self;
    fn duplicate(self) -> Self::Out {
        self
    }
}
impl NodeChild<'_> for ImportKind {
    type Out = Self;
    fn duplicate(self) -> Self::Out {
        self
    }
}
impl NodeChild<'_> for ExportKind {
    type Out = Self;
    fn duplicate(self) -> Self::Out {
        self
    }
}
impl NodeChild<'_> for NodeString {
    type Out = Self;
    fn duplicate(self) -> Self::Out {
        self
    }
}
impl<'gc> NodeChild<'gc> for &NodeString {
    type Out = NodeString;
    fn duplicate(self) -> Self::Out {
        self.clone()
    }
}
impl NodeChild<'_> for &Option<NodeString> {
    type Out = Option<NodeString>;
    fn duplicate(self) -> Self::Out {
        self.clone()
    }
}

impl<'gc, T: NodeChild<'gc> + NodeChild<'gc, Out = T>> NodeChild<'gc> for Option<T> {
    type Out = Self;

    fn visit_child<V: Visitor<'gc>>(self, ctx: &'gc GCLock, visitor: &mut V, path: Path<'gc>) {
        if let Some(t) = self {
            t.visit_child(ctx, visitor, path);
        }
    }

    /// Visit this child of the given `node`.
    /// Should be no-op for any type that doesn't contain pointers to other
    /// `Node`s.
    fn visit_child_mut<V: VisitorMut<'gc>>(
        self,
        ctx: &'gc GCLock,
        visitor: &mut V,
        path: Path<'gc>,
    ) -> TransformResult<Self::Out> {
        use TransformResult::*;
        match self {
            None => Unchanged,
            Some(inner) => match inner.visit_child_mut(ctx, visitor, path) {
                Unchanged => Unchanged,
                Removed => Changed(None),
                Changed(new_node) => Changed(Some(new_node)),
                Expanded(_) => {
                    panic!("Attempt to replace a single optional node with multiple");
                }
            },
        }
    }

    fn duplicate(self) -> Self::Out {
        self.map(|inner| inner.duplicate())
    }
}

impl<'gc> NodeChild<'gc> for &Option<&'gc Node<'gc>> {
    type Out = Option<&'gc Node<'gc>>;

    fn visit_child<V: Visitor<'gc>>(self, ctx: &'gc GCLock, visitor: &mut V, path: Path<'gc>) {
        if let Some(t) = *self {
            t.visit_child(ctx, visitor, path);
        }
    }

    /// Visit this child of the given `node`.
    /// Should be no-op for any type that doesn't contain pointers to other
    /// `Node`s.
    fn visit_child_mut<V: VisitorMut<'gc>>(
        self,
        ctx: &'gc GCLock,
        visitor: &mut V,
        path: Path<'gc>,
    ) -> TransformResult<Self::Out> {
        use TransformResult::*;
        match self {
            None => Unchanged,
            Some(inner) => match inner.visit_child_mut(ctx, visitor, path) {
                Unchanged => Unchanged,
                Removed => Changed(None),
                Changed(new_node) => Changed(Some(new_node)),
                Expanded(_) => {
                    panic!("Attempt to replace a single optional node with multiple");
                }
            },
        }
    }

    fn duplicate(self) -> Self::Out {
        *self
    }
}

impl<'gc> NodeChild<'gc> for &'gc Node<'gc> {
    type Out = Self;

    fn visit_child<V: Visitor<'gc>>(self, ctx: &'gc GCLock, visitor: &mut V, path: Path<'gc>) {
        visitor.call(ctx, self, Some(path));
    }

    /// Visit this child of the given `node`.
    /// Should be no-op for any type that doesn't contain pointers to other
    /// `Node`s.
    fn visit_child_mut<V: VisitorMut<'gc>>(
        self,
        ctx: &'gc GCLock,
        visitor: &mut V,
        path: Path<'gc>,
    ) -> TransformResult<Self::Out> {
        match visitor.call(ctx, self, Some(path)) {
            TransformResult::Removed => {
                TransformResult::Changed(builder::EmptyStatement::build_template(
                    ctx,
                    template::EmptyStatement {
                        metadata: TemplateMetadata {
                            phantom: Default::default(),
                            range: SourceRange {
                                file: self.range().file,
                                start: self.range().start,
                                end: self.range().start,
                            },
                        },
                    },
                ))
            }
            result => result,
        }
    }

    fn duplicate(self) -> Self::Out {
        self
    }
}

impl<'gc> NodeChild<'gc> for &NodeList<'gc> {
    type Out = NodeList<'gc>;

    fn visit_child<V: Visitor<'gc>>(self, ctx: &'gc GCLock, visitor: &mut V, path: Path<'gc>) {
        for child in self {
            visitor.call(ctx, *child, Some(path));
        }
    }

    fn visit_child_mut<V: VisitorMut<'gc>>(
        self,
        ctx: &'gc GCLock,
        visitor: &mut V,
        path: Path<'gc>,
    ) -> TransformResult<Self::Out> {
        use TransformResult::*;
        let mut index = 0;
        let len = self.len();
        // Assume no copies to start.
        while index < len {
            let node = visitor.call(ctx, self[index], Some(path));
            if let Unchanged = node {
                index += 1;
                continue;
            }
            // First change found, either removed or changed node.
            let mut result: Self::Out = vec![];
            // Fill in the elements we skippped.
            for elem in self.iter().take(index) {
                result.push(elem);
            }
            // If the node was changed or expanded, push it.
            match node {
                Changed(new_node) => result.push(new_node),
                Expanded(new_nodes) => {
                    for node in new_nodes {
                        result.push(node);
                    }
                }
                Removed => {}
                Unchanged => {
                    unreachable!("checked for unchanged above")
                }
            };
            index += 1;
            // Fill the rest of the elements.
            while index < len {
                match visitor.call(ctx, self[index], Some(path)) {
                    Unchanged => result.push(self[index]),
                    Removed => {}
                    Changed(new_node) => result.push(new_node),
                    Expanded(new_nodes) => {
                        for node in new_nodes {
                            result.push(node);
                        }
                    }
                }
                index += 1;
            }
            return Changed(result);
        }
        Unchanged
    }

    fn duplicate(self) -> Self::Out {
        self.clone()
    }
}

impl<'gc> NodeChild<'gc> for &Option<NodeList<'gc>> {
    type Out = Option<NodeList<'gc>>;

    fn visit_child<V: Visitor<'gc>>(self, ctx: &'gc GCLock, visitor: &mut V, path: Path<'gc>) {
        if let Some(list) = self {
            for child in list {
                visitor.call(ctx, *child, Some(path));
            }
        }
    }

    fn visit_child_mut<V: VisitorMut<'gc>>(
        self,
        ctx: &'gc GCLock,
        visitor: &mut V,
        path: Path<'gc>,
    ) -> TransformResult<Self::Out> {
        use TransformResult::*;
        match self.as_ref() {
            None => Unchanged,
            Some(inner) => match inner.visit_child_mut(ctx, visitor, path) {
                Unchanged => Unchanged,
                Removed => Changed(None),
                Changed(new_node) => Changed(Some(new_node)),
                Expanded(_) => {
                    unreachable!("NodeList::visit_child_mut cannot return Expanded");
                }
            },
        }
    }

    fn duplicate(self) -> Self::Out {
        self.clone()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::collections::HashMap;

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

    #[test]
    fn test_node_ref() {
        let mut ctx = Context::new();
        let lock = GCLock::new(&mut ctx);

        let mut m = HashMap::new();

        let n1 = NodePtr::from(builder::NumericLiteral::build_template(
            &lock,
            template::NumericLiteral {
                metadata: Default::default(),
                value: 10.0,
            },
        ));
        let n2 = NodePtr::from(builder::NumericLiteral::build_template(
            &lock,
            template::NumericLiteral {
                metadata: Default::default(),
                value: 20.0,
            },
        ));
        let n25 = Box::new(n2);
        assert_ne!(n1, n2);
        assert_eq!(n2, *n25);
        m.insert(n1, 10);
        m.insert(n2, 20);

        assert_eq!(10, *m.get(&n1).unwrap());
        assert_eq!(20, *m.get(&n2).unwrap());
        assert_eq!(20, *m.get(&*n25).unwrap());
    }
}
