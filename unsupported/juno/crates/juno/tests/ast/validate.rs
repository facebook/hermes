/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use juno::ast::*;
use juno::hparser;
use juno::hparser::ParserDialect;
use juno::hparser::ParserFlags;

fn validate_src_with_flags(
    flags: hparser::ParserFlags,
    src: &str,
) -> Result<(), TreeValidationError> {
    let mut ctx = Context::new();
    let ast = hparser::parse_with_flags(flags, src, &mut ctx).unwrap();
    validate_tree(&mut ctx, &ast)
}

fn validate_src(src: &str) -> Result<(), TreeValidationError> {
    validate_src_with_flags(Default::default(), src)
}

fn validate_src_jsx(src: &str) -> Result<(), TreeValidationError> {
    validate_src_with_flags(
        ParserFlags {
            enable_jsx: true,
            ..Default::default()
        },
        src,
    )
}

fn validate_src_flow(src: &str) -> Result<(), TreeValidationError> {
    validate_src_with_flags(
        ParserFlags {
            dialect: ParserDialect::Flow,
            ..Default::default()
        },
        src,
    )
}

#[test]
fn test_valid() {
    let mut ctx = Context::new();
    let return_stmt = {
        let gc = GCLock::new(&mut ctx);
        NodeRc::from_node(
            &gc,
            builder::ReturnStatement::build_template(
                &gc,
                template::ReturnStatement {
                    metadata: Default::default(),
                    argument: None,
                },
            ),
        )
    };
    assert!(validate_tree_pure(&mut ctx, &return_stmt).is_ok());

    let return_stmt = {
        let gc = GCLock::new(&mut ctx);
        NodeRc::from_node(
            &gc,
            builder::ReturnStatement::build_template(
                &gc,
                template::ReturnStatement {
                    metadata: Default::default(),
                    argument: Some(builder::NumericLiteral::build_template(
                        &gc,
                        template::NumericLiteral {
                            metadata: Default::default(),
                            value: 1.0,
                        },
                    )),
                },
            ),
        )
    };
    assert!(validate_tree_pure(&mut ctx, &return_stmt).is_ok());

    let return_stmt = {
        let gc = GCLock::new(&mut ctx);
        NodeRc::from_node(
            &gc,
            builder::ReturnStatement::build_template(
                &gc,
                template::ReturnStatement {
                    metadata: Default::default(),
                    argument: Some(builder::ReturnStatement::build_template(
                        &gc,
                        template::ReturnStatement {
                            metadata: Default::default(),
                            argument: None,
                        },
                    )),
                },
            ),
        )
    };
    assert!(validate_tree_pure(&mut ctx, &return_stmt).is_err());
}

#[test]
fn test_error() {
    let mut ctx = Context::new();

    let ast: NodeRc = {
        let gc = GCLock::new(&mut ctx);
        NodeRc::from_node(
            &gc,
            builder::BlockStatement::build_template(
                &gc,
                template::BlockStatement {
                    metadata: Default::default(),
                    body: NodeList::from_iter(
                        &gc,
                        [builder::ReturnStatement::build_template(
                            &gc,
                            template::ReturnStatement {
                                metadata: Default::default(),
                                argument: Some(builder::ReturnStatement::build_template(
                                    &gc,
                                    template::ReturnStatement {
                                        metadata: Default::default(),
                                        argument: None,
                                    },
                                )),
                            },
                        )],
                    ),
                },
            ),
        )
    };

    let bad_ret: NodeRc = {
        let gc = GCLock::new(&mut ctx);
        match ast.node(&gc) {
            Node::BlockStatement(BlockStatement { body, .. }) => {
                NodeRc::from_node(&gc, body.head().unwrap())
            }
            _ => {
                unreachable!("bad match");
            }
        }
    };
    match validate_tree_pure(&mut ctx, &ast) {
        Ok(()) => {
            panic!("Must be error");
        }
        Err(e) => {
            assert_eq!(e.len(), 1);
            assert_eq!(e[0].node, bad_ret);
        }
    }
}

#[test]
fn test_literals() {
    validate_src("({});").unwrap();
    validate_src("({x: y});").unwrap();
    validate_src("({x: y, ...z});").unwrap();
    validate_src("[]").unwrap();
    validate_src("[x,y,,z]").unwrap();
    validate_src("[x, y, ...z]").unwrap();
}

#[test]
fn test_patterns() {
    validate_src("[x] = y;").unwrap();
    validate_src("[x.y , , ...z] = y;").unwrap();
    validate_src("({x} = y)").unwrap();
    validate_src("({x: y} = y)").unwrap();
}

#[test]
fn test_calls() {
    validate_src("foo()").unwrap();
    validate_src("foo(1,2)").unwrap();
    validate_src("foo(1,2,...bar)").unwrap();
}

#[test]
fn test_jsx() {
    validate_src_jsx("<foo />").unwrap();
    validate_src_jsx("<foo bar={1} />").unwrap();
    validate_src_jsx("<foo bar='abc' />").unwrap();
    validate_src_jsx("<foo><bar></bar></foo>").unwrap();
    validate_src_jsx("<foo><bar>abc</bar></foo>").unwrap();
}

#[test]
fn test_flow() {
    validate_src_flow("function foo(a: number): number { return a; }").unwrap();
    validate_src_flow("function foo(a: number): number %checks { return a; }").unwrap();
    validate_src_flow("function foo(a: number): number %checks(a[1]) { return a; }").unwrap();
    validate_src_flow(
        "class A<T> extends B<T> {
            foo: number;
            +bar: string;
    }",
    )
    .unwrap();
}
