/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use super::node;
use juno::ast::{validate_tree, Node, NodeKind};

#[test]
fn test_valid() {
    use NodeKind::*;
    assert!(validate_tree(&node(ReturnStatement { argument: None })).is_ok());

    assert!(
        validate_tree(&node(ReturnStatement {
            argument: Some(node(NumericLiteral { value: 1.0 }))
        }))
        .is_ok()
    );

    assert!(
        validate_tree(&node(ReturnStatement {
            argument: Some(node(ReturnStatement { argument: None })),
        }))
        .is_err()
    );
}

#[test]
fn test_error() {
    use NodeKind::*;
    let ast = node(BlockStatement {
        body: vec![node(ReturnStatement {
            argument: Some(node(ReturnStatement { argument: None })),
        })],
    });
    let bad_ret: &Node = match &ast.kind {
        BlockStatement { body } => &body[0],
        _ => {
            unreachable!("bad match");
        }
    };
    match validate_tree(&ast) {
        Ok(()) => {
            panic!("Must be error");
        }
        Err(e) => {
            assert_eq!(e.len(), 1);
            assert_eq!(e[0].node as *const Node, bad_ret as *const Node);
        }
    }
}
