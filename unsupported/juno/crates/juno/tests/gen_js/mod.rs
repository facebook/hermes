/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use juno::ast::*;
use juno::gen_js;
use juno::hparser;
use juno::sourcemap::merge_sourcemaps;

fn do_gen<'ast>(ctx: &mut Context<'ast>, node: &NodeRc, pretty: gen_js::Pretty) -> String {
    use juno::gen_js::*;
    let mut out: Vec<u8> = vec![];
    generate(
        &mut out,
        ctx,
        node,
        gen_js::Opt {
            pretty,
            ..Default::default()
        },
    )
    .unwrap();
    String::from_utf8(out).expect("Invalid UTF-8 output in test")
}

fn test_roundtrip_with_flags(flags: hparser::ParserFlags, src1: &str) {
    use juno::ast::*;

    let mut ctx = Context::new();

    for pretty in &[gen_js::Pretty::Yes, gen_js::Pretty::No] {
        let ast1 = hparser::parse_with_flags(flags, src1, &mut ctx).unwrap();
        let mut dump: Vec<u8> = vec![];
        dump_json(&mut dump, &mut ctx, &ast1, juno::ast::Pretty::Yes).unwrap();
        let ast1_json = String::from_utf8(dump).expect("Invalid UTF-8 output in test");

        let src2 = do_gen(&mut ctx, &ast1, *pretty);
        let ast2 = hparser::parse_with_flags(flags, &src2, &mut ctx).unwrap_or_else(|_| {
            panic!(
                "Invalid JS generated: Pretty={:?}\nOriginal Source:\n{}\nGenerated Source:\n{}",
                pretty, &src1, &src2,
            )
        });
        let mut dump: Vec<u8> = vec![];
        dump_json(&mut dump, &mut ctx, &ast2, juno::ast::Pretty::Yes).unwrap();
        let ast2_json = String::from_utf8(dump).expect("Invalid UTF-8 output in test");

        assert_eq!(
            ast1_json, ast2_json,
            "AST mismatch: Pretty={:?}\nOriginal Source:\n{}\nGenerated Source:\n{}",
            pretty, &src1, &src2
        );
    }
}

fn test_roundtrip(src1: &str) {
    test_roundtrip_with_flags(Default::default(), src1)
}

fn test_roundtrip_flow(src1: &str) {
    test_roundtrip_with_flags(
        hparser::ParserFlags {
            strict_mode: false,
            enable_jsx: false,
            dialect: hparser::ParserDialect::Flow,
            store_doc_block: false,
        },
        src1,
    );
}

fn test_roundtrip_jsx(src1: &str) {
    test_roundtrip_with_flags(
        hparser::ParserFlags {
            strict_mode: false,
            enable_jsx: true,
            dialect: hparser::ParserDialect::JavaScript,
            store_doc_block: false,
        },
        src1,
    )
}

#[test]
fn test_literals() {
    let mut ctx = Context::new();
    let string = {
        let gc = ctx.lock();
        NodeRc::from_node(
            &gc,
            builder::StringLiteral::build_template(
                &gc,
                template::StringLiteral {
                    metadata: Default::default(),
                    value: gc.atom_u16(vec!['A' as u16, 0x1234u16, '\t' as u16]),
                },
            ),
        )
    };
    assert_eq!(
        do_gen(&mut ctx, &string, gen_js::Pretty::Yes).trim(),
        r#"'A\u1234\t'"#,
    );

    test_roundtrip("1");
    test_roundtrip("1n");
    test_roundtrip("11298379123162378326187361n");
    test_roundtrip("\"abc\"");
    test_roundtrip(r#" "\ud800" "#);
    test_roundtrip(r#" "\ud83d\udcd5" "#);
    test_roundtrip(r#" "\u060b" "#);
    test_roundtrip("true");
    test_roundtrip("false");
    test_roundtrip("null");
    test_roundtrip("undefined");

    test_roundtrip("/abc/");
    test_roundtrip("/abc/gi");
    test_roundtrip("/abc/gi");
    test_roundtrip(r#"/ab\/cd/gi"#);
    test_roundtrip("/😹/");

    test_roundtrip(r#" `abc` "#);
    test_roundtrip(r#" `abc\ndef` "#);
    test_roundtrip(
        r#" `abc
        def` "#,
    );
    test_roundtrip(r#" `abc \ud800 def` "#);
    test_roundtrip(r#" `abc \ud800 def` "#);
    test_roundtrip(r#" `\ud83d\udcd5` "#);
    test_roundtrip(r#" `escape backtick: \` should work` "#);
    test_roundtrip(r#" `😹` "#);
}

#[test]
fn test_identifier() {
    test_roundtrip("foo");
    test_roundtrip("class C { #foo() {} }");
}

#[test]
fn test_binop() {
    test_roundtrip("null + null");
    test_roundtrip("1 + 1");
    test_roundtrip("1 * 2 + (3 + 4)");
    test_roundtrip("1 ** 2 ** 3 ** 4");
    test_roundtrip("1 in 2 + (2 - 4) / 3");
    test_roundtrip("1 instanceof 2 + (2 - 4) / 3");
}

#[test]
fn test_conditional() {
    test_roundtrip("a ? b : c");
    test_roundtrip("a ? b : c ? d : e");
    test_roundtrip("(a ? b : c) ? d : e");
    test_roundtrip("a ? b : (c ? d : e)");
    test_roundtrip("a?.3:.4");
}

#[test]
fn test_vars() {
    test_roundtrip("var x=3;");
    test_roundtrip("var x=3, y=4;");
}

#[test]
fn test_functions() {
    test_roundtrip("function foo() {}");
    test_roundtrip("function foo(x, y) {}");
    test_roundtrip("function foo(x, y=3) {}");
    test_roundtrip("function foo([x, y], {z}) {}");
    test_roundtrip("function foo([x, y] = [1,2], {z:q}, {w = 1}) {}");
    test_roundtrip("function foo() { return this; }");
    test_roundtrip("function *foo() {}");
    test_roundtrip("function *foo() { yield 1; }");
    test_roundtrip("function *foo() { yield* f(); }");
    test_roundtrip("async function foo() {}");
    test_roundtrip("async function foo() { await f(); }");
    test_roundtrip("async function *foo() {}");
    test_roundtrip("async function *foo() { await f(); yield 1; }");
    test_roundtrip("x => 3");
    test_roundtrip("(x) => 3");
    test_roundtrip("(x,y) => 3");
    test_roundtrip("x => {3}");
    test_roundtrip("x => ({y: 10})");
    test_roundtrip("x => ({y: 10}[z])");
    test_roundtrip("async x => {3}");
    test_roundtrip("async (x,y) => {3}");
    test_roundtrip("(x => 1) + (y => 1)");
    test_roundtrip("x = y => 1");
    test_roundtrip("x = (y => 1)");
    test_roundtrip("x = (({a, b}) => 1)");
    test_roundtrip_flow("var x = (): (number=>string) => 1");
    test_roundtrip(
        "function foo() {
        return (y => 1);
    }",
    );
    test_roundtrip(
        "function* foo() {
        yield y => 1;
    }",
    );
}

#[test]
fn test_calls() {
    test_roundtrip("f();");
    test_roundtrip("f(1);");
    test_roundtrip("f(1, 2);");
    test_roundtrip("f(1, (2,3), 4);");
    test_roundtrip("(f?.(1, 2))(3);");
    test_roundtrip("f?.(1, 2)?.(3)(5);");
    test_roundtrip("f(...x)");
    test_roundtrip("new f();");
    test_roundtrip("new f(1);");
    test_roundtrip("new f(...x)");
    test_roundtrip("new(a.b);");
    test_roundtrip("new(a.b());");
    test_roundtrip("new(a.b())();");
    test_roundtrip("new(a.b())(c);");
    test_roundtrip("new(a?.b())(c);");
    test_roundtrip("new(1 + 2);");
    test_roundtrip("new(fn(foo)[bar])()");
    test_roundtrip("new(fn(foo)[bar])(c)");
    test_roundtrip("new(fn(foo).bar)()");
    test_roundtrip("new(fn(foo).bar)(c)");
    test_roundtrip("import('foo')");
}

#[test]
fn test_statements() {
    test_roundtrip("while (1) {}");
    test_roundtrip("while (1) { fn(); }");
    test_roundtrip("while (1) fn();");
    test_roundtrip("while (1) fn()");
    test_roundtrip("for (;;) { fn(); }");
    test_roundtrip("for (;;) fn();");
    test_roundtrip("for (x;;) { fn(); }");
    test_roundtrip("for (;x;) { fn(); }");
    test_roundtrip("for (;;x) { fn(); }");
    test_roundtrip("for (var x=1;x<10;++x) { fn(); }");
    test_roundtrip("for (x in y) { fn(); }");
    test_roundtrip("for (var x of y) { fn(); }");
    test_roundtrip(
        "async () => {
            for await (x of y) { fn(); }
        }",
    );
    test_roundtrip("do {fn();} while (1)");
    test_roundtrip("do fn(); while (1)");
    test_roundtrip("do x, y, z; while (1)");
    test_roundtrip("do if (x) y; while (1)");
    test_roundtrip("debugger");
    test_roundtrip("{fn(); fn();}");
    test_roundtrip("for (;;) { break; }");
    test_roundtrip("for (;;) { continue; }");
    test_roundtrip("function f() { return; }");
    test_roundtrip("function f() { return 3; }");
    test_roundtrip(
        "switch(x) {
            case 1:
                break;
            case 2:
            case 3:
                break;
            default:
                break;
        }",
    );
    test_roundtrip("a: var x = 3;");
    test_roundtrip(
        "try {
            fn();
        } catch {
            fn();
        }",
    );
    test_roundtrip(
        "try {
            fn();
        } catch (e) {
            fn();
        }",
    );
    test_roundtrip(
        "try {
            fn();
        } catch (e) {
            fn();
        } finally {
            fn();
        }",
    );
    test_roundtrip("if (x) {fn();}");
    test_roundtrip("if (x) {fn();} else {fn();}");
    test_roundtrip("if (x) fn(); else fn();");
    test_roundtrip(
        "if (x)
          try { } catch (e) { }
        else
          fn();",
    );
}

#[test]
fn test_logical() {
    test_roundtrip("a && b || c");
    test_roundtrip("a || b && c");
    test_roundtrip("(a || b) && c");
    test_roundtrip("(a || b) ?? c");
    test_roundtrip("(a ?? b) || c");
}

#[test]
fn test_sequences() {
    test_roundtrip("var x = (1, 2, 3);");
    test_roundtrip("foo((1, 2, 3), 4);");
}

#[test]
fn test_objects() {
    test_roundtrip("({ })");
    test_roundtrip(
        "({
            a: 1,
            [x]: 1,
            fn() {},
            b,
            ...from,
        })",
    );
    test_roundtrip_flow(
        "({
            foo<T>() {},
        })",
    );
}

#[test]
fn test_arrays() {
    test_roundtrip("([])");
    test_roundtrip("var x = [, 1, , 3]");
    test_roundtrip("var x = [1, 2, 3, ...from]");
    test_roundtrip("var x = [1, 2, 3, ...from, 4, 5, 6]");
}

#[test]
fn test_assignment() {
    test_roundtrip("x = 1");
    test_roundtrip("x = y = 1");
    test_roundtrip("x += 1");
    test_roundtrip("x -= 1");
    test_roundtrip("x *= 1");
    test_roundtrip("x /= 1");
    test_roundtrip("x **= 1");
    test_roundtrip("x |= 1");
    test_roundtrip("x &= 1");
    test_roundtrip("x ||= 1");
    test_roundtrip("x &&= 1");
    test_roundtrip("x ??= 1");
    test_roundtrip("foo()[1] = 1");
    test_roundtrip("a = b && c");
    test_roundtrip("(a = b) && c");
    test_roundtrip("a && b = c");
    test_roundtrip("a && (b = c)");
    test_roundtrip("var {x: {y: [{z}]}} = foo;");
    test_roundtrip("({x: {y: [{z}]}} = foo);");
    test_roundtrip("var [x, y] = foo;");
    test_roundtrip("([x, y] = foo);");
}

#[test]
fn test_unary() {
    test_roundtrip("+x");
    test_roundtrip("-x");
    test_roundtrip("!x");
    test_roundtrip("~x");
    test_roundtrip("-(-x)");
    test_roundtrip("-(-5)");
    test_roundtrip("--x");
    test_roundtrip("x--");
    test_roundtrip("++x");
    test_roundtrip("x++");
    test_roundtrip("+!-x");
    test_roundtrip("delete x");
    test_roundtrip("typeof x");
}

#[test]
fn test_update() {
    test_roundtrip("x++");
    test_roundtrip("x--");
    test_roundtrip("++x");
    test_roundtrip("--x");
    test_roundtrip("--(-x)");
    test_roundtrip("+x++");
}

#[test]
fn test_members() {
    test_roundtrip("a.b");
    test_roundtrip("a.b.c");
    test_roundtrip("a?.b");
    test_roundtrip("a?.[b]");
    test_roundtrip("(a?.b).c");
    test_roundtrip("a?.b().c");
    test_roundtrip("(a?.b()).c");
    test_roundtrip("a?.().b");
    test_roundtrip("a?.().b");
    test_roundtrip("a?.b?.c?.()");
    test_roundtrip("(a?.b?.c?.()).d");
    test_roundtrip("(a?.b?.c?.())?.d");
    test_roundtrip("(a?.b?.c?.())(d)");
    test_roundtrip("(a?.b?.c?.())?.(d)");
    test_roundtrip("class C { constructor() { new.target; } }");
    test_roundtrip("50..toString()");
    test_roundtrip("1.5.toString()");
    test_roundtrip("1e100.toString()");
    test_roundtrip("-1e100.toString()");
    test_roundtrip("0x10293.toString()");
}

#[test]
fn test_classes() {
    test_roundtrip("class C {}");
    test_roundtrip("class C extends D {}");
    test_roundtrip(
        "class C extends D {
            prop1;
            #prop2;
            constructor() {}
            a() {}
            #b() {}
            c(x, y) {}
            static d() {}
        }",
    );
    test_roundtrip_flow(
        "class C<T> extends D<T> {
            prop1: ?number = null;
            +prop2: number;
            -prop3;
            declare prop4;
            #prop5;
            #prop5: ?number = null;
            declare +prop6;
            static +prop7;
            static +[prop8];
            declare static +prop9;
            foo<T>() {}
        }",
    );
    test_roundtrip(
        "var cls = (class C extends D {
            prop1;
            #prop2;
            constructor() {}
            a() {}
            #b() {}
            c(x, y) {}
            static d() {}
            get e() {}
            set e(v) {}
            ;
        })",
    );
}

#[test]
fn test_import() {
    test_roundtrip("import x from 'foo'");
    test_roundtrip("import x, {y} from 'foo'");
    test_roundtrip("import * as Foo from 'foo'");
    test_roundtrip("import x, {y as z, a as b} from 'foo'");
    test_roundtrip("import {a, b, c} from 'foo'");
    test_roundtrip("import 'foo';");
    test_roundtrip("import 'foo' assert {kind: 'json'};");
    test_roundtrip(
        "
        import 'foo';
        import 'bar';
        ",
    );
}

#[test]
fn test_export() {
    test_roundtrip("export var x = 3;");
    test_roundtrip("export function foo() {}");
    test_roundtrip("export default function foo() {}");
    test_roundtrip("export {x as y};");
    test_roundtrip("export * from 'foo';");
    test_roundtrip_flow("export type Foo = number;");
    test_roundtrip_flow("export type { x as y } from 'foo';");
}

#[test]
fn test_types() {
    test_roundtrip_flow("number | boolean & string");
    test_roundtrip_flow("type A = number");
    test_roundtrip_flow("type A = ?number");
    test_roundtrip_flow("type A = ?(number | string)");
    test_roundtrip_flow("type A = string");
    test_roundtrip_flow("type A = \"foo\"");
    test_roundtrip_flow("type A = 'foo'");
    test_roundtrip_flow("type A = 3");
    test_roundtrip_flow("type A = 3n");
    test_roundtrip_flow("type A = boolean");
    test_roundtrip_flow("type A = true | false");
    test_roundtrip_flow("type A = true & false");
    test_roundtrip_flow("type A = (X | Y) & Z");
    test_roundtrip_flow("type A = X | Y & Z");
    test_roundtrip_flow("type A = X<Y, Z>");
    test_roundtrip_flow("type A = X<Y>");
    test_roundtrip_flow("type A<X: Y, Z> = T");
    test_roundtrip_flow("type A = symbol");
    test_roundtrip_flow("type A = mixed");
    test_roundtrip_flow("type A = any");
    test_roundtrip_flow("type A = void");
    test_roundtrip_flow("type A = null");
    test_roundtrip_flow("type A = number => number");
    test_roundtrip_flow("type A = X.Y");
    test_roundtrip_flow("type A = X.Y<Z>");
    test_roundtrip_flow("type A = typeof X");
    test_roundtrip_flow("type A = [number, string]");
    test_roundtrip_flow("type A = []");
    test_roundtrip_flow("type A = number[]");
    test_roundtrip_flow("type A = number[string]");
    test_roundtrip_flow("type A = number?.[string]");
    test_roundtrip_flow("type A = [number, string][]");
    test_roundtrip_flow("type A = (foo: number) => number");
    test_roundtrip_flow("type A = (foo?: number) => number");
    test_roundtrip_flow("type A = (foo?: ?number) => number");
    test_roundtrip_flow("type A = (number, string) => number");
    test_roundtrip_flow("type A = (?number) => number");
    test_roundtrip_flow("type A = ?(number, string) => number");
    test_roundtrip_flow("type A = (this: number, number, string) => number");
    test_roundtrip_flow("interface A { }");
    test_roundtrip_flow("interface A extends B { }");
    test_roundtrip_flow("interface A extends B, C, D { }");
    test_roundtrip_flow("type A = { x: number }");
    test_roundtrip_flow("type A = {| x: number |}");
    test_roundtrip_flow(
        "
        type A = {
            a?: number,
            b: ?string,
            +[c]: string,
            (d?: number): number;
            [[e]]: number,
            [[f]]?: number,
            [[g]](a: T): number,
            ...h,
            static (i?: number): number;
            +proto: number,
            ...
        };
        ",
    );
}

#[test]
fn test_declare() {
    test_roundtrip_flow("declare function foo(): number;");
    test_roundtrip_flow("declare var x : number;");
    test_roundtrip_flow("declare export var x: number;");
    test_roundtrip_flow("declare opaque type x;");
    test_roundtrip_flow("declare export opaque type x: y;");
    test_roundtrip_flow("declare type x = number;");
    test_roundtrip_flow("declare interface Foo {}");
    test_roundtrip_flow("declare class A extends B {}");
    test_roundtrip_flow("declare class A extends B mixins C, D implements E {}");
    test_roundtrip_flow("declare export class A extends B {}");
    test_roundtrip_flow("declare module A {}");
    test_roundtrip_flow("declare module.exports: number;");
    test_roundtrip_flow("declare export function foo(): number;");
}

#[test]
fn test_enum() {
    test_roundtrip_flow("enum Foo {}");
    test_roundtrip_flow("enum Foo of string {A = 'A', B = 'B'}");
    test_roundtrip_flow("enum Foo of string {A, B, C}");
    test_roundtrip_flow("enum Foo of string {A = 'A', B = 'B', ...}");
    test_roundtrip_flow("enum Foo of number {A = 1}");
    test_roundtrip_flow("enum Foo of boolean {A = true}");
}

#[test]
fn test_typecast() {
    test_roundtrip_flow("async function foo() { return (x: any); }");
    test_roundtrip_flow("var x = (y: number | number => string)");
}

#[test]
fn test_predicate() {
    test_roundtrip_flow("function foo(): %checks {}");
    test_roundtrip_flow("function foo(): number %checks {}");
    test_roundtrip_flow("function foo(): number %checks(bar) {}");
    test_roundtrip_flow("((x): %checks => 3)");
    test_roundtrip_flow("((x): number %checks => 3)");
    test_roundtrip_flow("((x): number %checks(bar) => 3)");
}

#[test]
fn test_this_param() {
    test_roundtrip_flow("function foo(this: number): number {}");
    test_roundtrip_flow("function foo(this: number, x: number): number {}");
    test_roundtrip_flow("declare function foo(this: number): number;");
    test_roundtrip_flow("declare function foo(this: number, x: number): number;");
}

#[test]
fn test_jsx() {
    test_roundtrip_jsx("<foo />");
    test_roundtrip_jsx("<foo></foo>");
    test_roundtrip_jsx("<foo>abc</foo>");
    test_roundtrip_jsx(
        r#"
        <asdf desc="foo
            bar"
            prop2='foo """ bar'>
            body
        </asdf>
        "#,
    );
    test_roundtrip_jsx("<></>");
    test_roundtrip_jsx(
        "
        <foo>
            <bar x={1} y='3' {...z} />
            abcdef and an emoji: 😹
            &gt; &x1f639;
            end of test text
            <baz.bar />
            <hello:goodbye />
        </foo>
        ",
    );
}

#[test]
fn test_sourcemap() {
    use juno::gen_js::*;
    let mut ctx = Context::new();
    let ast1: NodeRc = hparser::parse(&mut ctx, "function foo() { return 1 }").unwrap();
    let mut out: Vec<u8> = vec![];
    let sourcemap = generate(&mut out, &mut ctx, &ast1, gen_js::Opt::new()).unwrap();
    let string = String::from_utf8(out).expect("Invalid UTF-8 output in test");
    assert_eq!(
        string,
        "function foo() {
  return 1;
}\n"
    );
    assert_eq!(sourcemap.get_token_count(), 4);

    assert_eq!(
        sourcemap.get_token(0).unwrap().get_raw_token(),
        sourcemap::RawToken {
            dst_line: 0,
            dst_col: 0,
            src_line: 0,
            src_col: 0,
            src_id: 0,
            name_id: !0,
        }
    );

    assert_eq!(
        sourcemap.get_token(1).unwrap().get_raw_token(),
        sourcemap::RawToken {
            dst_line: 0,
            dst_col: 9,
            src_line: 0,
            src_col: 9,
            src_id: 0,
            name_id: !0,
        }
    );

    assert_eq!(
        sourcemap.get_token(2).unwrap().get_raw_token(),
        sourcemap::RawToken {
            dst_line: 1,
            dst_col: 2,
            src_line: 0,
            src_col: 17,
            src_id: 0,
            name_id: !0,
        }
    );

    assert_eq!(
        sourcemap.get_token(3).unwrap().get_raw_token(),
        sourcemap::RawToken {
            dst_line: 1,
            dst_col: 9,
            src_line: 0,
            src_col: 24,
            src_id: 0,
            name_id: !0,
        }
    );
}

#[test]
fn test_sourcemap_merged() {
    use juno::gen_js::*;
    let mut context = Context::new();
    let ctx = &mut context;
    // original TS source:
    // type A = number;
    // function foo() { 1; }

    let input_src = r#"function foo() { 1; }"#;
    let input_map = sourcemap::SourceMap::from_slice(
        br#"{
            "version":3,
            "file":"test.js",
            "sourceRoot":"",
            "sources":["test.ts"],
            "names":[],
            "mappings":"AACA,SAAS,GAAG,KAAK,CAAC,CAAC,CAAC,CAAC"
        }"#,
    )
    .unwrap();
    let mut out: Vec<u8> = vec![];
    let node = hparser::parse_with_flags(Default::default(), input_src, ctx).unwrap();
    let output_map = generate(&mut out, ctx, &node, gen_js::Opt::new()).unwrap();
    let output = String::from_utf8(out).expect("Invalid UTF-8 output in test");
    assert_eq!(output, "function foo() {\n  1;\n}\n",);

    let merged = merge_sourcemaps(&input_map, &output_map);

    // Source maps are 0-indexed, so use those for testing purposes.
    let input_token: sourcemap::Token = merged.lookup_token(1, 2).unwrap();
    assert_eq!(input_token.get_source().unwrap(), "test.ts");
    assert_eq!(input_token.get_src(), (1, 17));
}
