/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -debug-only=codeblock -lazy -non-strict -target=HBC -O0 %s 2>&1 | %FileCheck --match-full-lines %s
// REQUIRES: debug_options, !fbcode

function foo() {
  function bar() {
    print("bar");
  }
  function unused() {
    print("unused");
  }

  print("foo");
  bar();
  bar();
}


// CHECK-LABEL: main
print("main");
// CHECK-NEXT: Compiling lazy function foo
// CHECK-NEXT: foo
// CHECK-NEXT: Compiling lazy function bar
// CHECK-NEXT: bar
// CHECK-NEXT: bar
foo();
// CHECK-NEXT: end
print("end");
