/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger
// Ensure that "exec var" in a function doesn't create a global property.

function foo(x) {
  var x = 10;
  debugger;
  print(x);
}

print('start');
foo();
print('prop =', this.prop);
print('end');

//CHECK:start
//CHECK-NEXT:Break on 'debugger' statement in foo: {{.*}}declare-in-function.js[{{[0-9]+}}]:14:3
//CHECK-NEXT:10
//CHECK-NEXT:11
//CHECK-NEXT:undefined
//CHECK-NEXT:Continuing execution
//CHECK-NEXT:11
//CHECK-NEXT:prop = undefined
//CHECK-NEXT:end
