/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use juno::ast::*;
use juno::gen_js;
use juno::hparser;

fn do_gen(node: &Node, pretty: gen_js::Pretty) -> String {
    use juno::gen_js::*;
    let mut out: Vec<u8> = vec![];
    generate(&mut out, node, pretty).unwrap();
    String::from_utf8(out).expect("Invalid UTF-8 output in test")
}

fn node(kind: NodeKind) -> NodePtr {
    let range = SourceRange {
        file: 0,
        start: SourceLoc { line: 0, col: 0 },
        end: SourceLoc { line: 0, col: 0 },
    };

    NodePtr::new(Node { range, kind })
}

fn test_roundtrip(src1: &str) {
    use juno::ast::*;

    for pretty in &[gen_js::Pretty::Yes, gen_js::Pretty::No] {
        let ast1 = hparser::parse(src1).unwrap();
        let mut dump: Vec<u8> = vec![];
        dump_json(&mut dump, &ast1, juno::ast::Pretty::Yes).unwrap();
        let ast1_json = String::from_utf8(dump).expect("Invalid UTF-8 output in test");

        let src2 = do_gen(&ast1, *pretty);
        let ast2 = hparser::parse(&src2).unwrap_or_else(|_| {
            panic!(
                "Invalid JS generated: Pretty={:?}\nOriginal Source:\n{}\nGenerated Source:\n{}",
                pretty, &src1, &src2,
            )
        });
        let mut dump: Vec<u8> = vec![];
        dump_json(&mut dump, &ast2, juno::ast::Pretty::Yes).unwrap();
        let ast2_json = String::from_utf8(dump).expect("Invalid UTF-8 output in test");

        assert_eq!(
            ast1_json, ast2_json,
            "AST mismatch: Pretty={:?}\nOriginal Source:\n{}\nGenerated Source:\n{}",
            pretty, &src1, &src2
        );
    }
}

#[test]
fn test_literals() {
    use NodeKind::*;
    assert_eq!(
        do_gen(&node(NullLiteral), gen_js::Pretty::Yes).trim(),
        "null"
    );
    assert_eq!(
        do_gen(
            &node(StringLiteral {
                value: juno::ast::StringLiteral {
                    str: vec!['A' as u16, 0x1234u16, '\t' as u16],
                }
            },),
            gen_js::Pretty::Yes
        )
        .trim(),
        r#""A\u1234\t""#,
    );
    assert_eq!(
        do_gen(&node(NumericLiteral { value: 1.0 },), gen_js::Pretty::Yes).trim(),
        "1"
    );

    test_roundtrip("1");
    test_roundtrip("\"abc\"");
    test_roundtrip(r#" "\ud800" "#);
    test_roundtrip(r#" "\ud83d\udcd5" "#);
    test_roundtrip("true");
    test_roundtrip("false");
    test_roundtrip("/abc/");
    test_roundtrip("/abc/gi");
    test_roundtrip("/abc/gi");
}

#[test]
fn test_binop() {
    use NodeKind::*;
    assert_eq!(
        do_gen(
            &node(BinaryExpression {
                left: node(NullLiteral),
                operator: BinaryExpressionOperator::Plus,
                right: node(NullLiteral),
            }),
            gen_js::Pretty::Yes
        )
        .trim(),
        "null + null"
    );

    test_roundtrip("1 + 1");
    test_roundtrip("1 * 2 + (3 + 4)");
    test_roundtrip("1 ** 2 ** 3 ** 4");
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
}

#[test]
fn test_types() {
    use NodeKind::*;
    assert_eq!(
        do_gen(
            &node(UnionTypeAnnotation {
                types: vec![
                    node(NumberTypeAnnotation),
                    node(IntersectionTypeAnnotation {
                        types: vec![node(BooleanTypeAnnotation), node(StringTypeAnnotation),]
                    })
                ]
            }),
            gen_js::Pretty::Yes
        )
        .trim(),
        "number | boolean & string"
    );
}
