/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use juno::ast::*;

#[test]
fn test_valid() {
    let mut ctx = Context::new();
    let return_stmt = {
        let gc = GCContext::new(&mut ctx);
        NodePtr::from_node(
            &gc,
            ReturnStatementBuilder::build_template(
                &gc,
                ReturnStatementTemplate {
                    metadata: Default::default(),
                    argument: None,
                },
            ),
        )
    };
    assert!(validate_tree(&mut ctx, &return_stmt).is_ok());

    let return_stmt = {
        let gc = GCContext::new(&mut ctx);
        NodePtr::from_node(
            &gc,
            ReturnStatementBuilder::build_template(
                &gc,
                ReturnStatementTemplate {
                    metadata: Default::default(),
                    argument: Some(NumericLiteralBuilder::build_template(
                        &gc,
                        NumericLiteralTemplate {
                            metadata: Default::default(),
                            value: 1.0,
                        },
                    )),
                },
            ),
        )
    };
    assert!(validate_tree(&mut ctx, &return_stmt).is_ok());

    let return_stmt = {
        let gc = GCContext::new(&mut ctx);
        NodePtr::from_node(
            &gc,
            ReturnStatementBuilder::build_template(
                &gc,
                ReturnStatementTemplate {
                    metadata: Default::default(),
                    argument: Some(ReturnStatementBuilder::build_template(
                        &gc,
                        ReturnStatementTemplate {
                            metadata: Default::default(),
                            argument: None,
                        },
                    )),
                },
            ),
        )
    };
    assert!(validate_tree(&mut ctx, &return_stmt).is_err());
}

#[test]
fn test_error() {
    let mut ctx = Context::new();

    let ast: NodePtr = {
        let gc = GCContext::new(&mut ctx);
        NodePtr::from_node(
            &gc,
            BlockStatementBuilder::build_template(
                &gc,
                BlockStatementTemplate {
                    metadata: Default::default(),
                    body: vec![ReturnStatementBuilder::build_template(
                        &gc,
                        ReturnStatementTemplate {
                            metadata: Default::default(),
                            argument: Some(ReturnStatementBuilder::build_template(
                                &gc,
                                ReturnStatementTemplate {
                                    metadata: Default::default(),
                                    argument: None,
                                },
                            )),
                        },
                    )],
                },
            ),
        )
    };

    let bad_ret: NodePtr = {
        let gc = GCContext::new(&mut ctx);
        match ast.node(&gc) {
            Node::BlockStatement(BlockStatement { body, .. }) => NodePtr::from_node(&gc, body[0]),
            _ => {
                unreachable!("bad match");
            }
        }
    };
    match validate_tree(&mut ctx, &ast) {
        Ok(()) => {
            panic!("Must be error");
        }
        Err(e) => {
            assert_eq!(e.len(), 1);
            assert_eq!(e[0].node, bad_ret);
        }
    }
}
