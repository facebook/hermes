/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes --typed --dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

type A<T> = A<T> | (void|void)[];
type B = A<void>;

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}regress-generic-union-cycle-error.js:10:13: error: ft: type contains a circular reference to itself
// CHECK-NEXT:type A<T> = A<T> | (void|void)[];
// CHECK-NEXT:            ^~~~
// CHECK-NEXT:Emitted 1 errors. exiting.
