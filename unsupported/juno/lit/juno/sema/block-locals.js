/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %juno %s --gen-resolved-js | %FileCheck %s --match-full-lines

let x;
x;

{
  let x;
  x;
}

function foo() {
  let x;
  x;
  foo;
  {
    let x;
    x;
  }
}

// CHECK-LABEL: let x@D0;
// CHECK-NEXT: x@D0;
// CHECK-NEXT: {
// CHECK-NEXT:   let x@D2;
// CHECK-NEXT:   x@D2;
// CHECK-NEXT: }
// CHECK-NEXT: function foo@global() {
// CHECK-NEXT:   let x@D3;
// CHECK-NEXT:   x@D3;
// CHECK-NEXT:   foo@global;
// CHECK-NEXT:   {
// CHECK-NEXT:     let x@D4;
// CHECK-NEXT:     x@D4;
// CHECK-NEXT:   }
// CHECK-NEXT: }
