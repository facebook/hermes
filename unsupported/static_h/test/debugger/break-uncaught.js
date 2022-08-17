/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hdb %s --break-at-start < %s.debug 2>&1) | %FileCheck --match-full-lines %s
// REQUIRES: debugger

// CHECK: Continuing execution

try { throw new Error('asdf') } catch (e) { print('caught', e); }
// CHECK-NEXT: caught Error: asdf

function foo() {
  throw new Error('asdf');
}
foo();
// CHECK-NEXT: Break on exception in foo: {{.*}}:17:3
// CHECK-NEXT: > 0: foo: {{.*}}:17:3
// CHECK-NEXT:   1: global: {{.*}}:19:4
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: JavaScript terminated via uncaught exception: asdf

// CHECK:      Error: asdf
// CHECK-NEXT:     at foo ({{.*}}:17:18)
// CHECK-NEXT:     at global ({{.*}}:19:4)
