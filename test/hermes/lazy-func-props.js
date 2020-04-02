/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -lazy -non-strict -target=HBC %s | %FileCheck --match-full-lines %s

// This test verifies that lazy functions still have expected .length and .name
function foo(bar,baz,etc) {
  // Functions are only lazily compiled if over a certain size, so...
  // Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod
  // tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim
  // veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea
  // commodo consequat. Duis aute irure dolor in reprehenderit in voluptate
  // velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat
  // cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id
  // est laborum.
}

// CHECK-LABEL: main
print("main");
// CHECK-NEXT: 3
print(foo.length);
// CHECK-NEXT: foo
print(foo.name);
foo();
// CHECK-NEXT: 3
print(foo.length);
// CHECK-NEXT: foo
print(foo.name);
