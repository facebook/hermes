/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -ferror-limit=0 -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

function foo<T, U>(): void {}

foo<number, number, number>();
foo<number>();

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}generic-function-error.js:12:4: error: type argument mismatch, expected 2, found 3
// CHECK-NEXT:foo<number, number, number>();
// CHECK-NEXT:   ^~~~~~~~~~~~~~~~~~~~~~~~
// CHECK-NEXT:{{.*}}generic-function-error.js:13:4: error: type argument mismatch, expected 2, found 1
// CHECK-NEXT:foo<number>();
// CHECK-NEXT:   ^~~~~~~~
// CHECK-NEXT:Emitted 2 errors. exiting.
