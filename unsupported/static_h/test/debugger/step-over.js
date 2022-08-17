/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb < %s.debug %s | %FileCheck --match-full-lines %s
// RUN: %hdb --lazy < %s.debug %s | %FileCheck --match-full-lines %s
// REQUIRES: debugger

print('step over');
// CHECK-LABEL: step over

function happy() {
  print('happy');
  print('stepping');
  return 'this is the result';
}

debugger;
print('first');
print(happy());
print('second');

// CHECK: Break on 'debugger' statement in global: {{.*}}:21:1
// CHECK-NEXT: Stepped to global: {{.*}}:22:1
// CHECK-NEXT: first
// CHECK-NEXT: Stepped to global: {{.*}}:23:1
// CHECK-NEXT: happy
// CHECK-NEXT: stepping
// CHECK-NEXT: this is the result
// CHECK-NEXT: Stepped to global: {{.*}}:24:1
// CHECK-NEXT: second
