/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -O0 -typed -exec %s | %FileCheck %s --match-full-lines
// RUN: %shermes -typed -exec %s | %FileCheck %s --match-full-lines

function id<T>(x: T): T {
  return x;
}

print(id<number>(10));
// CHECK: 10
print(id<string>('abc'));
// CHECK-NEXT: abc
