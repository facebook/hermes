/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use juno::ast::*;
use juno::gen_js;
use juno::hparser;

fn do_gen(ctx: &Context, node: NodePtr, pretty: gen_js::Pretty) -> String {
    use juno::gen_js::*;
    let mut out: Vec<u8> = vec![];
    generate(&mut out, ctx, node, pretty).unwrap();
    String::from_utf8(out).expect("Invalid UTF-8 output in test")
}

fn test_roundtrip_with_flags(flags: hparser::ParserFlags, src1: &str) {
    use juno::ast::*;

    let mut context = Context::new();
    let ctx = &mut context;

    for pretty in &[gen_js::Pretty::Yes, gen_js::Pretty::No] {
        let ast1 = hparser::parse_with_file_id(flags, src1, ctx, 0).unwrap();
        let mut dump: Vec<u8> = vec![];
        dump_json(&mut dump, ctx, ast1, juno::ast::Pretty::Yes).unwrap();
        let ast1_json = String::from_utf8(dump).expect("Invalid UTF-8 output in test");

        let src2 = do_gen(ctx, ast1, *pretty);
        let ast2 = hparser::parse_with_file_id(flags, &src2, ctx, 0).unwrap_or_else(|_| {
            panic!(
                "Invalid JS generated: Pretty={:?}\nOriginal Source:\n{}\nGenerated Source:\n{}",
                pretty, &src1, &src2,
            )
        });
        let mut dump: Vec<u8> = vec![];
        dump_json(&mut dump, ctx, ast2, juno::ast::Pretty::Yes).unwrap();
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
        },
        src1,
    );
}

#[test]
fn test_literals() {
    use NodeKind::*;
    let mut ctx = Context::new();
    let null = make_node!(ctx, NullLiteral);
    assert_eq!(do_gen(&ctx, null, gen_js::Pretty::Yes).trim(), "null");
    let string = make_node!(
        ctx,
        StringLiteral {
            value: juno::ast::NodeString {
                str: vec!['A' as u16, 0x1234u16, '\t' as u16],
            }
        },
    );
    assert_eq!(
        do_gen(&ctx, string, gen_js::Pretty::Yes).trim(),
        r#""A\u1234\t""#,
    );
    let number = make_node!(ctx, NumericLiteral { value: 1.0 });
    assert_eq!(do_gen(&ctx, number, gen_js::Pretty::Yes).trim(), "1");

    test_roundtrip("1");
    test_roundtrip("\"abc\"");
    test_roundtrip(r#" "\ud800" "#);
    test_roundtrip(r#" "\ud83d\udcd5" "#);
    test_roundtrip("true");
    test_roundtrip("false");

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
    use NodeKind::*;
    let mut ctx = Context::new();
    let left = make_node!(ctx, NullLiteral);
    let right = make_node!(ctx, NullLiteral);
    let binary = make_node!(
        ctx,
        BinaryExpression {
            left,
            operator: BinaryExpressionOperator::Plus,
            right,
        },
    );
    assert_eq!(
        do_gen(&ctx, binary, gen_js::Pretty::Yes).trim(),
        "null + null"
    );

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
    test_roundtrip("function foo([x, y] = [1,2], {z:q}) {}");
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
    test_roundtrip("async x => {3}");
    test_roundtrip("async (x,y) => {3}");
}

#[test]
fn test_calls() {
    test_roundtrip("f();");
    test_roundtrip("f(1);");
    test_roundtrip("f(1, 2);");
    test_roundtrip("(f?.(1, 2))(3);");
    test_roundtrip("f?.(1, 2)?.(3)(5);");
    test_roundtrip("new f();");
    test_roundtrip("new f(1);");
    test_roundtrip("import('foo')");
}

#[test]
fn test_statements() {
    test_roundtrip("while (1) {}");
    test_roundtrip("while (1) { fn(); }");
    test_roundtrip("while (1) fn();");
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
            ...from,
        })",
    );
}

#[test]
fn test_arrays() {
    test_roundtrip("([])");
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
}

#[test]
fn test_unary() {
    test_roundtrip("+x");
    test_roundtrip("-x");
    test_roundtrip("!x");
    test_roundtrip("~x");
    test_roundtrip("-(-x)");
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
}

#[test]
fn test_types() {
    use NodeKind::*;
    let mut ctx = Context::new();
    let union_ty = make_node!(
        ctx,
        UnionTypeAnnotation {
            types: vec![
                make_node!(ctx, NumberTypeAnnotation),
                make_node!(
                    ctx,
                    IntersectionTypeAnnotation {
                        types: vec![
                            make_node!(ctx, BooleanTypeAnnotation),
                            make_node!(ctx, StringTypeAnnotation)
                        ],
                    },
                )
            ],
        },
    );
    assert_eq!(
        do_gen(&ctx, union_ty, gen_js::Pretty::Yes).trim(),
        "number | boolean & string"
    );

    test_roundtrip_flow("type A = number");
    test_roundtrip_flow("type A = ?number");
    test_roundtrip_flow("type A = string");
    test_roundtrip_flow("type A = 'foo'");
    test_roundtrip_flow("type A = 3");
    test_roundtrip_flow("type A = boolean");
    test_roundtrip_flow("type A = true | false");
    test_roundtrip_flow("type A = symbol");
    test_roundtrip_flow("type A = mixed");
    test_roundtrip_flow("type A = any");
    test_roundtrip_flow("type A = void");
    test_roundtrip_flow("type A = number => number");
    test_roundtrip_flow("type A = (number, string) => number");
}
