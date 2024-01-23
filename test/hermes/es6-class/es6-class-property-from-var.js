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
  public: 'myPublicProperty'
}

class Test {
  get [keys.private]() {
    return `Private: ${this._private}`;
  }

  set [keys.private](v) {
    this._private = v;
  }

  get [keys.public]() {
    return `Public: ${this._public}`;
  }

  set [keys.public](v) {
    this._public = v;
  }
}

const object = new Test();

print(object.myPublicProperty);
// CHECK: Public: undefined

object.myPublicProperty = '42';

print(object.myPublicProperty);
// CHECK: Public: 42

print(object[keys.private]);
// CHECK: Private: undefined

object[keys.private] = '1';

print(object[keys.private]);
// CHECK: Private: 1
