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
        // * `NodePtr`
        // * `Option<NodePtr>`
        // * `NodeList`
        // * `StringLiteral`
        // * `NodeLabel`
        // * `bool`
        // * `f64`
        #[derive(Debug)]
        #[repr(C)]
        pub enum Node {
            // Create each field in the enum.
            $($kind($kind),)*
        }


        $(
        #[derive(Debug)]
        #[repr(C)]
        pub struct $kind {
            // Common fields to all node kinds.
            pub range: SourceRange,
            // Create each field that's meant for just this node kind.
            $($(pub $field : $type,)*)?
        }
        )*

        impl Node {
            /// Visit the child fields of this kind.
            /// `node` is the node for which this is the kind.
            pub fn visit_children<V: Visitor>(&self, ctx: &Context, visitor: &mut V, node: NodePtr) {
                match self {
                    $(
                        Self::$kind($kind {
                            $($($field,)*)?
                            ..
                        }) => {
                            $($(
                                $field.visit(ctx, visitor, node);
                            )*)?
                        }
                    ),*
                }
            }

            pub fn variant(&self) -> NodeVariant {
                match self {
                    $(
                        Self::$kind { .. } => NodeVariant::$kind
                    ),*
                }
            }

            #[inline]
            pub fn range(&self) -> &SourceRange {
                match self {
                    $(
                        Self::$kind($kind { range, .. }) => range
                    ),*
                }
            }

            #[inline]
            pub fn range_mut(&mut self) -> &mut SourceRange {
                match self {
                    $(
                        Self::$kind($kind { range, .. }) => range
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
    };
}

nodekind_defs! { gen_nodekind_enum }
