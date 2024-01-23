/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xes6-class %s | %FileCheck --match-full-lines %s
// REQUIRES: es6_class

class Test {
  constructor(getter) {
    if (getter) {
      this.getMyNumber = getter;
    }
  }

  getMyNumber() {
    return 42;
  }
}

const instance1 = new Test();
print(instance1.getMyNumber());
//CHECK: 42

const instance2 = new Test(() => 1);

print(instance2.getMyNumber());
//CHECK: 1
