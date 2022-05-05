/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %juno %s --gen-resolved-js | %FileCheck %s --match-full-lines

function foo() {
  let x;
  var y;
  {
    function x() {}
    function y() {}
    function z() {}
  }
  x;
  y;
  z;
}

// CHECK-LABEL: function foo@global() {
// CHECK-NEXT:   let x@D1;
// CHECK-NEXT:   var y@D2;
// CHECK-NEXT:   {
// CHECK-NEXT:     function x@D4() {}
// CHECK-NEXT:     function y@D2() {}
// CHECK-NEXT:     function z@D3() {}
// CHECK-NEXT:   }
// CHECK-NEXT:   x@D1;
// CHECK-NEXT:   y@D2;
// CHECK-NEXT:   z@D3;
// CHECK-NEXT: }
