/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb --lazy %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

// @nolint
function foo() { print('foo called'); /** * Some text to pad out the function so that it won't be eagerly compiled * for being too short. Lorem ipsum dolor sit amet, consectetur adipiscing * elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.  */ function bar() { print('bar called'); /** * Some text to pad out the function so that it won't be eagerly compiled * for being too short. Lorem ipsum dolor sit amet, consectetur adipiscing * elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.  */ } function baz() { print('baz called'); /** * Some text to pad out the function so that it won't be eagerly compiled * for being too short. Lorem ipsum dolor sit amet, consectetur adipiscing * elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.  */ }; bar(); baz(); }

debugger;
foo();

/* This test verifies that you can set breakpoints in uncompiled
 * functions, without fuzzy column matching accidentally resolving the
 * breakpoint to the closest already-compiled instruction it can find.
 * This comes up when debugging minified source with source maps.
 */

// CHECK: Break on 'debugger' statement in global: {{.*}}:14:1

// Breakpoint is in the middle of the 'print' for 'bar called'
// CHECK-NEXT: Set breakpoint 1 at {{.*}}:12:286
// Breakpoint is in the middle of the 'print' for 'baz called'
// CHECK-NEXT: Set breakpoint 2 at {{.*}}:12:556

// CHECK-NEXT: Continuing execution
// CHECK-NEXT: foo called
// CHECK-NEXT: Break on breakpoint 1 in bar: {{.*}}:12:286
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: bar called
// CHECK-NEXT: Break on breakpoint 2 in baz: {{.*}}:12:556
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: baz called
