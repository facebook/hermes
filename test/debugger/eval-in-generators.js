/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb --Xes6-block-scoping %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

(function () {
  function *gen1() {
    let x = 50;
    yield x;
    // The environment created to hold `x` should have been spilled across the above yield.
    debugger;
  }
  let genObj = gen1();
  genObj.next();
  genObj.next();
})();
// CHECK: Break on 'debugger' statement in gen1: {{.*}}:16:5
// CHECK-NEXT: 50
// CHECK-NEXT: Continuing execution

// Combine block scoping and generators.
(function () {
  var v1 = 10;
  function *gen2() {
    let v2 = 20;
    for (let i = 0; i < 2; i++){
      class B {} // Class declaration forces loop to create a new scope / environment.
      let v3 = 30;
      yield i;
      // The environment created to hold `v3` should have been spilled across the above yield.
      debugger;
    }
  }
  let genObj = gen2();
  genObj.next();
  genObj.next();
})();
// CHECK: Break on 'debugger' statement in gen2: {{.*}}:36:7
// CHECK-NEXT: 30
// CHECK-NEXT: 0
// CHECK-NEXT: Continuing execution
