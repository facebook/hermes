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

// Test for `for await...of` inside `async function*`.
//
// The AST transformer (AsyncGenerator.cpp) converts `async function*` into a
// regular `function*` wrapped in `_wrapAsyncGenerator`, and transforms
// `await expr` into `yield _awaitAsyncGenerator(expr)`. The
// `_awaitAsyncGenerator` call wraps the value in an OverloadYield object, which
// tells the AsyncGenerator runtime driver that this yield is an internal await
// (not a user-visible yield) and the resolved value should be sent back into
// the generator rather than forwarded to the consumer.
//
// However, `for await...of` is compiled directly by IRGen
// (genAsyncForOfStatement), which calls genYieldOrAwaitExpr() to await the
// iterator's .next() result. In an async generator's inner generator, this
// produces a raw SaveAndYieldInst without the OverloadYield wrapper. The
// AsyncGenerator driver sees the raw yield as a user-visible yield and sends
// the resolved value to the outer consumer. On the next .next() call, the
// generator resumes with `undefined`, and `undefined.done` throws a TypeError.
//
// The failure mode is subtle: the TypeError is a rejected promise on the async
// generator's .next() result. Without explicit error handling, Hermes silently
// drops unhandled rejections, so the program simply produces no output instead
// of crashing with a visible error.
//
// The fix wraps the await points in genAsyncForOfStatement and
// _emitIteratorCloseImpl with emitAwaitAsyncGenerator() when inside an async
// generator's inner generator (but not inside a plain async function's inner
// generator, where spawnAsync handles raw yields correctly).

// Install a rejection hook so that if the fix regresses, the test prints the
// actual error instead of silently producing no output.
HermesInternal.enablePromiseRejectionTracker({
  allRejections: true,
  onUnhandled: function(id, error) {
    print("Unhandled rejection:", error instanceof Error ? error.message : error);
  },
});

// Test 1: for await...of with async iterator inside async function*
async function* asyncGenForAwait() {
    var results = [];
    async function* inner() {
        yield 1;
        yield 2;
        yield 3;
    }
    for await (var x of inner()) {
        results.push(x);
    }
    yield results;
}

// Test 2: for await...of with sync iterable inside async function*
async function* asyncGenForAwaitSync() {
    var results = [];
    for await (var x of [10, 20, 30]) {
        results.push(x);
    }
    yield results;
}

// Test 4: for await...of with early break inside async function*
async function* asyncGenBreak() {
    async function* source() {
        yield 1;
        yield 2;
        yield 3;
        yield 4;
        yield 5;
    }
    var results = [];
    for await (var x of source()) {
        results.push(x);
        if (x === 3) break;
    }
    yield results;
}

// Test 5: for await...of with exception inside async function*
async function* asyncGenThrow() {
    async function* source() {
        yield 1;
        yield 2;
        throw new Error("oops");
    }
    var results = [];
    try {
        for await (var x of source()) {
            results.push(x);
        }
    } catch (e) {
        results.push("caught:" + e.message);
    }
    yield results;
}

// Test 6: Nested for await + yield inside async function*
async function* asyncGenNested() {
    async function* source() {
        yield Promise.resolve(10);
        yield Promise.resolve(20);
        yield Promise.resolve(30);
    }
    for await (var x of source()) {
        yield x * 2;
    }
}

// Chain all tests sequentially to ensure deterministic output order.
(async function runAll() {
    // Test 1
    for await (var r of asyncGenForAwait()) {
        print("test1:", JSON.stringify(r));
    }
    // CHECK: test1: [1,2,3]

    // Test 2
    for await (var r of asyncGenForAwaitSync()) {
        print("test2:", JSON.stringify(r));
    }
    // CHECK-NEXT: test2: [10,20,30]

    // Test 3: for await...of in plain async function still works
    var results = [];
    async function* source3() {
        yield 100;
        yield 200;
        yield 300;
    }
    for await (var x of source3()) {
        results.push(x);
    }
    print("test3:", JSON.stringify(results));
    // CHECK-NEXT: test3: [100,200,300]

    // Test 4
    for await (var r of asyncGenBreak()) {
        print("test4:", JSON.stringify(r));
    }
    // CHECK-NEXT: test4: [1,2,3]

    // Test 5
    for await (var r of asyncGenThrow()) {
        print("test5:", JSON.stringify(r));
    }
    // CHECK-NEXT: test5: [1,2,"caught:oops"]

    // Test 6
    var results6 = [];
    for await (var r of asyncGenNested()) {
        results6.push(r);
    }
    print("test6:", JSON.stringify(results6));
    // CHECK-NEXT: test6: [20,40,60]
})();
