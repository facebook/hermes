/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use juno::ast::node_cast;
use juno::ast::*;
use juno::hparser;

mod validate;

#[test]
#[should_panic]
fn test_node_outlives_context() {
    let ast;
    {
        let mut ctx = Context::new();
        ast = {
            let gc = GCLock::new(&mut ctx);
            NodeRc::from_node(
                &gc,
                builder::NumericLiteral::build_template(
                    &gc,
                    template::NumericLiteral {
                        metadata: Default::default(),
                        value: 1.0f64,
                    },
                ),
            )
        };
        // Forget `ast` in order to prevent the `Drop` impl from being called on panic.
        std::mem::forget(ast);
    }
}

#[test]
#[should_panic]
#[allow(clippy::redundant_clone)]
fn test_node_clone_outlives_context() {
    let ast;
    {
        let mut ctx = Context::new();
        {
            let ast_orig = {
                let gc = GCLock::new(&mut ctx);
                NodeRc::from_node(
                    &gc,
                    builder::NumericLiteral::build_template(
                        &gc,
                        template::NumericLiteral {
                            metadata: Default::default(),
                            value: 1.0f64,
                        },
                    ),
                )
            };
            ast = ast_orig.clone();
            // Forget `ast` in order to prevent the `Drop` impl from being called on panic.
            std::mem::forget(ast);
        }
    }
}

#[test]
fn test_list_empty() {
    let mut ctx = Context::new();
    let gc = GCLock::new(&mut ctx);
    let ast = builder::Program::build_template(
        &gc,
        template::Program {
            metadata: Default::default(),
            body: NodeList::new(&gc),
        },
    );
    match ast {
        Node::Program(Program { body, .. }) => {
            assert!(body.is_empty());
            assert_eq!(body.len(), 0);
        }
        _ => unreachable!(),
    }
}

#[test]
fn test_list_elements() {
    let mut ctx = Context::new();
    let gc = GCLock::new(&mut ctx);
    let ast = builder::Program::build_template(
        &gc,
        template::Program {
            metadata: Default::default(),
            body: NodeList::from_iter(
                &gc,
                [
                    builder::NullLiteral::build_template(
                        &gc,
                        template::NullLiteral {
                            metadata: Default::default(),
                        },
                    ),
                    builder::BooleanLiteral::build_template(
                        &gc,
                        template::BooleanLiteral {
                            metadata: Default::default(),
                            value: false,
                        },
                    ),
                ],
            ),
        },
    );
    match ast {
        Node::Program(Program { body, .. }) => {
            assert!(!body.is_empty());
            assert_eq!(body.len(), 2);
            let mut it = body.iter();
            assert!(matches!(it.next(), Some(Node::NullLiteral(_))));
            assert!(matches!(
                it.next(),
                Some(Node::BooleanLiteral(BooleanLiteral { value: false, .. }))
            ));
        }
        _ => unreachable!(),
    }
}

#[test]
#[allow(clippy::float_cmp)]
fn test_visit() {
    let mut ctx = Context::new();
    let ast = {
        let gc = GCLock::new(&mut ctx);
        NodeRc::from_node(
            &gc,
            builder::BlockStatement::build_template(
                &gc,
                template::BlockStatement {
                    metadata: Default::default(),
                    body: NodeList::from_iter(
                        &gc,
                        [
                            builder::ExpressionStatement::build_template(
                                &gc,
                                template::ExpressionStatement {
                                    metadata: Default::default(),
                                    expression: builder::NumericLiteral::build_template(
                                        &gc,
                                        template::NumericLiteral {
                                            metadata: Default::default(),
                                            value: 1.0,
                                        },
                                    ),
                                    directive: None,
                                },
                            ),
                            builder::ExpressionStatement::build_template(
                                &gc,
                                template::ExpressionStatement {
                                    metadata: Default::default(),
                                    expression: builder::NumericLiteral::build_template(
                                        &gc,
                                        template::NumericLiteral {
                                            metadata: Default::default(),
                                            value: 2.0,
                                        },
                                    ),
                                    directive: None,
                                },
                            ),
                        ],
                    ),
                },
            ),
        )
    };

    // Accumulates the numbers found in the AST.
    struct NumberFinder {
        acc: Vec<f64>,
    }

    impl<'gc> Visitor<'gc> for NumberFinder {
        /// Visit the Node `node` with the given `parent`.
        fn call(&mut self, ctx: &'gc GCLock, node: &'gc Node<'gc>, path: Option<Path<'gc>>) {
            if let Node::NumericLiteral(NumericLiteral { value, .. }) = node {
                assert!(matches!(
                    path.unwrap().parent,
                    Node::ExpressionStatement(ExpressionStatement { .. })
                ));
                self.acc.push(*value);
            }
            node.visit_children(ctx, self);
        }
    }

    let mut visitor = NumberFinder { acc: vec![] };
    let gc = GCLock::new(&mut ctx);
    ast.node(&gc).visit(&gc, &mut visitor, None);
    assert_eq!(visitor.acc, [1.0, 2.0]);
}

#[test]
fn test_visit_mut() {
    let mut ctx = Context::new();

    let (x, y) = (1.0, 2.0);
    let ast = {
        let gc = GCLock::new(&mut ctx);
        NodeRc::from_node(
            &gc,
            builder::ExpressionStatement::build_template(
                &gc,
                template::ExpressionStatement {
                    metadata: Default::default(),
                    directive: None,
                    expression: builder::BinaryExpression::build_template(
                        &gc,
                        template::BinaryExpression {
                            metadata: Default::default(),
                            operator: BinaryExpressionOperator::Plus,
                            left: builder::NumericLiteral::build_template(
                                &gc,
                                template::NumericLiteral {
                                    metadata: Default::default(),
                                    value: x,
                                },
                            ),
                            right: builder::UnaryExpression::build_template(
                                &gc,
                                template::UnaryExpression {
                                    metadata: Default::default(),
                                    prefix: true,
                                    operator: UnaryExpressionOperator::Minus,
                                    argument: builder::NumericLiteral::build_template(
                                        &gc,
                                        template::NumericLiteral {
                                            metadata: Default::default(),
                                            value: y,
                                        },
                                    ),
                                },
                            ),
                        },
                    ),
                },
            ),
        )
    };

    struct Pass {}

    impl<'gc> VisitorMut<'gc> for Pass {
        fn call(
            &mut self,
            ctx: &'gc GCLock,
            node: &'gc Node<'gc>,
            _path: Option<Path<'gc>>,
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
                return node.replace_with_new(
                    builder::Builder::BinaryExpression(builder),
                    ctx,
                    self,
                );
            }
            node.visit_children_mut(ctx, self)
        }
    }
    let mut pass = Pass {};

    let transformed = {
        let gc = GCLock::new(&mut ctx);
        NodeRc::from_node(&gc, ast.node(&gc).visit_mut(&gc, &mut pass, None).unwrap())
    };

    {
        let gc = GCLock::new(&mut ctx);
        match transformed.node(&gc) {
            Node::ExpressionStatement(ExpressionStatement {
                expression:
                    Node::BinaryExpression(BinaryExpression {
                        operator: BinaryExpressionOperator::Minus,
                        left:
                            Node::NumericLiteral(NumericLiteral {
                                value: val_left, ..
                            }),
                        right:
                            Node::NumericLiteral(NumericLiteral {
                                value: val_right, ..
                            }),
                        ..
                    }),
                ..
            }) => {
                assert_eq!(
                    (*val_left, *val_right),
                    (x, y),
                    "Transformation failed: {:#?}",
                    transformed.node(&gc),
                );
            }
            _ => panic!("Transformation failed: {:#?}", transformed.node(&gc)),
        };
    }

    {
        ctx.gc();
    }
}

#[test]
fn test_replace_var_decls() {
    let mut ctx = Context::new();
    let ast = hparser::parse_with_flags(Default::default(), "var x, y;", &mut ctx).unwrap();

    {
        let gc = GCLock::new(&mut ctx);
        match ast.node(&gc) {
            Node::Program(Program { body, .. }) => {
                assert_eq!(body.len(), 1, "Program is {:#?}", ast.node(&gc));
            }
            _ => panic!("Parse failed: {:#?}", ast.node(&gc)),
        };
    }

    struct Pass {}
    impl<'gc> VisitorMut<'gc> for Pass {
        fn call(
            &mut self,
            lock: &'gc GCLock,
            node: &'gc Node<'gc>,
            path: Option<Path<'gc>>,
        ) -> TransformResult<&'gc Node<'gc>> {
            match node {
                Node::VariableDeclaration(VariableDeclaration {
                    metadata: _,
                    kind,
                    declarations,
                }) if declarations.len() > 1 => {
                    assert_eq!(path.unwrap().field, NodeField::body);
                    let mut result: Vec<builder::Builder> = Vec::new();
                    for decl in *declarations {
                        result.push(builder::Builder::VariableDeclaration(
                            builder::VariableDeclaration::from_template(
                                template::VariableDeclaration {
                                    metadata: (*decl.range()).into(),
                                    kind: *kind,
                                    declarations: NodeList::from_iter(lock, [decl]),
                                },
                            ),
                        ));
                    }
                    node.replace_with_multiple(result, lock, self)
                }
                _ => node.visit_children_mut(lock, self),
            }
        }
    }
    let mut pass = Pass {};

    let transformed = {
        let gc = GCLock::new(&mut ctx);
        NodeRc::from_node(&gc, ast.node(&gc).visit_mut(&gc, &mut pass, None).unwrap())
    };

    {
        let gc = GCLock::new(&mut ctx);
        match transformed.node(&gc) {
            Node::Program(Program { body, .. }) => {
                assert_eq!(body.len(), 2, "Program is {:#?}", transformed.node(&gc));
                let x_decl = node_cast!(Node::VariableDeclaration, body.head().unwrap())
                    .declarations
                    .head()
                    .unwrap();
                assert_eq!(
                    gc.ctx().str(
                        node_cast!(
                            Node::Identifier,
                            node_cast!(Node::VariableDeclarator, x_decl).id
                        )
                        .name
                    ),
                    "x"
                );
                assert_eq!(x_decl.range().start.line, 1);
                assert_eq!(x_decl.range().start.col, 5);
                assert_eq!(
                    gc.ctx().str(
                        node_cast!(
                            Node::Identifier,
                            node_cast!(
                                Node::VariableDeclarator,
                                node_cast!(Node::VariableDeclaration, body.iter().nth(1).unwrap())
                                    .declarations
                                    .head()
                                    .unwrap()
                            )
                            .id
                        )
                        .name
                    ),
                    "y"
                );
            }
            _ => panic!("Transformation failed: {:#?}", transformed.node(&gc)),
        };
    }
}

#[test]
fn test_many_nodes() {
    let mut ctx = Context::new();
    let mut cached = None;
    let mut val = 0f64;
    for _ in 0..10 {
        {
            let gc = GCLock::new(&mut ctx);
            for i in 0..10_000 {
                cached = Some(NodeRc::from_node(
                    &gc,
                    builder::NumericLiteral::build_template(
                        &gc,
                        template::NumericLiteral {
                            metadata: Default::default(),
                            value: i as f64,
                        },
                    ),
                ));
                val = i as f64;
            }
        }
        ctx.gc();
    }

    let gc = GCLock::new(&mut ctx);
    match cached.unwrap().node(&gc) {
        Node::NumericLiteral(NumericLiteral { value, .. }) => {
            assert!(
                (*value - val).abs() < f64::EPSILON,
                "Incorrect cached value: {:#?}",
                *value
            );
        }
        n => {
            panic!("Incorrect cached value: {:#?}", n);
        }
    };
}

#[test]
fn test_store_node() {
    let mut ctx = Context::new();

    struct Foo<'gc> {
        n: Option<&'gc Node<'gc>>,
    }

    impl<'gc> Foo<'gc> {
        fn set_n(&mut self, node: &'gc Node<'gc>) {
            self.n = Some(node);
        }
    }

    impl<'gc> Visitor<'gc> for Foo<'gc> {
        fn call(&mut self, gc: &'gc GCLock, node: &'gc Node<'gc>, _path: Option<Path<'gc>>) {
            self.set_n(node);
            node.visit_children(gc, self)
        }
    }

    {
        let gc = GCLock::new(&mut ctx);
        let mut pass = Foo { n: None };
        let ast = NodeRc::from_node(
            &gc,
            builder::NumericLiteral::build_template(
                &gc,
                template::NumericLiteral {
                    metadata: Default::default(),
                    value: 1.0f64,
                },
            ),
        );
        ast.node(&gc).visit(&gc, &mut pass, None);
        assert!(pass.n.is_some());
    }
}
