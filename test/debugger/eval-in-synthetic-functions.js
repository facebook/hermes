/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

(function () {
  let outerVar = 20;
  function foo() {
    let var1 = 45;
    debugger;
  }
  class A {
    f1 = foo();
  }
  new A();
})();
// CHECK: Break on 'debugger' statement in foo: {{.*}}:15:5
// CHECK-NEXT: 45
// CHECK-NEXT: Selected frame 1
// CHECK-NEXT: 20
// CHECK-NEXT: Selected frame 2
// CHECK-NEXT: 20
// CHECK-NEXT: Continuing execution

(function () {
  let outerVar = 30;
  function foo() { debugger; }
  class B {
    static f2 = foo();
  }
  new B();
})();
// CHECK: Break on 'debugger' statement in foo: {{.*}}:32:20
// CHECK-NEXT: Selected frame 1
// CHECK-NEXT: 30
// CHECK-NEXT: Selected frame 2
// CHECK-NEXT: 30
// CHECK-NEXT: Continuing execution

(function () {
  let outerVar = 40;
  function foo() { debugger; }
  class C {
    static {
      foo();
    }
  }
})();
// CHECK: Break on 'debugger' statement in foo: {{.*}}:47:20
// CHECK-NEXT: Selected frame 1
// CHECK-NEXT: 40
// CHECK-NEXT: Continuing execution
