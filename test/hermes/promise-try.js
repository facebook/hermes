/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xmicrotask-queue %s | %FileCheck --match-full-lines %s
// RUN: %hermesc -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

print('try' in Promise);
// CHECK: true

// Promise.try per spec §27.2.4.9.
// Sync return value flows to fulfillment.
Promise.try(function() { return 'try-val'; })
  .then(function(v) { print('try-val:', v); });
// CHECK-NEXT: try-val: try-val

// Sync throw flows to rejection.
Promise.try(function() { throw 'try-err'; })
  .then(undefined, function(e) { print('try-err:', e); });
// CHECK-NEXT: try-err: try-err

// Extra arguments are passed positionally to the callback.
Promise.try(function(a, b) { return a + b; }, 1, 2)
  .then(function(v) { print('try-args:', v); });
// CHECK-NEXT: try-args: 3

// Returning a promise causes the result to follow it.
Promise.try(function() { return Promise.resolve('inner'); })
  .then(function(v) { print('try-thenable:', v); });
// CHECK-NEXT: try-thenable: inner
