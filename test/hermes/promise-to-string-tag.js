/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s
// RUN: %hermes -Xmicrotask-queue %s | %FileCheck --match-full-lines %s

print('promise-toStringTag');
// CHECK-LABEL: promise-toStringTag

// 1. check toStringTag value
print(Promise.prototype[Symbol.toStringTag]);
// CHECK-NEXT: Promise

// 2. not writable
Promise.prototype[Symbol.toStringTag] = '1'
print(Promise.prototype[Symbol.toStringTag]);
// CHECK-NEXT: Promise

// 3. Should be inherited by subclass
class MyPromise extends Promise {}

print(MyPromise.prototype[Symbol.toStringTag]);
// CHECK-NEXT: Promise
