/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -dump-sema %s 2>&1 ) | %FileCheck %s --match-full-lines

const x = 1;
x = 2;
// CHECK:{{.*}}const-reassignment.js:11:1: error: invalid assignment left-hand side
// CHECK-NEXT:x = 2;
// CHECK-NEXT:^

(function funcName() {
  // This is ok when not in loose mode.
  funcName = 1;
})();

(function () {
  "use strict";
  (function funcName() {
    // This errors in strict mode.
    funcName = 1;
// CHECK-NEXT:{{.*}}const-reassignment.js:25:5: error: invalid assignment left-hand side
// CHECK-NEXT:    funcName = 1;
// CHECK-NEXT:    ^~~~~~~~
  })();
})();
