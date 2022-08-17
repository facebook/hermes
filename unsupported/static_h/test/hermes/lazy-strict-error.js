/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -non-strict -O0 %s 2>&1 | %FileCheck --match-full-lines %s
// RUN: %hermes -lazy -non-strict -O0 %s 2>&1 | %FileCheck --match-full-lines %s

// foo is never called, but its strictness must be set correctly.
function foo(x) {
  "use strict";
}

(function bar() {
  try {
    // .caller access is forbidden on strict functions.
    print(foo.caller);
  } catch (e) {
    print('caught', e.name);
  }
})();
// CHECK: caught TypeError
