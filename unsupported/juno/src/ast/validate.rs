/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use super::{Node, NodeKind, NodeLabel, NodeList, NodePtr, NodeVariant, StringLiteral, Visitor};

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
            fn validate_node<'n>(node: &'n Node) -> Result<(), ValidationError> {
                match &node.kind {
                    $(
                        NodeKind::$kind $({$($field),*})? => {
                            // Run the validation for each child.
                            // Use `true &&` to make it work when there's no children.
                            $($(
                                $field.validate_child(node, &[$($(NodeVariant::$constraint),*)?])?;
                            )*)?
                        }
                    ),*
                };
                validate_custom(node)?;
                Ok(())
            }
    }
}

nodekind_defs! { gen_validate_fn }

trait ValidChild {
    /// Check whether this is a valid child of `node` given the constraints.
    fn validate_child<'a>(
        &self,
        _node: &'a Node,
        _constraints: &[NodeVariant],
    ) -> Result<(), ValidationError<'a>> {
        Ok(())
    }
}

impl ValidChild for f64 {}

impl ValidChild for bool {}

impl ValidChild for NodeLabel {}

impl ValidChild for StringLiteral {}

impl<T: ValidChild> ValidChild for Option<T> {
    fn validate_child<'a>(
        &self,
        node: &'a Node,
        constraints: &[NodeVariant],
    ) -> Result<(), ValidationError<'a>> {
        match self {
            None => Ok(()),
            Some(t) => t.validate_child(node, constraints),
        }
    }
}

impl ValidChild for NodePtr {
    fn validate_child<'a>(
        &self,
        node: &'a Node,
        constraints: &[NodeVariant],
    ) -> Result<(), ValidationError<'a>> {
        for &constraint in constraints {
            if instanceof(self.kind.variant(), constraint) {
                return Ok(());
            }
        }
        Err(ValidationError {
            node,
            message: format!("Unexpected {:?}", self.kind.variant()),
        })
    }
}

impl ValidChild for NodeList {
    fn validate_child<'a>(
        &self,
        node: &'a Node,
        constraints: &[NodeVariant],
    ) -> Result<(), ValidationError<'a>> {
        'elems: for elem in self {
            for &constraint in constraints {
                if instanceof(elem.kind.variant(), constraint) {
                    // Found a valid constraint for this element,
                    // move on to the next element.
                    continue 'elems;
                }
            }
            // Failed to find a constraint that matched, early return.
            return Err(ValidationError {
                node,
                message: format!("Unexpected {:?}", elem.kind.variant()),
            });
        }
        Ok(())
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
fn validate_custom(node: &Node) -> Result<(), ValidationError> {
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
                property.validate_child(node, &[NodeVariant::Expression])?;
            } else {
                property.validate_child(node, &[NodeVariant::Identifier])?;
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
                return Err(ValidationError {
                    node,
                    message: "Property cannot be computed and shorthand".to_string(),
                });
            }
            if !*computed {
                key.validate_child(node, &[NodeVariant::Identifier, NodeVariant::Literal])?;
            }
            if *method {
                value.validate_child(node, &[NodeVariant::FunctionExpression])?;
            }
            if kind.str == "get" || kind.str == "set" {
                value.validate_child(node, &[NodeVariant::FunctionExpression])?;
            }
        }

        _ => {}
    }
    Ok(())
}

/// An AST validation error.
pub struct ValidationError<'a> {
    /// The AST node which failed to validate.
    pub node: &'a Node,

    /// A description of the invalid state encountered.
    pub message: String,
}

/// Runs validation on the AST and stores errors.
struct Validator<'a> {
    /// Every error encountered so far.
    /// If empty after validation, the AST is valid.
    pub errors: Vec<ValidationError<'a>>,
}

impl<'a> Validator<'a> {
    pub fn new() -> Validator<'a> {
        Validator { errors: Vec::new() }
    }

    /// Run validation recursively starting at the `root`.
    pub fn validate_root(&mut self, root: &'a Node) {
        self.validate_node(root);
    }

    /// Validate `node` and recursively validate its children.
    fn validate_node(&mut self, node: &'a Node) {
        if let Err(e) = validate_node(node) {
            self.errors.push(e);
        }
        node.visit_children(self);
    }
}

impl<'a> Visitor<'a> for Validator<'a> {
    fn call(&mut self, node: &'a Node, _parent: Option<&'a Node>) {
        self.validate_node(node);
    }
}

/// Validate the full AST tree.
/// If it fails, return all the errors encountered along the way.
pub fn validate_tree(root: &Node) -> Result<(), Vec<ValidationError>> {
    let mut validator = Validator::new();
    validator.validate_root(root);
    if validator.errors.is_empty() {
        Ok(())
    } else {
        Err(validator.errors)
    }
}
