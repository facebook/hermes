/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb --break-at-start %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

function foo() {
  print('first');
  print('second'); print('third'); print("fourth");
  print('fifth'); print('sixth');
}

function bar() {
  foo();
}

bar();
// CHECK-LABEL: Break on script load {{.+}}

// column=1 is before all statements. Match the first.
// CHECK-NEXT: Set breakpoint 1 at {{.+}}:12:3

// column=22 is the middle of the 'third' print. Match that.
// CHECK-NEXT: Set breakpoint 2 at {{.+}}:13:20

// column=200 is after all statements. Match the last.
// CHECK-NEXT: Set breakpoint 3 at {{.+}}:14:24

// CHECK-NEXT: Continuing execution
// CHECK-NEXT: Break on breakpoint 1 in foo: {{.+}}:12:3
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: first
// CHECK-NEXT: second
// CHECK-NEXT: Break on breakpoint 2 in foo: {{.+}}:13:20
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: third
// CHECK-NEXT: fourth
// CHECK-NEXT: fifth
// CHECK-NEXT: Break on breakpoint 3 in foo: {{.+}}:14:24
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: sixth
