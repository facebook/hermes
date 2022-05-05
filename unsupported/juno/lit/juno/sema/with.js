/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %juno %s --gen-resolved-js | %FileCheck %s --match-full-lines

let outer;
function foo(x) {
  with (x) {
    let y;
    outer;
    x;
    y;
    z;
  }
}

// CHECK-LABEL: let outer@D0;
// CHECK-NEXT: function foo@global(x@D2) {
// CHECK-NEXT:   with (x@D2) {
// CHECK-NEXT:     let y@D3;
// CHECK-NEXT:     outer@unresolvable;
// CHECK-NEXT:     x@unresolvable;
// CHECK-NEXT:     y@D3;
// CHECK-NEXT:     z@unresolvable;
// CHECK-NEXT:   }
// CHECK-NEXT: }
