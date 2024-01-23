/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xes6-class %s | %FileCheck --match-full-lines %s
// REQUIRES: es6_class

const keys = {
  private: Symbol(),
  public: 'myPublicMethod'
}

class Test {
  [keys.private]() {
    print('Hello from private method!');
  }

  [keys.public]() {
    print('Hello from public method!');
  }
}

const object = new Test();

object.myPublicMethod();
// CHECK: Hello from public method!

object[keys.private]();
// CHECK: Hello from private method!
