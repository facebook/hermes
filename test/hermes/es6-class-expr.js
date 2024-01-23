/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xes6-class %s | %FileCheck --match-full-lines %s
// REQUIRES: es6_class

const Parent = class {
  constructor(id) {
    this.id = id;
  }
};

const parentInstance = new Parent(42);

print(parentInstance.id);
//CHECK: 42

const childInstance = [0, new (class extends Parent {
  constructor() {
    super(1);
  }

  getDebugInfo() {
    return `MyID: ${this.id}`;
  }
})][1];

print(childInstance.id);
//CHECK: 1

print(childInstance.getDebugInfo());
//CHECK: MyID: 1
