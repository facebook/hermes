/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb --Xes6-block-scoping %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

(function () {
  var v1 = 10;
  function foo() {
    let v2 = 20;
    while (true) {
      class B {} // Class declaration forces while loop to create a new scope / environment.
      let v3 = 30;
      debugger;
      return;
    }
  }
  foo();
})();

// CHECK: Break on 'debugger' statement in foo: {{.*}}:18:7
// CHECK-NEXT: 30
// CHECK-NEXT: 20
// CHECK-NEXT: 10
// CHECK-NEXT: Continuing execution
