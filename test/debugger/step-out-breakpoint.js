/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb %s < %s.debug | %FileCheckOrRegen --match-full-lines %s
// REQUIRES: debugger

var obj = {
  get number() {
    debugger;
  },
};

function foo(a) {
  // Set a breakpoint on the `.` here so that we have a breakpoint on GetById.
  // Then stepping out needs to account for the size of GetById, not Debugger
  // when setting the step-out breakpoint.
  a.number;
}

foo(obj);

// Auto-generated content below. Please do not modify manually.

// CHECK:Break on 'debugger' statement in get number: {{.*}}step-out-breakpoint.js[2]:13:5
// CHECK-NEXT:Set breakpoint 1 at {{.*}}step-out-breakpoint.js[2]:21:4
// CHECK-NEXT:Stepped to foo: {{.*}}step-out-breakpoint.js[2]:21:4
