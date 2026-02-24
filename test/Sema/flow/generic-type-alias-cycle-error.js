/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -ferror-limit=0 -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

type A<T> = B<T> | null;
type B<T> = A<T> | null;

type C = A<number>;

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}generic-type-alias-cycle-error.js:10:13: error: ft: type contains a circular reference to itself
// CHECK-NEXT:type A<T> = B<T> | null;
// CHECK-NEXT:            ^~~~
// CHECK-NEXT:Emitted 1 errors. exiting.
