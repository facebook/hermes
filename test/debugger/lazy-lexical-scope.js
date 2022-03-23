/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb --lazy %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

function foo() {
  var x = "a string";
  function bar() {
    var y = 42;
    print("start");
    print(x);
    debugger;
  }
  bar();
}
function baz() {
  var z;
  foo();
}
baz()

// CHECK: start
// CHECK-NEXT: a string
// CHECK-NEXT: Break on 'debugger' statement in bar: {{.*}}
// CHECK-NEXT: this = undefined
// CHECK-NEXT:  0:          y = 42
// CHECK-NEXT:  1:          x = a string
// CHECK-NEXT:  1:        bar = function {{.*}}
