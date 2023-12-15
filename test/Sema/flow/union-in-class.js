/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes --typed --dump-sema -fno-std-globals %s | %FileCheckOrRegen %s --match-full-lines

type A = number | string;
class C {
  // Need to ensure that A is checked for looping arms and canonicalized
  // before attempting to complete the unions inside each class.
  x: A | null;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%union.2 = union(string | number)
// CHECK-NEXT:%class.3 = class(C {
// CHECK-NEXT:  %homeObject: %class.4
// CHECK-NEXT:  x: %union.5
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.6 = class_constructor(%class.3)
// CHECK-NEXT:%union.5 = union(null | string | number)
// CHECK-NEXT:%class.4 = class( {
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'C' Class : %class_constructor.6
// CHECK-NEXT:            Decl %d.3 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    TypeAlias
// CHECK-NEXT:                        Id 'A'
// CHECK-NEXT:                        UnionTypeAnnotation
// CHECK-NEXT:                            NumberTypeAnnotation
// CHECK-NEXT:                            StringTypeAnnotation
// CHECK-NEXT:                    ClassDeclaration
// CHECK-NEXT:                        Id 'C' [D:E:%d.2 'C']
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            ClassProperty : %union.5
// CHECK-NEXT:                                Id 'x'
// CHECK-NEXT:            ObjectExpression
