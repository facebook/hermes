/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xes6-class %s | %FileCheck --match-full-lines %s
// REQUIRES: es6_class

class Parent {
  lastName = "Smith";
  firstName = "George";

  constructor(id) {
    this.id = id;
  }
}

class Child extends Parent {
  lastName = "Child";

  constructor(id) {
    super(id);
  }
}

const parentInstance = new Parent(42);

print(parentInstance.id);
//CHECK: 42

print(parentInstance.firstName);
//CHECK: George

print(parentInstance.lastName);
//CHECK: Smith

const childInstance = new Child(1);

print(childInstance.id);
//CHECK: 1

print(childInstance.firstName);
//CHECK: George

print(childInstance.lastName);
//CHECK: Child
