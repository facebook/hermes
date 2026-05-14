/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s
// RUN: %hermes -Xmicrotask-queue %s | %FileCheck --match-full-lines %s

print('promise-try');
// CHECK-LABEL: promise-try

// try exists and is a function
print(typeof Promise.try);
// CHECK-NEXT: function
// CHECK-NEXT: true

// Basic resolve
var r1 = Promise.try(function () { return 30; });
print(r1 instanceof Promise);
r1.then(function(v) { print('resolved:', v); });

// Basic reject
var r2 = Promise.try(function () { throw 'oops...'; });
r2.then(null, function(e) { print('rejected:', e); });

// Arguments passing down
var r3 = Promise.try(function (a,b,c) { return a + b + c; }, 1, 2, 3);
r3.then(function(v) { print('arguments:', v); });

// No callback
var r4 = Promise.try();
r4.then(null, function(e) { print('no_callback:', e.message); });

// Async callback
var r5 = Promise.try(function () { return Promise.resolve(31) })
r5.then(function(v) { print('async resolved:', v); });

// Microtask callbacks run after all synchronous code
// CHECK-NEXT: resolved: 30
// CHECK-NEXT: rejected: oops...
// CHECK-NEXT: arguments: 6
// CHECK-NEXT: no_callback: Cannot read property 'apply' of undefined
// CHECK-NEXT: async resolved: 31
