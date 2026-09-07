/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xmicrotask-queue %s | %FileCheck --match-full-lines %s
// RUN: %hermesc -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

// Per await-dictionary proposal §27.2.4.1 Promise.allKeyed
// https://tc39.es/proposal-await-dictionary/#sec-promise.allkeyed

print('promise-all-settled-keyed-resolution-order.js');
// CHECK-LABEL: promise-all-settled-keyed-resolution-order.js
print('allSettledKeyed' in Promise);
// CHECK: true

// Check order of promise resolution
Promise.allSettledKeyed({ a: 1, b: 2, c: 3, d: 4 }).then(() => print('4 keys'))
Promise.allSettledKeyed({ a: 1, b: 2 }).then(() => print('2 keys'))
Promise.allSettledKeyed({}).then(() => print('0 keys'))

// CHECK-NEXT: 0 keys
// CHECK-NEXT: 4 keys
// CHECK-NEXT: 2 keys
