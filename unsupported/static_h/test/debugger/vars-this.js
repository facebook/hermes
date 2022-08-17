/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

print('vars this');
// CHECK-LABEL: vars this

function Foo() {}
Foo.prototype.f1 = function f1() {
  this.f2();
}
Foo.prototype.f2 = function f2() {
  function f3() {
    debugger;
  }
  f3.call(412);
}

var foo = new Foo();
foo.f1();

// CHECK-NEXT: Break on 'debugger' statement in f3: {{.*}}
// CHECK-NEXT: Selected frame 0
// CHECK-NEXT: this = 412
// CHECK-NEXT: 1: f3 = function f3() { [bytecode] }
// CHECK-NEXT: 2: ?anon_1_closure = function f1() { [bytecode] }
// CHECK-NEXT: 2: ?anon_2_closure = function f2() { [bytecode] }
// CHECK-NEXT: Selected frame 2
// CHECK-NEXT: this = [object Object]
// CHECK-NEXT: 0: f3 = function f3() { [bytecode] }
// CHECK-NEXT: 1: ?anon_1_closure = function f1() { [bytecode] }
// CHECK-NEXT: 1: ?anon_2_closure = function f2() { [bytecode] }
// CHECK-NEXT: Selected frame 3
// CHECK-NEXT: this = [object Object]
// CHECK-NEXT: 1: ?anon_1_closure = function f1() { [bytecode] }
// CHECK-NEXT: 1: ?anon_2_closure = function f2() { [bytecode] }
// CHECK-NEXT: Selected frame 4
// CHECK-NEXT: this = [object global]
// CHECK-NEXT: Continuing execution
