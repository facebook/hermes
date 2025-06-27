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
    // The environment can't be tracked across this yield, so we should expect to just get 'undefined'
    debugger;
  }
  let genObj = gen1();
  genObj.next();
  genObj.next();
})();
// CHECK: Break on 'debugger' statement in gen1: {{.*}}:16:5
// CHECK-NEXT: undefined
// CHECK-NEXT: Continuing execution
