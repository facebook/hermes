/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xes6-class %s | %FileCheck --match-full-lines %s
// REQUIRES: es6_class

class Parent {
  static get clsName() {
    this.getterCallCount = (this.getterCallCount ?? 0) + 1;
    return this._name;
  }
  static set clsName(v) {
    this.setterCallCount = (this.setterCallCount ?? 0) + 1;
    this._name = v;
  }
}

print(Parent.clsName);
//CHECK: undefined

print(Parent.getterCallCount);
//CHECK: 1

Parent.clsName = "Parent";

print(Parent.clsName);
//CHECK: Parent

print(Parent.setterCallCount);
//CHECK: 1

print(Parent.getterCallCount);
//CHECK: 2

print(Object.keys(Parent).sort());
//CHECK: _name,getterCallCount,setterCallCount
