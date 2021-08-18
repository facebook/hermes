/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use super::{Node, NodeKind, NodeLabel, NodeList, NodePtr, NodeVariant, StringLiteral};

macro_rules! gen_validate_fn {
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
            /// Check whether this is a valid kind for `node`.
            pub fn validate_node<'n>(node: &'n Node) -> bool {
                (match &node.kind {
                    $(
                        NodeKind::$kind $({$($field),*})? => {
                            // Run the validation for each child.
                            // Use `true &&` to make it work when there's no children.
                            true $(&& $(
                                $field.validate_child(node, &[$($(NodeVariant::$constraint),*)?])
                            )&&*)?
                        }
                    ),*
                }) && validate_custom(node)
            }
    }
}

nodekind_defs! { gen_validate_fn }

trait ValidChild {
    /// Check whether this is a valid child of `node` given the constraints.
    fn validate_child(&self, node: &Node, constraints: &[NodeVariant]) -> bool;
}

impl ValidChild for f64 {
    fn validate_child(&self, _node: &Node, _constraints: &[NodeVariant]) -> bool {
        true
    }
}

impl ValidChild for bool {
    fn validate_child(&self, _node: &Node, _constraints: &[NodeVariant]) -> bool {
        true
    }
}

impl ValidChild for NodeLabel {
    fn validate_child(&self, _node: &Node, _constraints: &[NodeVariant]) -> bool {
        true
    }
}

impl ValidChild for StringLiteral {
    fn validate_child(&self, _node: &Node, _constraints: &[NodeVariant]) -> bool {
        true
    }
}

impl<T: ValidChild> ValidChild for Option<T> {
    fn validate_child(&self, node: &Node, constraints: &[NodeVariant]) -> bool {
        match self {
            None => true,
            Some(t) => t.validate_child(node, constraints),
        }
    }
}

impl ValidChild for NodePtr {
    fn validate_child(&self, _node: &Node, constraints: &[NodeVariant]) -> bool {
        for &constraint in constraints {
            if instanceof(self.kind.variant(), constraint) {
                return true;
            }
        }
        false
    }
}

impl ValidChild for NodeList {
    fn validate_child(&self, _node: &Node, constraints: &[NodeVariant]) -> bool {
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

/// Custom validation function for constraints which can't be expressed
/// using just the inheritance structure in NodeKind.
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
            if !*computed
                && !key.validate_child(node, &[NodeVariant::Identifier, NodeVariant::Literal])
            {
                return false;
            }
            if *method && !value.validate_child(node, &[NodeVariant::FunctionExpression]) {
                return false;
            }
            if (kind.str == "get" || kind.str == "set")
                && !value.validate_child(node, &[NodeVariant::FunctionExpression])
            {
                return false;
            }
            true
        }

        _ => true,
    }
}
