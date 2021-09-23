/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use juno::ast::{Context, Node, NodeKind, NodePtr, Visitor};

/// Create a node with a default source range for testing.
/// Use a macro to make it easier to construct nested macros
/// by spiltting mutable borrows of the context.
/// Necessary because Rust doesn't yet support "two phase borrows" where
/// we can borrow `ctx` for just the evaluation of an argument and not
/// for the entire duration of the call.
macro_rules! make_node {
    ($ctx:expr, $kind:expr $(,)?) => {{
        use juno::ast;
        let range = ast::SourceRange {
            file: 0,
            start: ast::SourceLoc { line: 1, col: 1 },
            end: ast::SourceLoc { line: 1, col: 1 },
        };
        let node = Node { range, kind: $kind };
        $ctx.alloc(node)
    }};
}

mod validate;

#[test]
#[allow(clippy::float_cmp)]
fn test_visit() {
    use NodeKind::*;
    let mut ctx = Context::new();
    // Dummy range, we don't care about ranges in this test.
    let ast = make_node!(
        ctx,
        BlockStatement {
            body: vec![
                make_node!(
                    ctx,
                    ExpressionStatement {
                        expression: make_node!(ctx, NumericLiteral { value: 1.0 },),
                        directive: None,
                    },
                ),
                make_node!(
                    ctx,
                    ExpressionStatement {
                        expression: make_node!(ctx, NumericLiteral { value: 2.0 },),
                        directive: None,
                    },
                ),
            ],
        },
    );

    // Accumulates the numbers found in the AST.
    struct NumberFinder {
        acc: Vec<f64>,
    }

    impl Visitor for NumberFinder {
        fn call(&mut self, ctx: &Context, node: NodePtr, parent: Option<NodePtr>) {
            if let NumericLiteral { value } = &node.get(ctx).kind {
                assert!(matches!(
                    parent.unwrap().get(ctx).kind,
                    ExpressionStatement { .. }
                ));
                self.acc.push(*value);
            }
            node.visit_children(ctx, self);
        }
    }

    let mut visitor = NumberFinder { acc: vec![] };
    ast.visit(&ctx, &mut visitor, None);
    assert_eq!(visitor.acc, [1.0, 2.0]);
}
