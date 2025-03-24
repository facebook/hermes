/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s

const Parent = class MyName {
  constructor(id) {
    this.id = id;
    MyName.lastId = id;
  }
};

new Parent(42);

print(typeof MyName);
//CHECK: undefined

print(Parent.lastId);
//CHECK: 42
