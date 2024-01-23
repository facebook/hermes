/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xes6-class %s | %FileCheck --match-full-lines %s
// REQUIRES: es6_class

class Parent {
  prop = print('Parent Prop');

  constructor() {
    print('Parent ctor');
  }
}

class Child extends Parent {
  lastName = print('Child Prop');

  constructor() {
    print('Child ctor before super');
    super(), print('Child ctor in sequence expression');
    print('Child ctor after super');
  }
}

new Child();
//CHECK: Child ctor before super
//CHECK: Parent Prop
//CHECK: Parent ctor
//CHECK: Child Prop
//CHECK: Child ctor in sequence expression
//CHECK: Child ctor after super
