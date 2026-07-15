/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xasync-generators %s | %FileCheck --match-full-lines %s
// XFAIL: *

// This test demonstrates a microtask tick ordering difference between
// the polyfill-based async generator and a native implementation.
//
// The polyfill converts async generators to regular generators wrapped
// by an AsyncGenerator driver class. The driver uses
// HermesPromise.resolve(value).then(callback) to process every yielded
// value. This .then() schedules a microtask, adding one extra tick
// compared to a native AsyncGeneratorYield which resolves the consumer's
// promise and suspends without an intermediate promise chain.
//
// As a result, after the async generator completes (done:true), the
// consumer's for-await-of loop exits one tick later than it should.
//
// Native (v8):  ["pre", "tick 1", "tick 2", "got 1", "tick 3", "post", ...]
// Polyfill:     ["pre", "tick 1", "tick 2", "got 1", "tick 3", "tick 4", "post", ...]
//
// The extra tick cannot be eliminated because the polyfill must always go
// through .then() to handle OverloadYield signaling (for await and yield*
// inside async generators). Even for plain yields and completion, the
// driver calls HermesPromise.resolve(value).then(cb) where cb calls
// conclude() which resolves the consumer's promise — adding one microtask
// hop that a native implementation avoids.

var log = [];

async function* gen() {
  yield 1;
}

async function test() {
  log.push("pre");
  for await (var x of gen()) {
    log.push("got " + x);
  }
  log.push("post");
}

Promise.resolve()
  .then(function() { log.push("tick 1"); })
  .then(function() { log.push("tick 2"); })
  .then(function() { log.push("tick 3"); })
  .then(function() { log.push("tick 4"); })
  .then(function() { log.push("tick 5"); })
  .then(function() {
    print(log.join(", "));
  });

test();

// A native engine completes the for-await-of loop at tick 3.
// The polyfill completes it at tick 4 due to the extra .then() hop.
// CHECK: pre, tick 1, tick 2, got 1, tick 3, post, tick 4, tick 5
