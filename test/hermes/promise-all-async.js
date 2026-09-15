/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xmicrotask-queue -test262 %s | %FileCheck --match-full-lines %s

// Per ECMA-262 (ES2025) §27.2.4.1.2 PerformPromiseAll, each element's
// resolved promise is passed to `.then(onFulfilled, onRejected)` (step
// 8.i: Invoke(nextPromise, "then", ...)). PromiseReactionJobs
// (§27.2.2.1) queued by `.then` run asynchronously per §27.2.1.4.
//
// Therefore `Promise.all([p]).then(h)` where `p` is already fulfilled
// must require at least two microtask hops before `h` runs: one for
// the internal per-element `.then` and one for the outer `.then(h)`.
// The default polyfill keeps a fast path that synchronously extracts
// the value from an already-fulfilled core Promise (collapsing this
// into a single hop) for performance. The spec-compliant path is
// gated on `--test262`, which this test passes.

var allHandlerRan = false;

Promise.all([Promise.resolve()]).then(function () {
  allHandlerRan = true;
});

// Synchronous observation: the handler has not run.
print('sync:', allHandlerRan);
// CHECK: sync: false

// After one microtask, the internal per-element `.then` has run and
// resolved the outer result, but the outer handler has not yet been
// drained. A sync fast path would have run the handler already.
Promise.resolve().then(function () {
  print('after 1 microtask:', allHandlerRan);
});
// CHECK-NEXT: after 1 microtask: false

// After two microtasks, the outer handler has run.
Promise.resolve().then(function () {
  Promise.resolve().then(function () {
    print('after 2 microtasks:', allHandlerRan);
  });
});
// CHECK-NEXT: after 2 microtasks: true
