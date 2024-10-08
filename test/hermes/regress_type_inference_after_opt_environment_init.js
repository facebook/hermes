/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s
// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s

// We previously incorrectly ran TypeInference after OptEnvironmentInit had
// deleted stores of undefined to the environment. This caused us to infer a
// narrower type for variables than they actually had, resulting in incorrect
// codegen.
// This test checks that we do not infer types after OptEnvironmentInit has run.

function foo(sink) {
  // Hoisted store of undefined to x is observable.
  function bar() {
    // x here must have type number|undefined
    print(x+1);
  }
  sink(bar);
  var x = 0;
}
foo((fn) => fn());

//CHECK: NaN
