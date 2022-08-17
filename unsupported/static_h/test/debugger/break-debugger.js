/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb --break-at-start %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

print('first');
debugger;
print('second');
debugger;
print('third');

// CHECK: Break on script load in global: {{.*}}:11:1
// CHECK-NEXT: Set breakpoint 1 at {{.*}}:12:1
// CHECK-NEXT: Set breakpoint 2 at {{.*}}:14:1
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: first
// CHECK-NEXT: Break on breakpoint 1 in global: {{.*}}:12:1
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: second
// CHECK-NEXT: Break on breakpoint 2 in global: {{.*}}:14:1
// CHECK-NEXT: Stepped to global: {{.*}}:15:1
// CHECK-NEXT: third
