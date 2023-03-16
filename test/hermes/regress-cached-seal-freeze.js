/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck %s

var x = []
x[0] = 10
Object.defineProperty(x, "length", {writable: false});
Object.preventExtensions(x);
print(Object.isSealed(x));
print(Object.isFrozen(x));
//CHECK: false
//CHECK-NEXT: false

Object.defineProperty(x, "0", {enumerable: false});
print(Object.isSealed(x));
print(Object.isFrozen(x));
//CHECK-NEXT: false
//CHECK-NEXT: false

var y = {};
var z = {};
// Both should have the same class.
Object.defineProperty(y, "a", {configurable: false, writable: false, value: 0});
Object.defineProperty(z, "a", {configurable: false, writable: false, value: 0});
Object.preventExtensions(y);

// Cache the value on their shared class.
print(Object.isSealed(y));
print(Object.isFrozen(y));
//CHECK-NEXT: true
//CHECK-NEXT: true

// Add a property to z that ensures it is not sealed or frozen.
Object.defineProperty(z, "b", {configurable: true, writable: true, value: 0});
Object.preventExtensions(z);
print(Object.isSealed(z));
print(Object.isFrozen(z));
//CHECK-NEXT: false
//CHECK-NEXT: false
