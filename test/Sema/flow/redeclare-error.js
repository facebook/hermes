/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes --typed --dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

type A = number;
class A {}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}redeclare-error.js:11:7: error: ft: type A already declared in this scope
// CHECK-NEXT:class A {}
// CHECK-NEXT:      ^
// CHECK-NEXT:{{.*}}redeclare-error.js:10:1: note: ft: previous declaration of A
// CHECK-NEXT:type A = number;
// CHECK-NEXT:^~~~~~~~~~~~~~~~
// CHECK-NEXT:Emitted 1 errors. exiting.
