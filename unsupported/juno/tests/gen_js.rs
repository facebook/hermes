/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use juno::ast::*;
use juno::gen_js::*;

fn do_gen(node: &Node) -> String {
    let mut out: Vec<u8> = vec![];
    generate(&mut out, node, Pretty::Yes).unwrap();
    String::from_utf8(out).expect("Invalid UTF-8 output in test")
}

fn node(kind: NodeKind) -> Box<Node> {
    let range = SourceRange {
        file: 0,
        start: SourceLoc { line: 0, col: 0 },
        end: SourceLoc { line: 0, col: 0 },
    };

    Box::new(Node { range, kind })
}

#[test]
fn test_literals() {
    use NodeKind::*;
    assert_eq!(do_gen(&node(NullLiteral)).trim(), "null");
}
