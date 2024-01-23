/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xes6-class %s | %FileCheck --match-full-lines %s
// REQUIRES: es6_class

class Parent {
  static nonOverridenMethod() {
    return "From Parent";
  }

  static overridenMethod() {
    return "From Parent";
  }
}

class Child extends Parent {
  static overridenMethod() {
    return "From Child";
  }
}

print(Parent.nonOverridenMethod());
//CHECK: From Parent

print(Parent.overridenMethod());
//CHECK: From Parent

print(Child.nonOverridenMethod());
//CHECK: From Parent

print(Child.overridenMethod());
//CHECK: From Child
