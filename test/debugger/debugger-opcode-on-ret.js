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
    // We'll install the breakpoint on the Ret for the next line.
    return 1;
  }
};

function test() {
  var arr = [2, 3, 4];
  arr.forEach(function(element) {
    print(element);
    return 1;
  });
}

debugger;

test();

// When paused in get number(), do a couple 'continue'.
print(obj.number);
print(obj.number);

// Pause on debugger statement. Setup all the breakpoints we need.
// CHECK: Break on 'debugger' statement in global: {{.*}}:26:1
// CHECK-NEXT: Set breakpoint 1 at {{.*}}:14:5
// CHECK-NEXT: Set breakpoint 2 at {{.*}}:22:5
// CHECK-NEXT: Continuing execution

// Testing forEach:
// CHECK-NEXT: 2
// CHECK-NEXT: Break on breakpoint 2 in anonymous: {{.*}}:22:5
// CHECK-NEXT: Stepped to anonymous: {{.*}}:21:5
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: 3
// CHECK-NEXT: Break on breakpoint 2 in anonymous: {{.*}}:22:5
// CHECK-NEXT: Stepped to anonymous: {{.*}}:21:5
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: 4
// CHECK-NEXT: Break on breakpoint 2 in anonymous: {{.*}}:22:5
// CHECK-NEXT: Stepped to test: {{.*}}:20:14
// CHECK-NEXT: Continuing execution

// Test doing 'continue' on breakpoint placed on top of Ret:
// CHECK-NEXT: Break on breakpoint 1 in get number: {{.*}}:14:5
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: 1
// CHECK-NEXT: Break on breakpoint 1 in get number: {{.*}}:14:5
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: 1
