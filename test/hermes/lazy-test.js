/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -lazy -debug-only=codeblock -non-strict -target=HBC %s 2>&1 | %FileCheck --match-full-lines %s
// REQUIRES: debug_options, !fbcode

function foo() {
  function bar() {
    print("bar");
    /* Some text to pad out the function so that it won't be eagerly compiled
     * for being too short. Lorem ipsum dolor sit amet, consectetur adipiscing
     * elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
     */
  }
  function unused() {
    print("unused");
    /* Some text to pad out the function so that it won't be eagerly compiled
     * for being too short. Lorem ipsum dolor sit amet, consectetur adipiscing
     * elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
     */
  }

  // This function is so small that it's not worth postponing compilation.
  function tiny() {
    print("tiny");
  }

  print("foo");
  bar();
  bar();
  tiny();
}


// CHECK-LABEL: main
print("main");
// CHECK-NEXT: Compiling lazy function foo
// CHECK-NEXT: foo
// CHECK-NEXT: Compiling lazy function bar
// CHECK-NEXT: bar
// CHECK-NEXT: bar
// CHECK-NEXT: tiny
foo();
// CHECK-NEXT: end
print("end");
