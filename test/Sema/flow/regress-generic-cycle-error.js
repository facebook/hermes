/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes --typed --dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

type A<T> = B<T>[];
type B<T> = A<B<T>>;
type C = A<number>;

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}regress-generic-cycle-error.js:11:13: error: ft: type contains a circular reference to itself
// CHECK-NEXT:type B<T> = A<B<T>>;
// CHECK-NEXT:            ^~~~~~~
// CHECK-NEXT:Emitted 1 errors. exiting.
