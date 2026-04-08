/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -ferror-limit=0 -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

// Test non-generic builtin: charAt expects number, got string
function testCharAt(s: string): string {
  return s.charAt('b');
}

// Test unknown array method
function testMap(arr: number[]): number[] {
  return arr.map((n: number): string => 'x', undefined);
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}builtin-argument-error.js:12:19: error: ft: function parameter 'pos' type mismatch
// CHECK-NEXT:  return s.charAt('b');
// CHECK-NEXT:                  ^~~
// CHECK-NEXT:{{.*}}builtin-argument-error.js:17:3: error: ft: return value incompatible with return type: cannot return class Array<string> as class Array<number>
// CHECK-NEXT:  return arr.map((n: number): string => 'x', undefined);
// CHECK-NEXT:  ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// CHECK-NEXT:{{.*}}builtin-argument-error.js:17:10: error: ft: function expects at most 1 arguments, but 2 supplied
// CHECK-NEXT:  return arr.map((n: number): string => 'x', undefined);
// CHECK-NEXT:         ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// CHECK-NEXT:Emitted 3 errors. exiting.
