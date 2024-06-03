/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xes6-class %s | %FileCheck --match-full-lines %s
// REQUIRES: es6_class

class Test {
}


print(Object.getPrototypeOf(Test) === Object.getPrototypeOf(Object));
//CHECK: true

print(Object.getPrototypeOf(Test.prototype) === Object.prototype);
//CHECK: true

const obj = new Test();

print(obj.propertyIsEnumerable('test'));
// CHECK: false