/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

print('step debugger');
// CHECK-LABEL: step debugger

debugger;
print('first');
print('second');

// CHECK-NEXT: Break on 'debugger' statement in global: {{.*}}:14:1
// CHECK-NEXT: Stepped to global: {{.*}}:15:1
// CHECK-NEXT: first
// CHECK-NEXT: Stepped to global: {{.*}}:16:1
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: second
