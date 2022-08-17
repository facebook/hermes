/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

print('conditional break');
// CHECK-LABEL: conditional break

debugger;
for (var i = 0; i < 10; ++i) {
  print('i =', i);
}
// CHECK-NEXT: Break on 'debugger' statement in global: {{.*}}:14:1
// CHECK-NEXT: Set breakpoint 1 at {{.*}}:16:3 if i > 5 || i === 3
// CHECK-NEXT: Deleted breakpoint 1
// CHECK-NEXT: Set breakpoint 2 at {{.*}}:16:3 if i > 5 || i === 3
// CHECK-NEXT: Deleted breakpoint 2
// CHECK-NEXT: Set breakpoint 3 at {{.*}}:16:3 if i > 5 || i === 3
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: i = 0
// CHECK-NEXT: i = 1
// CHECK-NEXT: i = 2
// CHECK-NEXT: Break on breakpoint 3 in global: {{.*}}:16:3
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: i = 3
// CHECK-NEXT: i = 4
// CHECK-NEXT: i = 5
// CHECK-NEXT: Break on breakpoint 3 in global: {{.*}}:16:3
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: i = 6
// CHECK-NEXT: Break on breakpoint 3 in global: {{.*}}:16:3
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: i = 7
// CHECK-NEXT: Break on breakpoint 3 in global: {{.*}}:16:3
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: i = 8
// CHECK-NEXT: Break on breakpoint 3 in global: {{.*}}:16:3
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: i = 9
