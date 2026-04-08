/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Werror -typed %s | %FileCheck --match-full-lines %s
// RUN: %shermes -Werror -typed -exec %s | %FileCheck --match-full-lines %s

class Utils {
  constructor() {}

  @Hermes.final
  static identity<T>(x: T): T {
    return x;
  }

  @Hermes.final
  static pair<T, U>(x: T, y: U): T {
    return x;
  }
}

// Test explicit type arguments.
print(Utils.identity<number>(42));
// CHECK: 42
print(Utils.identity<string>("hello"));
// CHECK-NEXT: hello

// Test type inference.
print(Utils.identity(42));
// CHECK-NEXT: 42
print(Utils.identity("hello"));
// CHECK-NEXT: hello

// Test multiple type parameters with explicit args.
print(Utils.pair<number, string>(10, "test"));
// CHECK-NEXT: 10
