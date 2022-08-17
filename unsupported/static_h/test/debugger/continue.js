/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: echo continue | %hdb %s | %FileCheck --match-full-lines %s
// REQUIRES: debugger

print("continue")
// CHECK-LABEL: continue
function foo() {
  var x = 1;
  debugger;
  x = 2;
  return x;
}
print(foo())
// CHECK-NEXT: Break on 'debugger' statement in foo: {{.*}}:15:3
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: 2
