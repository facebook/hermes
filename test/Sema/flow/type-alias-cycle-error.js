/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror --typed --dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

type A = B;
type B = A;

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}type-alias-cycle-error.js:10:1: error: ft: type contains a circular reference to itself
// CHECK-NEXT:type A = B;
// CHECK-NEXT:^
// CHECK-NEXT:Emitted 1 errors. exiting.
