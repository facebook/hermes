/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes --typed --dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

type RCO<T> = [RC<string>];
type CU<T> = [Foo[] | null];
type CVOI = RCO<string>;
type CV<T> = [CVOI[] | null];
type RC<T> = CV<T> | CU<T>;
type X = RC<number>;

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}regress-generic-union-cycle-error-2.js:11:15: error: ft: undefined type Foo
// CHECK-NEXT:type CU<T> = [Foo[] | null];
// CHECK-NEXT:              ^
// CHECK-NEXT:{{.*}}regress-generic-union-cycle-error-2.js:11:15: error: ft: undefined type Foo
// CHECK-NEXT:type CU<T> = [Foo[] | null];
// CHECK-NEXT:              ^
// CHECK-NEXT:Emitted 2 errors. exiting.
