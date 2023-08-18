/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes --typed --dump-sema -fno-std-globals %s | %FileCheckOrRegen %s --match-full-lines

type A = B[] | B[];
type B = A;

type X = Y[];
type Y = X | X;

type U = A | X;

// These two types should be the same.
// The two loops have two different paths to reach them,
// but they should be uniqued properly.
let a: A;
let u: U;

// Auto-generated content below. Please do not modify manually.

// CHECK:array %t.1 = array array %t.1

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'a' Let : array %t.1
// CHECK-NEXT:        Decl %d.2 'u' Let : array %t.1

// CHECK:Program Scope %s.1
// CHECK-NEXT:    TypeAlias
// CHECK-NEXT:        Id 'A'
// CHECK-NEXT:        UnionTypeAnnotation
// CHECK-NEXT:            ArrayTypeAnnotation
// CHECK-NEXT:                GenericTypeAnnotation
// CHECK-NEXT:                    Id 'B'
// CHECK-NEXT:            ArrayTypeAnnotation
// CHECK-NEXT:                GenericTypeAnnotation
// CHECK-NEXT:                    Id 'B'
// CHECK-NEXT:    TypeAlias
// CHECK-NEXT:        Id 'B'
// CHECK-NEXT:        GenericTypeAnnotation
// CHECK-NEXT:            Id 'A'
// CHECK-NEXT:    TypeAlias
// CHECK-NEXT:        Id 'X'
// CHECK-NEXT:        ArrayTypeAnnotation
// CHECK-NEXT:            GenericTypeAnnotation
// CHECK-NEXT:                Id 'Y'
// CHECK-NEXT:    TypeAlias
// CHECK-NEXT:        Id 'Y'
// CHECK-NEXT:        UnionTypeAnnotation
// CHECK-NEXT:            GenericTypeAnnotation
// CHECK-NEXT:                Id 'X'
// CHECK-NEXT:            GenericTypeAnnotation
// CHECK-NEXT:                Id 'X'
// CHECK-NEXT:    TypeAlias
// CHECK-NEXT:        Id 'U'
// CHECK-NEXT:        UnionTypeAnnotation
// CHECK-NEXT:            GenericTypeAnnotation
// CHECK-NEXT:                Id 'A'
// CHECK-NEXT:            GenericTypeAnnotation
// CHECK-NEXT:                Id 'X'
// CHECK-NEXT:    VariableDeclaration
// CHECK-NEXT:        VariableDeclarator
// CHECK-NEXT:            Id 'a' [D:E:%d.1 'a']
// CHECK-NEXT:    VariableDeclaration
// CHECK-NEXT:        VariableDeclarator
// CHECK-NEXT:            Id 'u' [D:E:%d.2 'u']
