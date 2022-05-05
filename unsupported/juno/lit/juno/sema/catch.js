/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %juno %s --gen-resolved-js | %FileCheck %s --match-full-lines

(function(x) {
  try {
    x();
  } catch (y) {
    let z;
    x;
    y;
    z;
  }
});

// CHECK-LABEL: (function(x@D0) {
// CHECK-NEXT:   try {
// CHECK-NEXT:     x@D0();
// CHECK-NEXT:   } catch (y@D1) {
// CHECK-NEXT:     let z@D2;
// CHECK-NEXT:     x@D0;
// CHECK-NEXT:     y@D1;
// CHECK-NEXT:     z@D2;
// CHECK-NEXT:   }
// CHECK-NEXT: });
