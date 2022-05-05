/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %juno %s --gen-resolved-js | %FileCheck %s --match-full-lines

(function() {
var a;
let b;
function c() {}
{
  function a() {}
  function b() {}
  function c() {}
}
{
  let a;
}
});

// CHECK-LABEL: (function() {
// CHECK-NEXT:   var a@D0;
// CHECK-NEXT:   let b@D1;
// CHECK-NEXT:   function c@D2() {}
// CHECK-NEXT:   {
// CHECK-NEXT:     function a@D0() {}
// CHECK-NEXT:     function b@D3() {}
// CHECK-NEXT:     function c@D2() {}
// CHECK-NEXT:   }
// CHECK-NEXT:   {
// CHECK-NEXT:     let a@D4;
// CHECK-NEXT:   }
// CHECK-NEXT: });
