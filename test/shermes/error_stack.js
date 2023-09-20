/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// Ensure debug info level is high enough to trigger stack traces with lines
// RUN: (%shermes -exec %s -g1 2>&1 || true) | %FileCheckOrRegen --match-full-lines %s

// Test the error constructor.
function bar(){
  var err = new Error();
  throw err;
}
function foo(){
  bar();
}
var arr = [1,2,3];
try {
  const found = arr.findLast(element => element == 3 ? foo() : false);
} catch (e){
  print(e.stack);
}

// Test an error being thrown.
try {
  (function(){
    "use strict";
    let a = Foo.bar;
  })();
} catch (e) {
  print(e.stack);
}

// Test Error.captureStackTrace
(function(){
  let err = {};
  Error.captureStackTrace(err);
  print(err.stack);
})();

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}error_stack.js:30:13: warning: the variable "Foo" was not declared in anonymous function
// CHECK-NEXT:    let a = Foo.bar;
// CHECK-NEXT:            ^~~
// CHECK-NEXT:Error
// CHECK-NEXT:    at bar ({{.*}}error_stack.js:13:22)
// CHECK-NEXT:    at foo ({{.*}}error_stack.js:17:6)
// CHECK-NEXT:    at anonymous ({{.*}}error_stack.js:21:59)
// CHECK-NEXT:    at findLast (<anonymous>)
// CHECK-NEXT:    at global ({{.*}}error_stack.js:21:29)
// CHECK-NEXT:ReferenceError: Property 'Foo' doesn't exist
// CHECK-NEXT:    at global ({{.*}}error_stack.js:30:13)
// CHECK-NEXT:Error
// CHECK-NEXT:    at global ({{.*}}error_stack.js:39:26)
