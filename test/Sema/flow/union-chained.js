/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes --typed --dump-sema -fno-std-globals %s | %FileCheckOrRegen %s --match-full-lines

type A = B | C;
type B = D | string;
type C = D | string;
type D = number | string;

// Auto-generated content below. Please do not modify manually.

// CHECK:union %t.1 = union string | number

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1

// CHECK:Program Scope %s.1
// CHECK-NEXT:    TypeAlias
// CHECK-NEXT:        Id 'A'
// CHECK-NEXT:        UnionTypeAnnotation
// CHECK-NEXT:            GenericTypeAnnotation
// CHECK-NEXT:                Id 'B'
// CHECK-NEXT:            GenericTypeAnnotation
// CHECK-NEXT:                Id 'C'
// CHECK-NEXT:    TypeAlias
// CHECK-NEXT:        Id 'B'
// CHECK-NEXT:        UnionTypeAnnotation
// CHECK-NEXT:            GenericTypeAnnotation
// CHECK-NEXT:                Id 'D'
// CHECK-NEXT:            StringTypeAnnotation
// CHECK-NEXT:    TypeAlias
// CHECK-NEXT:        Id 'C'
// CHECK-NEXT:        UnionTypeAnnotation
// CHECK-NEXT:            GenericTypeAnnotation
// CHECK-NEXT:                Id 'D'
// CHECK-NEXT:            StringTypeAnnotation
// CHECK-NEXT:    TypeAlias
// CHECK-NEXT:        Id 'D'
// CHECK-NEXT:        UnionTypeAnnotation
// CHECK-NEXT:            NumberTypeAnnotation
// CHECK-NEXT:            StringTypeAnnotation
