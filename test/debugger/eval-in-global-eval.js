/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb < %s.debug %s | %FileCheckOrRegen --match-full-lines %s
// REQUIRES: debugger

(1,eval)(`
function main() {
  var x = 1;
  function foo() {
    debugger;
  }
  foo();
}
main();
`);

// Auto-generated content below. Please do not modify manually.

// CHECK:Break on 'debugger' statement in foo: [4]:5:5
// CHECK-NEXT:1
// CHECK-NEXT:Continuing execution
