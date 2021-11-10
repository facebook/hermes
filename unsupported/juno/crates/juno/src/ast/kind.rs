/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use super::*;

/// Generate boilerplate code for the `Node` enum.
///
/// The `Node` enum has variants for each of the different types of AST nodes
/// listed in [`nodekind_defs`].
/// Each variant contains a struct of the same name, which is where the data is actually
/// stored. These member structs' first fields contain data which is common to all kinds of nodes
/// (e.g. `range`, which represents location info).
/// Force each of the member structs as well as the `Node` enum to have `repr(C)`
/// to ensure that the shared fields are in the same place
/// in all the structs. This means that the identical match arms will be optimized
/// away into some fast pointer arithmetic that's easy to inline.
macro_rules! gen_nodekind_enum {
    ($name:ident {
        $(
            $kind:ident $([ $parent:ident ])? $({
                $(
                    $field:ident : $type:ty
                    $( [ $( $constraint:ident ),* ] )?
                ),*
                $(,)?
            })?
        ),*
        $(,)?
    }) => {
        // The kind of an AST node.
        // Matching against this enum allows traversal of the AST.
        // Each variant of the enum must only have fields of the following types:
        // * `NodeRc`
        // * `Option<NodeRc>`
        // * `NodeList`
        // * `StringLiteral`
        // * `NodeLabel`
        // * `bool`
        // * `f64`
        #[derive(Debug)]
        #[repr(C)]
        pub enum Node<'a> {
            // Create each field in the enum.
            $($kind($kind<'a>),)*
        }

        $(
        #[derive(Debug)]
        #[repr(C)]
        pub struct $kind<'a> {
            // Common fields to all node kinds.
            pub metadata: NodeMetadata<'a>,
            // Create each field that's meant for just this node kind.
            $($(pub $field : $type,)*)?
        }
        )*

        impl<'gc> Node<'gc> {
            pub fn variant(&self) -> NodeVariant {
                match self {
                    $(
                        Self::$kind { .. } => NodeVariant::$kind
                    ),*
                }
            }

            /// Visit the child fields of `self`.
            pub fn visit_children<'ast: 'gc, V: Visitor<'gc>>(
                &'gc self,
                ctx: &'gc GCLock<'ast, '_>,
                visitor: &mut V,
            ) {
                match self {
                    $(
                        Node::$kind($kind {
                            $($($field,)*)?
                            ..
                        }) => {
                            $($(
                                $field.visit_child(ctx, visitor, self);
                            )*)?
                        }
                    ),*
                }
            }

            /// Visit the child fields of this node.
            /// `self` is the *original* parent of the children to visit.
            /// Will only allocate a new node if one of the children was changed.
            pub fn visit_children_mut<'ast: 'gc, V: VisitorMut<'gc>>(
                &'gc self,
                ctx: &'gc GCLock<'ast, '_>,
                visitor: &mut V,
            ) -> TransformResult<&'gc Node<'gc>> {
                let builder = builder::Builder::from_node(self);
                #[allow(unused_mut)]
                match builder {
                    $(
                        builder::Builder::$kind(mut builder) => {
                            $($(
                                if let TransformResult::Changed($field) = (&builder.inner.$field)
                                    .visit_child_mut(ctx, visitor, self) {
                                    builder.$field($field);
                                }
                            )*)?
                            builder.build(ctx)
                        }
                    ),*
                }
            }

            /// Replace `self` in the AST with the result of the `builder`.
            /// Always allocates a new node.
            /// `self` is the *original* parent of the children to visit,
            /// but the children which will be visited are determined via `builder`.
            pub fn replace_with_new<'ast: 'gc, V: VisitorMut<'gc>>(
                &'gc self,
                builder: builder::Builder<'gc>,
                ctx: &'gc GCLock<'ast, '_>,
                visitor: &mut V,
            ) -> TransformResult<&'gc Node<'gc>> {
                #[allow(unused_mut)]
                match builder {
                    $(
                        builder::Builder::$kind(mut builder) => {
                            $($(
                                if let TransformResult::Changed($field) = (&builder.inner.$field)
                                    .visit_child_mut(ctx, visitor, self) {
                                    builder.$field($field);
                                }
                            )*)?
                            builder.build(ctx)
                        }
                    ),*
                }
            }

            /// Replace `self` in the AST with the `existing` node.
            /// `self` is the *original* parent of the children to visit,
            /// but the children which will be visited are determined via `existing`.
            /// Will only allocate a new node if the `existing` node also must be modified.
            pub fn replace_with_existing<'ast: 'gc, V: VisitorMut<'gc>>(
                &'gc self,
                existing: &'gc Node<'gc>,
                ctx: &'gc GCLock<'ast, '_>,
                visitor: &mut V,
            ) -> TransformResult<&'gc Node<'gc>> {
                let builder = builder::Builder::from_node(existing);
                #[allow(unused_mut)]
                match builder {
                    $(
                        builder::Builder::$kind(mut builder) => {
                            $($(
                                if let TransformResult::Changed($field) = (&builder.inner.$field)
                                    .visit_child_mut(ctx, visitor, self) {
                                    builder.$field($field);
                                }
                            )*)?
                            match builder.build(ctx) {
                                TransformResult::Unchanged => TransformResult::Changed(existing),
                                TransformResult::Removed => {
                                    unreachable!("Builder can't remove a node");
                                }
                                TransformResult::Changed(n) => TransformResult::Changed(n),
                            }
                        }
                    ),*
                }
            }

            #[inline]
            pub fn range(&self) -> &SourceRange {
                match self {
                    $(
                        Self::$kind($kind { metadata, .. }) => &metadata.range
                    ),*
                }
            }

            #[inline]
            pub fn range_mut(&mut self) -> &mut SourceRange {
                match self {
                    $(
                        Self::$kind($kind { metadata, .. }) => &mut metadata.range
                    ),*
                }
            }

            pub fn name(&self) -> &'static str {
                match self {
                    $(
                        Self::$kind { .. } => {
                            stringify!($kind)
                        }
                    ),*
                }
            }
        }

        /// Just type information on the node without any of the children.
        /// Used for performing tasks based only on the type of the AST node
        /// without having to know more about it.
        /// Includes "abstract" nodes which cannot be truly constructed.
        #[derive(Debug, Copy, Clone, Eq, PartialEq)]
        pub enum NodeVariant {
            Expression,
            Statement,
            Declaration,
            Literal,
            Pattern,
            LVal,
            Flow,
            FlowType,
            FlowDeclaration,
            FlowExpression,
            FlowPredicate,
            FlowEnumBody,
            $($kind),*
        }

        impl NodeVariant {
            /// The `parent` of the variant in ESTree, used for validation.
            /// Return `None` if there is no parent.
            pub fn parent(&self) -> Option<NodeVariant> {
                match self {
                    Self::Expression => None,
                    Self::Statement => None,
                    Self::Declaration => Some(Self::Statement),
                    Self::Literal => Some(Self::Expression),
                    Self::Pattern => Some(Self::LVal),
                    Self::LVal => Some(Self::Expression),
                    Self::Flow => None,
                    Self::FlowType => Some(Self::Flow),
                    Self::FlowDeclaration => Some(Self::Declaration),
                    Self::FlowExpression => Some(Self::Expression),
                    Self::FlowPredicate => Some(Self::Flow),
                    Self::FlowEnumBody => None,
                    $(
                        Self::$kind => {
                            None$(.or(Some(Self::$parent)))?
                        }
                    ),*
                }
            }
        }

        pub mod template {
            use super::{
                AssignmentExpressionOperator,
                BinaryExpressionOperator,
                ExportKind,
                ImportKind,
                LogicalExpressionOperator,
                MethodDefinitionKind,
                Node,
                NodeLabel,
                NodeList,
                NodeString,
                PropertyKind,
                UnaryExpressionOperator,
                UpdateExpressionOperator,
                VariableDeclarationKind,
            };
            $(
            #[derive(Debug, Clone)]
            #[repr(C)]
            pub struct $kind<'a> {
                // Common fields to all node kinds.
                pub metadata: super::TemplateMetadata<'a>,
                // Create each field that's meant for just this node kind.
                $($(pub $field : $type,)*)?
            }
            )*
        }

        pub mod builder {
            use super::{
                AssignmentExpressionOperator,
                BinaryExpressionOperator,
                ExportKind,
                GCLock,
                ImportKind,
                LogicalExpressionOperator,
                MethodDefinitionKind,
                Node,
                NodeChild,
                NodeLabel,
                NodeList,
                NodeMetadata,
                NodeString,
                PropertyKind,
                TransformResult,
                UnaryExpressionOperator,
                UpdateExpressionOperator,
                VariableDeclarationKind,
            };

            /// Used for building various kinds of AST nodes.
            #[derive(Debug)]
            pub enum Builder<'a> {
                // Create each field in the enum.
                $($kind(self::$kind<'a>),)*
            }

            impl<'a> Builder<'a> {
                pub fn from_node(node: &'a Node<'a>) -> Self {
                    match node {
                        $(
                        Node::$kind(original) => Self::$kind(self::$kind::from_node(original)),
                        )*
                    }
                }
            }

            $(
            /// Used for building nodes.
            #[derive(Debug)]
            pub struct $kind<'a> {
                is_changed: bool,
                pub(super) inner: super::$kind<'a>,
            }

            impl<'a> $kind<'a> {
                /// Initialize the builder from `node`.
                pub fn from_node(node: &'a super::$kind<'a>) -> Self {
                    Self {
                        is_changed: false,
                        inner: super::$kind {
                            metadata: NodeMetadata {
                                phantom: node.metadata.phantom,
                                range: node.metadata.range,
                            },
                            $($(
                                $field: (&node.$field).duplicate(),
                            )*)?
                        }
                    }
                }

                /// Return Unchanged if the node the builder started with was never changed.
                /// Return Changed(node) with a new node if it was changed.
                pub fn build(self, gc: &'a GCLock) -> TransformResult<&'a Node<'a>> {
                    if self.is_changed {
                        TransformResult::Changed(gc.alloc(super::Node::$kind(self.inner)))
                    } else {
                        TransformResult::Unchanged
                    }
                }

                /// Build from a template.
                pub fn build_template(
                    gc: &'a GCLock,
                    node: super::template::$kind<'a>,
                ) -> &'a Node<'a> {
                    gc.alloc(super::Node::$kind(super::$kind {
                        metadata: NodeMetadata::build_template(node.metadata),
                        $($(
                                $field: node.$field,
                        )*)?
                    }))
                }

                // Setters for the fields.
                $($(
                pub fn $field(&mut self, $field: $type) {
                    self.is_changed = true;
                    self.inner.$field = $field;
                }
                )*)?
            }
            )*

        }

    };
}

nodekind_defs! { gen_nodekind_enum }

impl<'gc> Node<'gc> {
    /// Shallow equality comparison.
    pub fn ptr_eq(&self, other: &'_ Node<'_>) -> bool {
        std::ptr::eq(self, other)
    }

    fn panic(&self, msg: &str) -> ! {
        panic!("{} {}", self.name(), msg)
    }
    fn function_like_panic(&self) -> ! {
        self.panic("is not function-like")
    }
    pub fn is_function_like(&self) -> bool {
        matches!(
            self,
            Node::FunctionExpression(..)
                | Node::ArrowFunctionExpression(..)
                | Node::FunctionDeclaration(..)
        )
    }
    pub fn function_like_id(&self) -> Option<&Node> {
        match self {
            Node::FunctionExpression(FunctionExpression { id, .. })
            | Node::ArrowFunctionExpression(ArrowFunctionExpression { id, .. })
            | Node::FunctionDeclaration(FunctionDeclaration { id, .. }) => *id,
            _ => self.function_like_panic(),
        }
    }
    pub fn function_like_params(&self) -> &NodeList {
        match self {
            Node::FunctionExpression(FunctionExpression { params, .. })
            | Node::ArrowFunctionExpression(ArrowFunctionExpression { params, .. })
            | Node::FunctionDeclaration(FunctionDeclaration { params, .. }) => params,
            _ => self.function_like_panic(),
        }
    }
    pub fn function_like_body(&self) -> &Node {
        match self {
            Node::FunctionExpression(FunctionExpression { body, .. })
            | Node::ArrowFunctionExpression(ArrowFunctionExpression { body, .. })
            | Node::FunctionDeclaration(FunctionDeclaration { body, .. }) => body,
            _ => self.function_like_panic(),
        }
    }
}

#[macro_export]
macro_rules! node_cast {
    ($kind:path, $node:expr) => {
        match $node {
            $kind(v) => v,
            _ => panic!(
                "invalid cast to {} from {}",
                stringify!($kind),
                $node.name()
            ),
        }
    };
}
#[macro_export]
macro_rules! node_isa {
    ($kind:path, $node:expr) => {
        match $node {
            $kind(_) => true,
            _ => false,
        }
    };
}
