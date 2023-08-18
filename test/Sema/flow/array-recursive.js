/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes --typed --dump-sema -fno-std-globals %s | %FileCheckOrRegen %s --match-full-lines

type A = A[];
let a1: A = [];

type B = (B | null)[];
let b1: B = [];

// Auto-generated content below. Please do not modify manually.

// CHECK:array %t.1 = array array %t.1
// CHECK-NEXT:array %t.2 = array union %t.3
// CHECK-NEXT:union %t.3 = union null | array %t.2

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'a1' Let : array %t.1
// CHECK-NEXT:        Decl %d.2 'b1' Let : array %t.2

// CHECK:Program Scope %s.1
// CHECK-NEXT:    TypeAlias
// CHECK-NEXT:        Id 'A'
// CHECK-NEXT:        ArrayTypeAnnotation
// CHECK-NEXT:            GenericTypeAnnotation
// CHECK-NEXT:                Id 'A'
// CHECK-NEXT:    VariableDeclaration
// CHECK-NEXT:        VariableDeclarator
// CHECK-NEXT:            ArrayExpression : array %t.1
// CHECK-NEXT:            Id 'a1' [D:E:%d.1 'a1']
// CHECK-NEXT:    TypeAlias
// CHECK-NEXT:        Id 'B'
// CHECK-NEXT:        ArrayTypeAnnotation
// CHECK-NEXT:            UnionTypeAnnotation
// CHECK-NEXT:                GenericTypeAnnotation
// CHECK-NEXT:                    Id 'B'
// CHECK-NEXT:                NullLiteralTypeAnnotation
// CHECK-NEXT:    VariableDeclaration
// CHECK-NEXT:        VariableDeclarator
// CHECK-NEXT:            ArrayExpression : array %t.2
// CHECK-NEXT:            Id 'b1' [D:E:%d.2 'b1']
