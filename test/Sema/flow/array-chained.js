/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes --typed --dump-sema -fno-std-globals %s | %FileCheckOrRegen %s --match-full-lines

type A = B[];
type B = C[];
type C = D[];
type D = number;
let a: A = [];

// Auto-generated content below. Please do not modify manually.

// CHECK:array %t.1 = array array %t.2
// CHECK-NEXT:array %t.2 = array array %t.3
// CHECK-NEXT:array %t.3 = array number

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'a' Let : array %t.1

// CHECK:Program Scope %s.1
// CHECK-NEXT:    TypeAlias
// CHECK-NEXT:        Id 'A'
// CHECK-NEXT:        ArrayTypeAnnotation
// CHECK-NEXT:            GenericTypeAnnotation
// CHECK-NEXT:                Id 'B'
// CHECK-NEXT:    TypeAlias
// CHECK-NEXT:        Id 'B'
// CHECK-NEXT:        ArrayTypeAnnotation
// CHECK-NEXT:            GenericTypeAnnotation
// CHECK-NEXT:                Id 'C'
// CHECK-NEXT:    TypeAlias
// CHECK-NEXT:        Id 'C'
// CHECK-NEXT:        ArrayTypeAnnotation
// CHECK-NEXT:            GenericTypeAnnotation
// CHECK-NEXT:                Id 'D'
// CHECK-NEXT:    TypeAlias
// CHECK-NEXT:        Id 'D'
// CHECK-NEXT:        NumberTypeAnnotation
// CHECK-NEXT:    VariableDeclaration
// CHECK-NEXT:        VariableDeclarator
// CHECK-NEXT:            ArrayExpression : array %t.1
// CHECK-NEXT:            Id 'a' [D:E:%d.1 'a']
