/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %juno %s --gen-resolved-js | %FileCheck %s --match-full-lines

(function(x) {
  let [y] = x;
  y;
})

// CHECK-LABEL: (function(x@D0) {
// CHECK-NEXT:   let [y@D1] = x@D0;
// CHECK-NEXT:   y@D1;
// CHECK-NEXT: });
