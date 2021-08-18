/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use super::{instanceof, Node, NodeChild, NodeLabel, NodeList, NodePtr, StringLiteral, Visitor};

/// Generate boilerplate code for the `NodeKind` enum.
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
        pub enum NodeKind {
            // Create each field in the enum.
            $(
                $kind $({
                    $($field : $type),*
                })?
            ),*
        }

        impl NodeKind {
            /// Visit the child fields of this kind.
            /// `node` is the node for which this is the kind.
            pub fn visit_children<V: Visitor>(&self, visitor: &mut V, node: &Node) {
                match self {
                    $(
                        Self::$kind $({$($field),*})? => {
                            $($(
                                $field.visit(visitor, node);
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

            /// Check whether this is a valid kind for `node`.
            pub fn validate<'n>(&self, node: &'n Node) -> bool {
                (match self {
                    $(
                        Self::$kind $({$($field),*})? => {
                            // Run the validation for each child.
                            // Use `true &&` to make it work when there's no children.
                            true $(&& $(
                                $field.validate(node, &[$($(NodeVariant::$constraint),*)?])
                            )&&*)?
                        }
                    ),*
                }) && validate_custom(node)
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

fn validate_custom(node: &Node) -> bool {
    use NodeKind::*;
    match &node.kind {
        MemberExpression {
            property,
            object: _,
            computed,
        }
        | OptionalMemberExpression {
            property,
            object: _,
            computed,
            optional: _,
        } => {
            if *computed {
                instanceof(property.kind.variant(), NodeVariant::Expression)
            } else {
                instanceof(property.kind.variant(), NodeVariant::Identifier)
            }
        }

        Property {
            key,
            value,
            kind,
            computed,
            method,
            shorthand,
        } => {
            if *computed && *shorthand {
                return false;
            }
            if !*computed && !key.validate(node, &[NodeVariant::Identifier, NodeVariant::Literal]) {
                return false;
            }
            if *method && !value.validate(node, &[NodeVariant::FunctionExpression]) {
                return false;
            }
            if (kind.str == "get" || kind.str == "set")
                && !value.validate(node, &[NodeVariant::FunctionExpression])
            {
                return false;
            }
            true
        }

        _ => true,
    }
}
