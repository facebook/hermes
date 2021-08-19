/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use juno::ast::{Node, NodeKind, NodePtr, SourceLoc, SourceRange, Visitor};

mod validate;

pub fn node(kind: NodeKind) -> Box<Node> {
    let range = SourceRange {
        file: 0,
        start: SourceLoc { line: 0, col: 0 },
        end: SourceLoc { line: 0, col: 0 },
    };

    Box::new(Node { range, kind })
}

#[test]
fn test_visit() {
    use NodeKind::*;
    // Dummy range, we don't care about ranges in this test.
    let range = SourceRange {
        file: 0,
        start: SourceLoc { line: 0, col: 0 },
        end: SourceLoc { line: 0, col: 0 },
    };
    let ast = Box::new(Node {
        range,
        kind: BlockStatement {
            body: vec![
                NodePtr::new(Node {
                    range,
                    kind: ExpressionStatement {
                        expression: Box::new(Node {
                            range,
                            kind: NumericLiteral { value: 1.0 },
                        }),
                        directive: None,
                    },
                }),
                NodePtr::new(Node {
                    range,
                    kind: ExpressionStatement {
                        expression: Box::new(Node {
                            range,
                            kind: NumericLiteral { value: 2.0 },
                        }),
                        directive: None,
                    },
                }),
            ],
        },
    });

    // Accumulates the numbers found in the AST.
    struct NumberFinder {
        acc: Vec<f64>,
    }

    impl<'a> Visitor<'a> for NumberFinder {
        fn call(&mut self, node: &'a Node, parent: Option<&'a Node>) {
            if let NumericLiteral { value } = &node.kind {
                assert!(matches!(parent.unwrap().kind, ExpressionStatement { .. }));
                self.acc.push(*value);
            }
            node.visit_children(self);
        }
    }

    let mut visitor = NumberFinder { acc: vec![] };
    ast.visit(&mut visitor, None);
    assert_eq!(visitor.acc, [1.0, 2.0]);
}
