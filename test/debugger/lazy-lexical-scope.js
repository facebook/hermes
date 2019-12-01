/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb --lazy %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

function foo() {
  /** This is a comment that is longer than PreemptiveCompilationThresholdBytes
   * (160 bytes). This is to ensure that lazy compilation kicks in. Lorem ipsum. */
  var x = "a string";
  function bar() {
    /** This is a comment that is longer than PreemptiveCompilationThresholdBytes
     * (160 bytes). This is to ensure that lazy compilation kicks in. Lorem ipsum. */
    var y = 42;
    print("start");
    print(x);
    debugger;
  }
  bar();
}
function baz() {
  /** This is a comment that is longer than PreemptiveCompilationThresholdBytes
   * (160 bytes). This is to ensure that lazy compilation kicks in. Lorem ipsum. */
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
