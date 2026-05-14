/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s
// RUN: %hermes -Xmicrotask-queue %s | %FileCheck --match-full-lines %s

print('promise-withResolvers');
// CHECK-LABEL: promise-withResolvers

// withResolvers exists and is a function
print(typeof Promise.withResolvers);
// CHECK-NEXT: function

// Basic resolve
var r1 = Promise.withResolvers();
print(r1.promise instanceof Promise);
// CHECK-NEXT: true
print(typeof r1.resolve);
// CHECK-NEXT: function
print(typeof r1.reject);
// CHECK-NEXT: function
r1.resolve(42);
r1.promise.then(function(v) { print('resolved:', v); });

// Basic reject
var r2 = Promise.withResolvers();
r2.reject('oops');
r2.promise.then(null, function(e) { print('rejected:', e); });

// Subclass support (uses 'this' as constructor)
class MyPromise extends Promise {}
var r3 = Promise.withResolvers.call(MyPromise);
print(r3.promise instanceof MyPromise);
// CHECK-NEXT: true
r3.resolve('sub');
r3.promise.then(function(v) { print('subclass:', v); });

// Microtask callbacks run after all synchronous code
// CHECK-NEXT: resolved: 42
// CHECK-NEXT: rejected: oops
// CHECK-NEXT: subclass: sub
