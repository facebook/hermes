/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC -emit-binary -out=%t %s && %hermes -O -b %t | %FileCheck --match-full-lines %s
// RUN: %hermes -O -Xmetro-require=false -target=HBC -emit-binary -out=%t %s && %hermes -O -b %t | %FileCheck --match-full-lines %s

function testBadRequire() {
  // We invoke the factory function with a require that is not a function.
  // The factory function invokes it.  This should throw a TypeError exception,
  // whether or not the requires optimization is enabled.
  $SHBuiltin.moduleFactory(
    1,
    function modFact1(global, require) {
      var req0 = require(0);
    })(undefined, 7);
}
try {
  testBadRequire();
} catch (x) {
  print(x);
  // CHECK-LABEL: TypeError: 7 is not a function
}
