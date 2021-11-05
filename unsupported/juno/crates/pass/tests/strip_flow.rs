/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

extern crate support;

use juno::{
    ast::{self, dump_json, NodeRc},
    gen_js,
    hparser::{self, ParserDialect},
};
use pass::PassManager;
use support::NullTerminatedBuf;

#[test]
fn run_test() {
    let empty = "";

    assert_strip(
        r#"import { Something, type SomeType, typeof SomeOtherThing } from 'some-module';"#,
        r#"import { Something } from 'some-module';"#,
    );

    assert_strip(
        r#"import type { SomeOtherType } from 'some-module';"#,
        empty,
    );
    // assert_strip(
    //     r#"
    //         async function test(x: Type, y /*.*/ ? /*.*/ , z /*.*/ ? /*.*/ : /*.*/ number = 123): string {
    //             return await (x: any);
    //         }"#,
    //     r#""#,
    // );
}

fn parse(input: &str) -> (ast::Context, NodeRc) {
    let mut ctx = ast::Context::new();

    let file_id = ctx
        .sm_mut()
        .add_source("input", NullTerminatedBuf::from_str_copy(input));
    let buf = ctx.sm().source_buffer_rc(file_id);
    let parsed = hparser::ParsedJS::parse(
        hparser::ParserFlags {
            dialect: ParserDialect::Flow,
            ..Default::default()
        },
        &buf,
    );

    assert!(!parsed.has_errors(), "{:?}", parsed.first_error());

    let ast = {
        let gc = ast::GCLock::new(&mut ctx);
        NodeRc::from_node(&gc, parsed.to_ast(&gc, file_id).unwrap())
    };
    drop(parsed);
    (ctx, ast)
}

fn assert_strip(input: &str, expected: &str) {
    let (mut ctx_input, ast_input) = parse(input);

    let pm = PassManager::strip_flow();
    let ast_transformed = pm.run(&mut ctx_input, ast_input);

    let mut transformed_json = vec![];
    dump_json(
        &mut transformed_json,
        &mut ctx_input,
        &ast_transformed,
        ast::Pretty::Yes,
    )
    .unwrap();

    let (mut ctx_expected, ast_expected) = parse(expected);
    let mut expected_json = vec![];
    dump_json(
        &mut expected_json,
        &mut ctx_expected,
        &ast_expected,
        ast::Pretty::Yes,
    )
    .unwrap();

    let mut transformed_js = vec![];
    gen_js::generate(
        &mut transformed_js,
        &mut ctx_input,
        &ast_transformed,
        gen_js::Pretty::Yes,
    )
    .unwrap();

    assert_eq!(
        std::str::from_utf8(&transformed_json).unwrap(),
        std::str::from_utf8(&expected_json).unwrap(),
        "AST mismatch: Expected Source:\n{}\nGenerated Source:\n{}",
        std::str::from_utf8(&transformed_js).unwrap(),
        expected
    );
}
