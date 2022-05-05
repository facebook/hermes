/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %juno %s --gen-resolved-js | %FileCheck %s --match-full-lines

(function(y) {
  for (let x = 1; z; x) {
    let z;
    x;
    y;
    z;
  }
})

// CHECK-LABEL: (function(y@D0) {
// CHECK-NEXT:   for(let x@D1 = 1; z@uglobal; x@D1) {
// CHECK-NEXT:     let z@D3;
// CHECK-NEXT:     x@D1;
// CHECK-NEXT:     y@D0;
// CHECK-NEXT:     z@D3;
// CHECK-NEXT:   }
// CHECK-NEXT: });
