/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xes6-class %s | %FileCheck --match-full-lines %s
// REQUIRES: es6_class

class Parent {
  get name() {
    this.getterCallCount = (this.getterCallCount ?? 0) + 1;
    return this._name;
  }
  set name(v) {
    this.setterCallCount = (this.setterCallCount ?? 0) + 1;
    this._name = v;
  }
}

const parentInstance = new Parent();

print(parentInstance.name);
//CHECK: undefined

print(parentInstance.getterCallCount);
//CHECK: 1

parentInstance.name = "Parent";

print(parentInstance.name);
//CHECK: Parent

print(parentInstance.setterCallCount);
//CHECK: 1

print(parentInstance.getterCallCount);
//CHECK: 2

print(Object.keys(parentInstance).sort());
//CHECK: _name,getterCallCount,setterCallCount
