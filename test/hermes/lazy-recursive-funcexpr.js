/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -lazy -non-strict -target=HBC %s | %FileCheck --match-full-lines %s

// Named function expressions have special scoping rules. `bar` only exists
// in the function, but not in the parent. Ensure we handle that:
var foo = function bar(n) { return n ? n+bar(n-1) : 0; }

// CHECK-LABEL: main
print("main");
// CHECK-NEXT: 6
print(foo(3));
