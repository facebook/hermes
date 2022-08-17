/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

function func1() {
  var var1_0 = 10;
  var var1_1 = 11;

  function func2(elem) {
    var var2_1 = 21;
    debugger;
  }

  // Use forEach to test listing variables when the call stack contains a native
  // function (D6424654)
  ['a', 'b'].forEach(func2);
}

func1();

// CHECK: Break on 'debugger' statement in func2: {{.*}}
// CHECK-NEXT: Selected frame 0
// CHECK-NEXT: this = undefined
// CHECK-NEXT: 0: var2_1 = 21
// CHECK-NEXT: 0: elem = a
// CHECK-NEXT: 1: var1_0 = 10
// CHECK-NEXT: 1: var1_1 = 11
// CHECK-NEXT: 1: func2 = function func2(a0) { [bytecode] }
// CHECK-NEXT: Selected frame 1
// CHECK-NEXT: this = a,b
// CHECK-NEXT: Selected frame 2
// CHECK-NEXT: this = undefined
// CHECK-NEXT: 0: var1_0 = 10
// CHECK-NEXT: 0: var1_1 = 11
// CHECK-NEXT: 0: func2 = function func2(a0) { [bytecode] }
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: Break on 'debugger' statement in func2: {{.*}}
// CHECK-NEXT: Selected frame 0
// CHECK-NEXT: this = undefined
// CHECK-NEXT: 0: var2_1 = 21
// CHECK-NEXT: 0: elem = b
// CHECK-NEXT: 1: var1_0 = 10
// CHECK-NEXT: 1: var1_1 = 11
// CHECK-NEXT: 1: func2 = function func2(a0) { [bytecode] }
// CHECK-NEXT: Selected frame 1
// CHECK-NEXT: this = a,b
// CHECK-NEXT: Selected frame 2
// CHECK-NEXT: this = undefined
// CHECK-NEXT: 0:     var1_0 = 10
// CHECK-NEXT: 0:     var1_1 = 11
// CHECK-NEXT: 0: func2 = function func2(a0) { [bytecode] }
// CHECK-NEXT: Continuing execution
