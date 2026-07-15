/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xasync-generators %s | %FileCheck --match-full-lines %s
// RUN: %hermes -Xasync-generators -O0 %s | %FileCheck --match-full-lines %s
// RUN: %hermes -Xasync-generators -lazy %s | %FileCheck --match-full-lines %s

// Regression: the named function expression preserves its `.name` after the
// async generator AST transform. The transform wraps the async generator
// body in an outer regular function expression; before the fix, the outer
// wrapper was anonymous (`_id = nullptr`), so `.name` was "" instead of the
// original identifier.
print((async function* MyGen() {}).name);
//CHECK: MyGen

// Regression: the named function expression's binding is accessible from
// within the async generator body, and assignment to it in sloppy mode is
// silently ignored per ES2025 §9.1.1.1.5 step 5.b. Without preserving _id
// on the outer wrapper, `Foo` would not be in scope inside the body and
// the assignment would fall through to the global object.
(async function testNamedFnExprBindingSloppy() {
    var f = async function* Foo() {
        yield typeof Foo;
        Foo = 1;
        yield typeof Foo;
    };
    for await (const v of f()) {
        print(v);
    }
})();
//CHECK-NEXT: function
//CHECK-NEXT: function
