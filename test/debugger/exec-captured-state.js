/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

function foo() {
  function bar() {
    function baz() {
      debugger;
    }
    baz.call(3);
  }
  bar.call('asdf');
}

foo.call(10);

(function () {
  function foo() {
    debugger;
  }
  function bar() { }
  bar.prop = 42;
  Reflect.construct(foo, [], bar);
})();

(function () {
  function foo() {
    let arr = () => {
      debugger;
    };
    this.a = 10;
    arr();
  }
  foo.b = 11;
  new foo();
})();

var o = {
  __proto__: { prop: 42 },
  m1() {
    let arr = () => {
      debugger;
    };
    arr();
  }
};
o.m1();

// CHECK: Break on 'debugger' statement in baz: {{.*}}:14:7
// CHECK-NEXT: 3
// CHECK-NEXT: 10
// CHECK-NEXT: Stepped to bar: {{.*}}:16:13
// CHECK-NEXT: asdf
// CHECK-NEXT: asdfasdf
// CHECK-NEXT: Stepped to foo: {{.*}}:18:11
// CHECK-NEXT: 10
// CHECK-NEXT: 13
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: Break on 'debugger' statement in foo: {{.*}}:25:5
// CHECK-NEXT: 42
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: Break on 'debugger' statement in arr: {{.*}}:35:7
// CHECK-NEXT: 10
// CHECK-NEXT: 11
// CHECK-NEXT: Stepped to foo: {{.*}}:38:8
// CHECK-NEXT: 10
// CHECK-NEXT: 11
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: Break on 'debugger' statement in arr: {{.*}}:48:7
// CHECK-NEXT: 42
// CHECK-NEXT: Stepped to m1: {{.*}}:50:8
// CHECK-NEXT: 42
