/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -lazy -non-strict -target=HBC %s | %FileCheck --match-full-lines %s

// This test verifies that lazy functions still have expected .length and .name
function foo(bar,baz,etc) { }

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
