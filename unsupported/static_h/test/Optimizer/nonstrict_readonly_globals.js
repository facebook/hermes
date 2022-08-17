/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -non-strict -O -target=HBC %s | %FileCheck %s

// In non-strict mode, writes to read-only global variables are ignored.  We
// treated them as always succeeding or throwing.

// CHECK-LABEL: hello
print("hello");

function foo() {
  NaN = true
  print (typeof(NaN));
}
// CHECK-NEXT: number
foo();

Infinity = true
// CHECK-NEXT: number
print (typeof(Infinity))
