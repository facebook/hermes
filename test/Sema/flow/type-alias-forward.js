/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes --typed --dump-sema -fno-std-globals %s | %FileCheckOrRegen %s --match-full-lines

type A = B;
type B = number;

// Auto-generated content below. Please do not modify manually.

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1

// CHECK:Program Scope %s.1
// CHECK-NEXT:    TypeAlias
// CHECK-NEXT:        Id 'A'
// CHECK-NEXT:        GenericTypeAnnotation
// CHECK-NEXT:            Id 'B'
// CHECK-NEXT:    TypeAlias
// CHECK-NEXT:        Id 'B'
// CHECK-NEXT:        NumberTypeAnnotation
