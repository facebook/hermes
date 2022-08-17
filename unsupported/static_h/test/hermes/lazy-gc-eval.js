/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -lazy -non-strict -Wno-direct-eval %s | %FileCheck --match-full-lines %s

// `eval` used to clean up Module/Context needed for lazy compilation.
// Ensure that we can eval a new module, return an uncompiled closure,
// gc everything else, and still run the closure.
// Use global.eval to modify the global scope.
this.eval("function getLazy() { " +
    "    return function() { " +
    "      return \"works\"; } }")

function test() {
  var local = getLazy();
  getLazy = undefined;
  gc();
  print(local());
}

// CHECK-LABEL: main
print("main");
// CHECK-NEXT: works
test();
