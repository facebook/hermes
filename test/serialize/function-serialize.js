/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -serialize-after-init-file=%t -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -deserialize-file=%t -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: serializer
"use strict";

print('Function');
// CHECK-LABEL: Function
print(typeof Function.__proto__, Function.__proto__ === Function.prototype);
// CHECK-NEXT: function true
print(Function.__proto__.length);
// CHECK-NEXT: 0
print(Function.length);
// CHECK-NEXT: 1
print(typeof print.constructor);
// CHECK-NEXT: function
print(print.length);
// CHECK-NEXT: 1
print(typeof print.__proto__);
// CHECK-NEXT: function
print(Function.__proto__());
// CHECK-NEXT: undefined
try {
  new Function.__proto__();
} catch (e) {
  print(e);
}

// CHECK-NEXT: TypeError: This function cannot be used as a constructor.
function foo() {}
print("foo.length/configurable:", Object.getOwnPropertyDescriptor(foo, "length").configurable);
//CHECK: foo.length/configurable: true
print("foo.__proto__.length/configurable:", Object.getOwnPropertyDescriptor(foo.__proto__, "length").configurable);
//CHECK: foo.__proto__.length/configurable: true
print("Function.length/configurable:", Object.getOwnPropertyDescriptor(Function, "length").configurable);
//CHECK: Function.length/configurable: true

print(isNaN.name);
//CHECK: isNaN
print(Function.name);
//CHECK: Function
