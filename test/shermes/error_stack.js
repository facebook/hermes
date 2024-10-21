/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// Ensure debug info level is high enough to trigger stack traces with lines
// RUN: %shermes -exec %s -g1 | %FileCheck --match-full-lines %s

print('error');
// CHECK-LABEL: error

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
// CHECK-NEXT:Error
// CHECK-NEXT:    at bar ({{.*}}error_stack.js{{.*}})
// CHECK-NEXT:    at foo ({{.*}}error_stack.js{{.*}})
// CHECK-NEXT:    at anonymous ({{.*}}error_stack.js{{.*}})
// CHECK-NEXT:    at findLast (native)
// CHECK-NEXT:    at global ({{.*}}error_stack.js{{.*}})
}

// Test an error being thrown.
try {
  (function(){
    "use strict";
    let a = Foo.bar;
  })();
} catch (e) {
  print(e.stack);
// CHECK-NEXT:ReferenceError: Property 'Foo' doesn't exist
// CHECK-NEXT:    at anonymous ({{.*}}error_stack.js{{.*}})
// CHECK-NEXT:    at global ({{.*}}error_stack.js{{.*}})
}

// Test Error.captureStackTrace
(function(){
  let err = {};
  Error.captureStackTrace(err);
  print(err.stack);
// CHECK-NEXT:Error
// CHECK-NEXT:    at anonymous ({{.*}}error_stack.js{{.*}})
// CHECK-NEXT:    at global ({{.*}}error_stack.js{{.*}})
})();

