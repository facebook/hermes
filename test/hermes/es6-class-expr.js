/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xes6-class %s | %FileCheck --match-full-lines %s

const Parent = class {
  constructor(id) {
    this.id = id;
  }
};

const parentInstance = new Parent(42);

print(parentInstance.id);
//CHECK: 42
