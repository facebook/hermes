/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s
// RUN: %hermes -lazy %s | %FileCheck --match-full-lines %s

(function main() {
  var x = 'outside';
  var paramsFunc;

  function foo(
    _ = (paramsFunc = function bar() {
      // Capture the outer 'x'
      return x;
    })
  ) {
    let x = 'inside';
  }
  foo();

  print(paramsFunc());
  // CHECK: outside
})();
