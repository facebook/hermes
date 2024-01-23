/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xes6-class %s | %FileCheck --match-full-lines %s
// REQUIRES: es6_class

class Parent {
  nonOverridenMethod() {
    return "From Parent";
  }

  overridenMethod() {
    return "From Parent";
  }
}

class Child extends Parent {
  overridenMethod() {
    return "From Child";
  }
}

const parentInstance = new Parent();

print(parentInstance.nonOverridenMethod());
//CHECK: From Parent

print(parentInstance.overridenMethod());
//CHECK: From Parent

const childInstance = new Child();

print(childInstance.nonOverridenMethod());
//CHECK: From Parent

print(childInstance.overridenMethod());
//CHECK: From Child

print(Object.keys(parentInstance).length);
// CHECK: 0

print(Object.keys(childInstance).length);
// CHECK: 0

print(Parent.prototype.nonOverridenMethod.name);
// CHECK: nonOverridenMethod
