/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use super::*;
use paste::paste;

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
        // * `NodePtr`
        // * `Option<NodePtr>`
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

        paste! {
        $(
        #[derive(Debug, Clone)]
        #[repr(C)]
        pub struct [<$kind Template>]<'a> {
            // Common fields to all node kinds.
            pub metadata: TemplateMetadata<'a>,
            // Create each field that's meant for just this node kind.
            $($(pub $field : $type,)*)?
        }
        )*
        }


        impl<'gc> Node<'gc> {
            pub fn variant(&self) -> NodeVariant {
                match self {
                    $(
                        Self::$kind { .. } => NodeVariant::$kind
                    ),*
                }
            }

            /// Visit the child fields of `self`.
            pub fn visit_children<'ast: 'gc, V: Visitor>(
                &'gc self,
                ctx: &'gc GCContext<'ast, '_>,
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

            /// Visit the child fields of the provided `builder`.
            /// `self` is the *original* parent of the children to visit,
            /// but the children which will be visited are determined via `builder`.
            pub fn visit_children_mut<'ast: 'gc, V: VisitorMut>(
                &'gc self,
                builder: NodeBuilder<'gc>,
                ctx: &'gc GCContext<'ast, '_>,
                visitor: &mut V,
            ) -> TransformResult<&'gc Node<'gc>> {
                #[allow(unused_mut)]
                match builder {
                    $(
                        NodeBuilder::$kind(mut builder) => {
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
                    $(
                        Self::$kind => {
                            None$(.or(Some(Self::$parent)))?
                        }
                    ),*
                }
            }
        }

        paste! {

        /// Used for building various kinds of AST nodes.
        #[derive(Debug)]
        pub enum NodeBuilder<'a> {
            // Create each field in the enum.
            $($kind([<$kind Builder>]<'a>),)*
        }

        impl<'a> NodeBuilder<'a> {
            pub fn from_node(node: &'a Node<'a>) -> Self {
                match node {
                    $(
                    Node::$kind(original) => Self::$kind([<$kind Builder>]::from_node(original)),
                    )*
                }
            }
        }

        $(
        /// Used for building nodes.
        #[derive(Debug)]
        pub struct [<$kind Builder>]<'a> {
            is_changed: bool,
            pub(super) inner: $kind<'a>,
        }

        impl<'a> [<$kind Builder>]<'a> {
            /// Initialize the builder from `node`.
            pub fn from_node(node: &'a $kind<'a>) -> Self {
                Self {
                    is_changed: false,
                    inner: $kind {
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
            pub fn build(self, gc: &'a GCContext<'_, '_>) -> TransformResult<&'a Node<'a>> {
                if self.is_changed {
                    TransformResult::Changed(gc.alloc(Node::$kind(self.inner)))
                } else {
                    TransformResult::Unchanged
                }
            }

            /// Build from a template.
            pub fn build_template(
                gc: &'a GCContext<'_, '_>,
                node: [<$kind Template>]<'a>,
            ) -> &'a Node<'a> {
                [<$kind Builder>] {
                    is_changed: true,
                    inner: $kind {
                        metadata: NodeMetadata::build_template(node.metadata),
                        $($(
                                $field: node.$field,
                        )*)?
                    }
                }.build(gc).unwrap()
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

        } // paste

    };
}

nodekind_defs! { gen_nodekind_enum }
