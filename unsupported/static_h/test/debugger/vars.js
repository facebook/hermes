/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

function func0() {
    var var0 = 10;
}

function func1() {
  var var1 = 5;
  var var22 = 17;
  function func2() {
    var var333 = "c";
    debugger;
    func0();
  }
  func2();
}

function var4() {
  var v5 = 0;
}

var v6 = undefined;

func1();

// CHECK: Break on 'debugger' statement in func2:{{.*}}
// CHECK-NEXT: this = undefined
// CHECK-NEXT: 0: var333 = c
// CHECK-NEXT: 1: var1 = 5
// CHECK-NEXT: 1: var22 = 17
// CHECK-NEXT: 1: func2 = function func2() { [bytecode] }
// CHECK-NEXT: Selected frame 1
// CHECK-NEXT: this = undefined
// CHECK-NEXT: 0: var1 = 5
// CHECK-NEXT: 0: var22 = 17
// CHECK-NEXT: 0: func2 = function func2() { [bytecode] }
// CHECK-NEXT: Selected frame 2
// CHECK-NEXT: this = [object global]
// CHECK-NEXT: Stepped to func2: {{.*}}
// CHECK-NEXT: Stepped to func0: {{.*}}
// CHECK-NEXT: Selected frame 2
// CHECK-NEXT: this = undefined
// CHECK-NEXT: 0: var1 = 5
// CHECK-NEXT: 0: var22 = 17
// CHECK-NEXT: 0: func2 = function func2() { [bytecode] }
// CHECK-NEXT: Selected frame 1
// CHECK-NEXT: this = undefined
// CHECK-NEXT: 0: var333 = c
// CHECK-NEXT: 1: var1 = 5
// CHECK-NEXT: 1: var22 = 17
// CHECK-NEXT: 1: func2 = function func2() { [bytecode] }
// CHECK-NEXT: Selected frame 0
// CHECK-NEXT: this = undefined
// CHECK-NEXT: 0: var0 = undefined
// CHECK-NEXT: Continuing execution
