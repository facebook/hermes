/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use super::{
    kind::*, AssignmentExpressionOperator, BinaryExpressionOperator, Context, ExportKind,
    ImportKind, LogicalExpressionOperator, MethodDefinitionKind, NodeKind, NodeLabel, NodeList,
    NodePtr, NodeString, NodeVariant, PropertyKind, UnaryExpressionOperator,
    UpdateExpressionOperator, VariableDeclarationKind, Visitor,
};

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
            fn validate_node(ctx: &Context, node: NodePtr) -> Result<(), ValidationError> {
                match &node.get(ctx).kind {
                    $(
                        NodeKind::$kind($kind {$($($field),*)?}) => {
                            // Run the validation for each child.
                            // Use `true &&` to make it work when there's no children.
                            $($(
                                $field.validate_child(ctx, node, &[$($(NodeVariant::$constraint),*)?])?;
                            )*)?
                        }
                    ),*
                };
                validate_custom(ctx, node)?;
                Ok(())
            }
    }
}

nodekind_defs! { gen_validate_fn }

trait ValidChild {
    /// Check whether this is a valid child of `node` given the constraints.
    fn validate_child(
        &self,
        _ctx: &Context,
        _node: NodePtr,
        _constraints: &[NodeVariant],
    ) -> Result<(), ValidationError> {
        Ok(())
    }
}

impl ValidChild for f64 {}

impl ValidChild for bool {}

impl ValidChild for NodeLabel {}
impl ValidChild for UnaryExpressionOperator {}
impl ValidChild for BinaryExpressionOperator {}
impl ValidChild for LogicalExpressionOperator {}
impl ValidChild for UpdateExpressionOperator {}
impl ValidChild for AssignmentExpressionOperator {}
impl ValidChild for VariableDeclarationKind {}
impl ValidChild for PropertyKind {}
impl ValidChild for MethodDefinitionKind {}
impl ValidChild for ImportKind {}
impl ValidChild for ExportKind {}

impl ValidChild for NodeString {}

impl<T: ValidChild> ValidChild for Option<T> {
    fn validate_child<'a>(
        &self,
        ctx: &'a Context,
        node: NodePtr,
        constraints: &[NodeVariant],
    ) -> Result<(), ValidationError> {
        match self {
            None => Ok(()),
            Some(t) => t.validate_child(ctx, node, constraints),
        }
    }
}

impl ValidChild for NodePtr {
    fn validate_child<'a>(
        &self,
        ctx: &'a Context,
        node: NodePtr,
        constraints: &[NodeVariant],
    ) -> Result<(), ValidationError> {
        for &constraint in constraints {
            if instanceof(self.get(ctx).kind.variant(), constraint) {
                return Ok(());
            }
        }
        Err(ValidationError {
            node,
            message: format!("Unexpected {:?}", self.get(ctx).kind.variant()),
        })
    }
}

impl ValidChild for NodeList {
    fn validate_child<'a>(
        &self,
        ctx: &'a Context,
        node: NodePtr,
        constraints: &[NodeVariant],
    ) -> Result<(), ValidationError> {
        'elems: for elem in self {
            for &constraint in constraints {
                if instanceof(elem.get(ctx).kind.variant(), constraint) {
                    // Found a valid constraint for this element,
                    // move on to the next element.
                    continue 'elems;
                }
            }
            // Failed to find a constraint that matched, early return.
            return Err(ValidationError {
                node,
                message: format!("Unexpected {:?}", elem.get(ctx).kind.variant()),
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
fn validate_custom(ctx: &Context, node: NodePtr) -> Result<(), ValidationError> {
    match &node.get(ctx).kind {
        NodeKind::MemberExpression(MemberExpression {
            property,
            object: _,
            computed,
        })
        | NodeKind::OptionalMemberExpression(OptionalMemberExpression {
            property,
            object: _,
            computed,
            optional: _,
        }) => {
            if *computed {
                property.validate_child(ctx, node, &[NodeVariant::Expression])?;
            } else {
                property.validate_child(ctx, node, &[NodeVariant::Identifier])?;
            }
        }

        NodeKind::Property(Property {
            key,
            value,
            kind,
            computed,
            method,
            shorthand,
        }) => {
            if *computed && *shorthand {
                return Err(ValidationError {
                    node,
                    message: "Property cannot be computed and shorthand".to_string(),
                });
            }
            if !*computed {
                key.validate_child(ctx, node, &[NodeVariant::Identifier, NodeVariant::Literal])?;
            }
            if *method {
                value.validate_child(ctx, node, &[NodeVariant::FunctionExpression])?;
            }
            if *kind == PropertyKind::Get || *kind == PropertyKind::Set {
                value.validate_child(ctx, node, &[NodeVariant::FunctionExpression])?;
            }
        }

        _ => {}
    }
    Ok(())
}

/// An AST validation error.
pub struct ValidationError {
    /// The AST node which failed to validate.
    pub node: NodePtr,

    /// A description of the invalid state encountered.
    pub message: String,
}

/// Runs validation on the AST and stores errors.
struct Validator {
    /// Every error encountered so far.
    /// If empty after validation, the AST is valid.
    pub errors: Vec<ValidationError>,
}

impl Validator {
    pub fn new() -> Validator {
        Validator { errors: Vec::new() }
    }

    /// Run validation recursively starting at the `root`.
    pub fn validate_root(&mut self, ctx: &Context, root: NodePtr) {
        self.validate_node(ctx, root);
    }

    /// Validate `node` and recursively validate its children.
    fn validate_node(&mut self, ctx: &Context, node: NodePtr) {
        if let Err(e) = validate_node(ctx, node) {
            self.errors.push(e);
        }
        node.visit_children(ctx, self);
    }
}

impl Visitor for Validator {
    fn call(&mut self, ctx: &Context, node: NodePtr, _parent: Option<NodePtr>) {
        self.validate_node(ctx, node);
    }
}

/// Validate the full AST tree.
/// If it fails, return all the errors encountered along the way.
pub fn validate_tree(ctx: &Context, root: NodePtr) -> Result<(), Vec<ValidationError>> {
    let mut validator = Validator::new();
    validator.validate_root(ctx, root);
    if validator.errors.is_empty() {
        Ok(())
    } else {
        Err(validator.errors)
    }
}
