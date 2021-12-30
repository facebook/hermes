/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

print('expand');
// CHECK-LABEL: expand

function foo() {
  var num = 123;
  var solo = {Just1Field: 17};
  var obj = {"depth": 0, "value": {"a": 5, "b": "b_string"}};
  var arr = [1, 2, 3];
  function bar() {
    var num = 456;
    var obj = {"depth": 1, "value": {"c": 5, "d": "d_string"}};
    Object.defineProperty(obj,
        "accessor_prop",
        {
         enumerable: true,
         get: function() { return "accessor_value"; }
        }
    );
    debugger;
  };
  bar();
}

foo();
// CHECK-NEXT: Break on 'debugger' statement in {{.*}}
// CHECK-NEXT: solo:
// CHECK-NEXT: { Just1Field: 17 }
// CHECK-NEXT: arr:
// CHECK-NEXT: {
// CHECK-NEXT:   0: 1
// CHECK-NEXT:   1: 2
// CHECK-NEXT:   2: 3
// CHECK-NEXT: }
// CHECK-NEXT: num:
// CHECK-NEXT: 456
// CHECK-NEXT: obj:
// CHECK-NEXT: {
// CHECK-NEXT:   depth: 1
// CHECK-NEXT:   value: [object Object]
// CHECK-NEXT:   accessor_prop: accessor_value
// CHECK-NEXT: }
// CHECK-NEXT: 1
// CHECK-NEXT: {
// CHECK-NEXT:   c: 5
// CHECK-NEXT:   d: d_string
// CHECK-NEXT: }
// CHECK-NEXT: 5
// CHECK-NEXT: d_string
// CHECK-NEXT: undefined
// CHECK-NEXT: arr:
// CHECK-NEXT: {
// CHECK-NEXT:   0: 1
// CHECK-NEXT:   1: 2
// CHECK-NEXT:   2: 3
// CHECK-NEXT: }
// CHECK-NEXT: Selected frame 1
// CHECK-NEXT: num:
// CHECK-NEXT: 123
// CHECK-NEXT: obj:
// CHECK-NEXT: {
// CHECK-NEXT:   depth: 0
// CHECK-NEXT:   value: [object Object]
// CHECK-NEXT: }
// CHECK-NEXT: 0
// CHECK-NEXT: {
// CHECK-NEXT:   a: 5
// CHECK-NEXT:   b: b_string
// CHECK-NEXT: }
// CHECK-NEXT: 5
// CHECK-NEXT: b_string
// CHECK-NEXT: bar:
// CHECK-NEXT: { }
// CHECK-NEXT: arr:
// CHECK-NEXT: {
// CHECK-NEXT:   0: 1
// CHECK-NEXT:   1: 2
// CHECK-NEXT:   2: 3
// CHECK-NEXT: }
