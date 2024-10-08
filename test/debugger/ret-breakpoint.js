/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb %s < %s.debug | %FileCheckOrRegen --match-full-lines %s
// REQUIRES: debugger

function foo() {
  debugger;
  // Ret has to check the actual opcode at the callsite,
  // which means it has to account for breakpoints being installed there.
  return 1;
}

function main(f) {
  return f();
}

print(main(foo));

// Auto-generated content below. Please do not modify manually.

// CHECK:Break on 'debugger' statement in foo: {{.*}}ret-breakpoint.js[2]:12:3
// CHECK-NEXT:Set breakpoint 1 at {{.*}}ret-breakpoint.js[2]:19:11
// CHECK-NEXT:Continuing execution
// CHECK-NEXT:1
