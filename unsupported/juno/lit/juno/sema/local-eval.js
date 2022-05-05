/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %juno %s --gen-resolved-js | %FileCheck %s --match-full-lines

let outer;
function foo(x) {
  eval(x);
  x;
  {
    let y;
    outer;
    x;
    y;
    z;
  }
}

// CHECK-LABEL: let outer@D0;
// CHECK-NEXT: function foo@unresolvable(x@D2) {
// CHECK-NEXT:   eval@unresolvable(x@D2);
// CHECK-NEXT:   x@D2;
// CHECK-NEXT:   {
// CHECK-NEXT:     let y@D4;
// CHECK-NEXT:     outer@unresolvable;
// CHECK-NEXT:     x@D2;
// CHECK-NEXT:     y@D4;
// CHECK-NEXT:     z@unresolvable;
// CHECK-NEXT:   }
// CHECK-NEXT: }
