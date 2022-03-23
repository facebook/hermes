/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//! Pass for optimizing addition of negated second operands.
//!
//! This pass is unsafe: if `x` is a string, then `+` is concatenation.
//!
//! Transforms
//! ```js
//! x + -y
//! ```
//! into
//! ```js
//! x - y
//! ```

use crate::Pass;
use juno::ast::*;

#[derive(Default)]
pub struct AddNegative {}

impl AddNegative {
    pub fn new() -> Self {
        Default::default()
    }
}

impl Pass for AddNegative {
    fn name(&self) -> &'static str {
        "Add negative"
    }
    fn description(&self) -> &'static str {
        "Transforms (x + -y) into (x - y)"
    }
    fn run<'gc>(
        &mut self,
        gc: &'gc GCLock<'_, '_>,
        node: &'gc Node<'gc>,
    ) -> TransformResult<&'gc Node<'gc>> {
        VisitorMut::call(self, gc, node, None)
    }
}

impl<'gc> VisitorMut<'gc> for AddNegative {
    fn call(
        &mut self,
        gc: &'gc GCLock<'_, '_>,
        node: &'gc Node<'gc>,
        _parent: Option<Path<'gc>>,
    ) -> TransformResult<&'gc Node<'gc>> {
        if let Node::BinaryExpression(
            e1 @ BinaryExpression {
                operator: BinaryExpressionOperator::Plus,
                right:
                    Node::UnaryExpression(UnaryExpression {
                        operator: UnaryExpressionOperator::Minus,
                        argument: e2,
                        ..
                    }),
                ..
            },
        ) = node
        {
            let mut builder = builder::BinaryExpression::from_node(e1);
            builder.operator(BinaryExpressionOperator::Minus);
            builder.right(e2);
            return node.replace_with_new(builder::Builder::BinaryExpression(builder), gc, self);
        }
        node.visit_children_mut(gc, self)
    }
}
