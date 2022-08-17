/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb --break-at-start %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

print('foo');
  print('bar');
print('baz');

// CHECK: Break on script load in global: {{.*}}:11:1
// CHECK-NEXT: Set breakpoint 1 at {{.*}}:12:3
// CHECK-NEXT: 1 E {{.*}}break-line.js:12:3
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: foo
// CHECK-NEXT: Break on breakpoint 1 in global: {{.*}}:12:3
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: bar
// CHECK-NEXT: baz
