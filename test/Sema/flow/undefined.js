/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals --typed --dump-sema %s | %FileCheckOrRegen %s --match-full-lines

undefined;

// Auto-generated content below. Please do not modify manually.

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'undefined' UndeclaredGlobalProperty

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        Id 'undefined' [D:E:%d.1 'undefined'] : void
