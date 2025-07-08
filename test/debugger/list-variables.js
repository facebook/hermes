/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

(function () {
  function bar() {
    let l3A = 1;
    let l3B = 2;
    var l3C = 3;
    let l3D = 4;
    function foo() {
      let l2A = 5;
      let l2B = 6;
      let l2C = 7;
      while (true) {
        let l1A = 8;
        let l1B = 9;
        class C {}; // force new scope to be made.
        debugger;
// CHECK:Break on 'debugger' statement in foo: {{.*}}list-variables.js[2]:25:9
// CHECK-NEXT:this = undefined
// CHECK-NEXT: 0:        l1A = 8
// CHECK-NEXT: 0:        l1B = 9
// CHECK-NEXT: 1:        l2A = 5
// CHECK-NEXT: 1:        l2B = 6
// CHECK-NEXT: 1:        l2C = 7
// CHECK-NEXT: 2:        l3A = 1
// CHECK-NEXT: 2:        l3B = 2
// CHECK-NEXT: 2:        l3C = 3
// CHECK-NEXT: 2:        l3D = 4
        break;
      }
    }
    foo();
  }
  bar();
})();

(function () {
  function baz() {
    let a = 1;
    {
      let b = 2;
      debugger;
// CHECK:Break on 'debugger' statement in baz: {{.*}}list-variables.js[2]:50:7
// CHECK-NEXT:this = undefined
// CHECK-NEXT: 0:          b = 2
// CHECK-NEXT: 1:          a = 1
    }
    {
      let c = 3;
      debugger;
// CHECK:Break on 'debugger' statement in baz: {{.*}}list-variables.js[2]:58:7
// CHECK-NEXT:this = undefined
// CHECK-NEXT: 0:          c = 3
// CHECK-NEXT: 1:          a = 1
      {
        let d = 4;
        debugger;
// CHECK:Break on 'debugger' statement in baz: {{.*}}list-variables.js[2]:65:9
// CHECK-NEXT:this = undefined
// CHECK-NEXT: 0:          d = 4
// CHECK-NEXT: 1:          c = 3
// CHECK-NEXT: 2:          a = 1
      }
      {
        let e = 5;
        debugger;
// CHECK:Break on 'debugger' statement in baz: {{.*}}list-variables.js[2]:74:9
// CHECK-NEXT:this = undefined
// CHECK-NEXT: 0:          e = 5
// CHECK-NEXT: 1:          c = 3
// CHECK-NEXT: 2:          a = 1
      }
    }
    debugger;
// CHECK:Break on 'debugger' statement in baz: {{.*}}list-variables.js[2]:82:5
// CHECK-NEXT:this = undefined
// CHECK-NEXT: 0:          a = 1
  }
  baz();
})();
