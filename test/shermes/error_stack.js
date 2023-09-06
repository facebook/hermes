/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// Ensure debug info level is high enough to trigger stack traces with lines
// RUN: (%shermes -exec %s -g1 2>&1 || true) | %FileCheckOrRegen --match-full-lines %s

function bar(){
  var err = new Error();
  throw err;
}
function foo(){
  bar();
}
var arr = [1,2,3];
const found = arr.findLast(element => element == 3 ? foo() : false);

// Auto-generated content below. Please do not modify manually.

// CHECK:Uncaught Error
// CHECK-NEXT:    at bar ({{.*}}error_stack.js:12:22)
// CHECK-NEXT:    at foo ({{.*}}error_stack.js:16:6)
// CHECK-NEXT:    at anonymous ({{.*}}error_stack.js:19:57)
// CHECK-NEXT:    at findLast (<anonymous>)
// CHECK-NEXT:    at global ({{.*}}error_stack.js:19:27)
