/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

print('throw');
// CHECK-LABEL: throw

debugger;
try {
  throw new Error('asdf');
} catch(e) {
  print('caught it');
}

// CHECK: Break on 'debugger' statement in global: {{.*}}:14:1
// CHECK-NEXT: Set pauseOnThrow: all errors
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: Break on exception in global: {{.*}}:16:3
// CHECK-NEXT: Stepped to global: {{.*}}:18:3
// CHECK-NEXT: caught it
