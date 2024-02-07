/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -O0 -typed -exec %s | %FileCheck %s --match-full-lines
// RUN: %shermes -typed -exec %s | %FileCheck %s --match-full-lines

function add<T>(x: T, y: T): T {
  return x + y;
}

print(add<number>(1, 2));
// CHECK: 3
print(add<string>('abc', 'def'));
// CHECK-NEXT: abcdef

function call<F, A, R>(f: F, a: A): R {
  return f(a);
}

var f = function(x: number): string { return 'abc' + String(x); };
var s: string = call<number => string, number, string>(f, 42);
print(s);
// CHECK-NEXT: abc42
