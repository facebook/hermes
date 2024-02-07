/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -O0 -typed -exec %s | %FileCheck %s --match-full-lines
// RUN: %shermes -typed -exec %s | %FileCheck %s --match-full-lines

function CHECKED_CAST<T>(x: mixed): T {
  'inline';
  return ((x: any): T);
}

function foo(x: number | string): void {
  var y: number = CHECKED_CAST<number>(x);
  print(x, y);
}
foo(12)
// CHECK: 12 12
