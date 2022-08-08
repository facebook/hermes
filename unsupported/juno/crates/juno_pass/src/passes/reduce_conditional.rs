/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//! Pass for optimizing conditional expressions where the conditional is a literal.
//!
//! Operates on both conditional expressions and `if` statements (with or without `else`).
//! For example, transforms
//! ```js
//! true ? a : b
//! ```
//! into
//! ```js
//! a
//! ```

use juno::ast::*;

use crate::Pass;

#[derive(Default)]
pub struct ReduceConditional {}

impl ReduceConditional {
    pub fn new() -> Self {
        Default::default()
    }
}

impl Pass for ReduceConditional {
    fn name(&self) -> &'static str {
        "Reduce conditional"
    }
    fn description(&self) -> &'static str {
        "Transforms literal conditionals into whichever branch is taken"
    }
    fn run<'gc>(
        &mut self,
        gc: &'gc GCLock<'_, '_>,
        node: &'gc Node<'gc>,
    ) -> TransformResult<&'gc Node<'gc>> {
        VisitorMut::call(self, gc, node, None)
    }
}

impl<'gc> VisitorMut<'gc> for ReduceConditional {
    fn call(
        &mut self,
        lock: &'gc GCLock,
        node: &'gc Node<'gc>,
        _parent: Option<Path<'gc>>,
    ) -> TransformResult<&'gc Node<'gc>> {
        match node {
            Node::ConditionalExpression(ConditionalExpression {
                test: Node::BooleanLiteral(BooleanLiteral { value, .. }),
                consequent,
                alternate,
                ..
            })
            | Node::IfStatement(IfStatement {
                test: Node::BooleanLiteral(BooleanLiteral { value, .. }),
                consequent,
                alternate: Some(alternate),
                ..
            }) => {
                node.replace_with_existing(if *value { consequent } else { alternate }, lock, self)
            }
            Node::IfStatement(IfStatement {
                test: Node::BooleanLiteral(BooleanLiteral { value, .. }),
                consequent,
                alternate: None,
                ..
            }) => {
                if *value {
                    node.replace_with_existing(consequent, lock, self)
                } else {
                    TransformResult::Removed
                }
            }
            _ => node.visit_children_mut(lock, self),
        }
    }
}
