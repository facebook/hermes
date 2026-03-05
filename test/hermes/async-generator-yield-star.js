/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xasync-generators %s | %FileCheck --match-full-lines %s
// RUN: %hermes -Xasync-generators -O0 %s | %FileCheck --match-full-lines %s
// RUN: %hermes -Xasync-generators -Xes6-block-scoping %s | %FileCheck --match-full-lines %s
// RUN: %hermes -Xasync-generators -Xes6-block-scoping -O0 %s | %FileCheck --match-full-lines %s
// RUN: %hermes -Xasync-generators -lazy %s | %FileCheck --match-full-lines %s

// Test for `yield*` inside `async function*`.
//
// The AST transformer (AsyncGenerator.cpp) converts `async function*` into a
// regular `function*` wrapped in `_wrapAsyncGenerator`, and transforms
// `await expr` into `yield _awaitAsyncGenerator(expr)`. However, `yield*`
// (delegate yield) requires special handling because the inner iterable is an
// async iterable with `Symbol.asyncIterator`, while IRGen's `genYieldStarExpr`
// compiles `yield*` using `emitGetIteratorSlow`, which looks up
// `Symbol.iterator`. Without transformation, this produces:
//   TypeError: undefined is not a function
// because the async iterable has no `Symbol.iterator` method.
//
// The fix transforms `yield*` at the AST level:
//   yield* expr  →  yield* _asyncGeneratorDelegate(expr)
//
// `_asyncGeneratorDelegate` (defined in 04-AsyncIterator.js) creates a sync
// iterator wrapper around the async iterable. The wrapper's `next()`,
// `throw()`, and `return()` methods produce `OverloadYield(promise, 1)` objects
// (kind=1 = delegate). The existing `AsyncGenerator.resume()` driver already
// handles kind=1: it resolves the promise, and if the inner iterator is not
// done, feeds the resolved `{value, done}` result back into the generator via
// `generatorFunction[nextKey](resolvedValue)`. This causes the wrapper's next
// call to return the resolved result directly (the `waiting` flag short-circuit),
// which `genYieldStarExpr` then processes as a normal sync iteration result.
//
// The net effect: `yield*` on async iterables works transparently, with all
// promise resolution handled by the AsyncGenerator driver, and IRGen's existing
// sync `yield*` compilation is reused without modification.

// Install a rejection hook so failures show errors instead of silent drops.
HermesInternal.enablePromiseRejectionTracker({
  allRejections: true,
  onUnhandled: function(id, error) {
    print("Unhandled rejection:", error instanceof Error ? error.message : error);
  },
});

// Test 1: Basic yield* delegation from async gen to async gen
async function* inner1() {
    yield 1;
    yield 2;
    yield 3;
}
async function* outer1() {
    yield* inner1();
}

// Test 2: yield* with return value from inner generator
async function* inner2() {
    yield 10;
    yield 20;
    return 30;
}
async function* outer2() {
    var result = yield* inner2();
    yield result;
}

// Test 3: Values before and after yield* delegation
async function* outer3() {
    yield "before";
    yield* inner1();
    yield "after";
}

// Test 4: yield* with sync iterable (array) — tests Symbol.iterator fallback
async function* outer4() {
    yield* [100, 200, 300];
}

// Test 5: Error propagation through yield*
async function* innerError() {
    yield 1;
    throw new Error("inner error");
}
async function* outer5() {
    try {
        yield* innerError();
    } catch (e) {
        yield "caught:" + e.message;
    }
}

// Test 6: Nested yield* delegation (outer -> middle -> inner)
async function* middle6() {
    yield* inner1();
}
async function* outer6() {
    yield* middle6();
}

// Test 7: yield* with promise-yielding async generator
async function* innerPromise() {
    yield Promise.resolve(42);
    yield Promise.resolve(43);
}
async function* outer7() {
    yield* innerPromise();
}

(async function runAll() {
    // Test 1: basic delegation
    var r1 = [];
    for await (var x of outer1()) r1.push(x);
    print("test1:", JSON.stringify(r1));
    // CHECK: test1: [1,2,3]

    // Test 2: return value from inner
    var r2 = [];
    for await (var x of outer2()) r2.push(x);
    print("test2:", JSON.stringify(r2));
    // CHECK-NEXT: test2: [10,20,30]

    // Test 3: values before and after
    var r3 = [];
    for await (var x of outer3()) r3.push(x);
    print("test3:", JSON.stringify(r3));
    // CHECK-NEXT: test3: ["before",1,2,3,"after"]

    // Test 4: sync iterable
    var r4 = [];
    for await (var x of outer4()) r4.push(x);
    print("test4:", JSON.stringify(r4));
    // CHECK-NEXT: test4: [100,200,300]

    // Test 5: error propagation
    var r5 = [];
    for await (var x of outer5()) r5.push(x);
    print("test5:", JSON.stringify(r5));
    // CHECK-NEXT: test5: [1,"caught:inner error"]

    // Test 6: nested delegation
    var r6 = [];
    for await (var x of outer6()) r6.push(x);
    print("test6:", JSON.stringify(r6));
    // CHECK-NEXT: test6: [1,2,3]

    // Test 7: promise-yielding
    var r7 = [];
    for await (var x of outer7()) r7.push(x);
    print("test7:", JSON.stringify(r7));
    // CHECK-NEXT: test7: [42,43]
})();
