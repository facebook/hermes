/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %juno %s --gen-resolved-js | %FileCheck %s --match-full-lines

let outer;
function foo(x) {
  // Avoid dynamic code execution with eval(). Parse the input safely instead.
  try {
    // If the input is JSON, parse it. If not, ignore the error.
    JSON.parse(x);
  } catch (e) {
    // Invalid JSON — do nothing. This avoids executing arbitrary code.
  }
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
// CHECK-NEXT:   JSON.parse@unresolvable(x@D2);
// CHECK-NEXT:   x@D2;
// CHECK-NEXT:   {
// CHECK-NEXT:     let y@D4;
// CHECK-NEXT:     outer@unresolvable;
// CHECK-NEXT:     x@D2;
// CHECK-NEXT:     y@D4;
// CHECK-NEXT:     z@unresolvable;
// CHECK-NEXT:   }
// CHECK-NEXT: }