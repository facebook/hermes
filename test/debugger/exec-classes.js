/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

class A {
  baseInstMethod() {
    return "baseInstMethod";
  }
  static baseStaticMethod() {
    return "baseStaticMethod";
  }
}

class B extends A {
  derivedInstMethod() {
    debugger;
  }
  static derivedStaticMethod() {
    debugger;
  }
}

new B().derivedInstMethod();
B.derivedStaticMethod();

// CHECK: Break on 'debugger' statement in derivedInstMethod: {{.*}}:22:5
// CHECK-NEXT: baseInstMethod
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: Break on 'debugger' statement in derivedStaticMethod: {{.*}}:25:5
// CHECK-NEXT: baseStaticMethod
// CHECK-NEXT: Continuing execution
