/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s -test262 | %FileCheck --match-full-lines %s

const x = 1;
try {
  x = 2;
} catch (e) {
  print(e.constructor.name);
// CHECK:TypeError
}

(function funcName() {
  // This is ok when not in loose mode.
  funcName = 1;
})();

(function () {
  "use strict";
  (function funcName() {
    try {
      // This errors in strict mode.
      funcName = 1;
    } catch (e) {
      print(e.constructor.name);
// CHECK-NEXT:TypeError
    }
  })();
})();

