/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s
"use strict";

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
