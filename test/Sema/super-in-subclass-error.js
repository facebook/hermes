/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes %s -dump-sema 2>&1 ) | %FileCheck --match-full-lines %s


class A {}

class B {
  constructor() {
    super();
  }
}
// CHECK: {{.*}} error: super() call only allowed in subclass constructor

class C {
  constructor() {
    let arr = () => {
      super();
    };
  }
}
// CHECK: {{.*}} error: super() call only allowed in subclass constructor

class E extends A {
  constructor() {
    function norm() {
      super();
    }
  }
}
// CHECK: {{.*}} error: super() call only allowed in constructor

var o = {
  m1() {
    super();
  }
}
// CHECK: {{.*}} error: super() call only allowed in constructor
