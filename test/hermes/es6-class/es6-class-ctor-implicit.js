/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s

class Parent {
  constructor(id) {
    this.id = id;
  }
}

class Child extends Parent {}

const childInstance = new Child(1);

print(childInstance.id);
//CHECK: 1
