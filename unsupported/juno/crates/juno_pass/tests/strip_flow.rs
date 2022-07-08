/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

extern crate juno_support;

use juno::ast;
use juno::ast::dump_json;
use juno::ast::NodeRc;
use juno::gen_js;
use juno::hparser;
use juno::hparser::ParserDialect;
use juno_pass::PassManager;
use juno_support::NullTerminatedBuf;

#[test]
fn imports_exports() {
    assert_strip(
        r#"
            import { Something, type SomeType, typeof SomeOtherThing } from 'some-module';
            import type { SomeOtherType } from 'some-module';

            export type { T };
            export { Something };
            export type ONE = { one: number };
            export interface IThing {
                exported: true;
            }

            export type TestReadOnly = {|
                +readOnly: $ReadOnlyArray<>
            |};
        "#,
        r#"
            import { Something } from 'some-module';
            export {Something as Something}
        "#,
    );
}

#[test]
fn declare() {
    assert_strip(
        r#"
            declare function someFunc(): void;

            declare interface ISomething {
                answer: number;
            }


            declare module 'fs' {
                declare function readThing(path: string): string;
            }

            declare type Location = {
                lat: number,
                lon: number
            };

            declare var SOME_CONST: string;

            declare class Baz {
                method(): mixed;
            }

            type T = string;

            declare export opaque type B;
            declare export var x;
            declare export function x(): void;
            declare export default T;
            ;
    "#,
        r#";"#,
    )
}

#[test]
fn classes() {
    assert_strip(
        r#"
            interface Foo {
                prop: any;
                method(): mixed;
            }

            interface SillyFoo extends Foo {
               silly: string;
            }
            ;"#,
        r#";"#,
    );

    assert_strip(
        r#"
        class Bar extends Other implements Foo, ISomething {
            answer: number = 42;
            +covariant: number = 42;
            prop: any;
            +propCo: number;

            method(): mixed {
                return;
            }
        }
        "#,
        r#"
        class Bar extends Other {
            answer = 42;
            covariant = 42;
            method() {
                return;
            }
        }
        "#,
    );
    assert_strip(
        r#"
        var SomeClass2 = class Baz implements Foo {
            prop: any;

            method(): mixed {
                return;
            }
        };
        "#,
        r#"
        var SomeClass2 = class Baz {
            method() {
                return;
            }
        };
        "#,
    );
    assert_strip(
        r#"
        class Wrapper<T> {
            get(): T {
                return this.value;
            }

            map<M>(): Wrapper<M> {
                // do something
            }
        }
        "#,
        r#"
        class Wrapper {
            get() {
                return this.value;
            }

            map() {
                // do something
            }
        }
        "#,
    );
    assert_strip(
        r#"
        class StringWrapper extends Wrapper<string> {
            // ...
        }
        "#,
        r#"
        class StringWrapper extends Wrapper {
            // ...
        }
        "#,
    );

    assert_strip(
        r#"
            class MyClass1 {
               declare prop: string;
            }
        "#,
        r#"class MyClass1 {}"#,
    );
    assert_strip(
        r#"
            class TestClassWithDefault<+T: TestReadOnly = TestReadOnly> {
                constructor() {}
            }
        "#,
        r#"
            class TestClassWithDefault {
                constructor() {}
            }
        "#,
    );
}

#[test]
fn functions() {
    assert_strip(
        r#"
            function f2<T, S = T>() {}
            async function test(x: Type, y? , z? : number = 123): string {
                return await (x: any);
            }
            function testit(arg: mixed): boolean %checks {
                return !!arg;
            }
            const f = async <T>(): T => {};
            "#,
        r#"
            function f2() {}
            async function test(x, y, z = 123) {
                return await x;
            }
            function testit(arg) {
                return !!arg;
            }
            const f = async () => {};
        "#,
    );
}

#[test]
fn expressions() {
    assert_strip(
        r#"
        var union: T | U;
        var intersection: T & U;
        var someObj = {
            objMethod(): void {
              // do nothing.
            }
        }
        doSomething<number>(3);
        doSomething <T, U>(3);
        new Event<number>();
    "#,
        r#"
        var union;
        var intersection;
        var someObj = {
            objMethod() {
            // do nothing.
            }
        }
        doSomething(3);
        doSomething(3);
        new Event();
    "#,
    )
}

#[test]
fn opaque() {
    assert_strip(
        r#"
            opaque type A = number;
            opaque type B: string = string;
            declare opaque type A;
            declare opaque type B: string;
            export opaque type A = number;
            ;
        "#,
        r#";"#,
    );
}

#[test]
fn comments() {
    assert_strip(
        r#"
            var X /*: {
                version: string,
            } */ = { version: "42" };

            function method(param /*: string */) /*: number */ {
                // ...
            }

            // Comment type includes are emptied out
            class MyClass2 {
                /*:: prop: string; */
            }
        "#,
        r#"
            var X /*: {
                version: string,
            } */ = { version: "42" };

            function method(param /*: string */) /*: number */ {
                // ...
            }

            // Comment type includes are emptied out
            class MyClass2 {
                /*:: prop: string; */
            }
        "#,
    )
}

#[test]
fn enums() {
    assert_strip(
        r#"enum E {A, B}"#,
        r#"const E = require("flow-enums-runtime").Mirrored(["A", "B"]);"#,
    );
    assert_strip(
        r#"enum E {A = "X", B = "Y"}"#,
        r#"const E = require("flow-enums-runtime")({A: "X", B: "Y"});"#,
    );
    assert_strip(
        r#"enum E {A = 1, B = 2}"#,
        r#"const E = require("flow-enums-runtime")({A: 1, B: 2});"#,
    );
    assert_strip(
        r#"enum E {A = true, B = false}"#,
        r#"const E = require("flow-enums-runtime")({A: true, B: false});"#,
    );
    assert_strip(
        r#"enum E of symbol {A, B}"#,
        r#"const E = require("flow-enums-runtime")({A: Symbol("A"), B: Symbol("B")});"#,
    );

    assert_strip(
        r#"enum E {}"#,
        r#"const E = require("flow-enums-runtime")({});"#,
    );
    assert_strip(
        r#"enum E of symbol {}"#,
        r#"const E = require("flow-enums-runtime")({});"#,
    );

    assert_strip(
        r#"export enum E {A, B}"#,
        r#"export const E = require("flow-enums-runtime").Mirrored(["A", "B"]);"#,
    );
    assert_strip(
        r#"export default enum E {A, B}"#,
        r#"const E = require("flow-enums-runtime").Mirrored(["A", "B"]); export default E;"#,
    );
}

#[test]
fn this_params() {
    assert_strip(
        r#"
            declare function y (this : string) : void
            function z (this : string) {}
            function u (this : string, ...a) {}

            function v (this : string
                , ...a) {}

            function w (this
            : string

                ,
                ...a) {}

            const f = function(this: string) {}
            const g = function(this: string, ...a) {}
            const h = function(this
            : string,
            ...a) {}
        "#,
        r#"
            function z() {}
            function u(...a) {}
            function v(...a) {}

            function w(...a) {}

            const f = function () {};
            const g = function (...a) {};
            const h = function (...a) {};
        "#,
    );
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
        gen_js::Opt::new(),
    )
    .unwrap();

    let mut expected_js = vec![];
    gen_js::generate(
        &mut expected_js,
        &mut ctx_expected,
        &ast_expected,
        gen_js::Opt::new(),
    )
    .unwrap();

    assert_eq!(
        std::str::from_utf8(&expected_json).unwrap(),
        std::str::from_utf8(&transformed_json).unwrap(),
        "AST mismatch:\n Expected Source:\n{}\nGenerated Source:\n{}",
        std::str::from_utf8(&expected_js).unwrap(),
        std::str::from_utf8(&transformed_js).unwrap()
    );
}
