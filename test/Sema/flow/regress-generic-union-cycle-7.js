/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals --typed --dump-sema %s | %FileCheckOrRegen %s --match-full-lines

type B<T> = number;
type C<T> = B<D>[];
type D = B<D>[] | C<number>;
var d: D;

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%array.2 = array(number)
// CHECK-NEXT:%object.3 = object({
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'd' Var : %array.2
// CHECK-NEXT:            Decl %d.3 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    TypeAlias
// CHECK-NEXT:                        Id 'B'
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                        NumberTypeAnnotation
// CHECK-NEXT:                    TypeAlias
// CHECK-NEXT:                        Id 'C'
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                        ArrayTypeAnnotation
// CHECK-NEXT:                            GenericTypeAnnotation
// CHECK-NEXT:                                Id 'B'
// CHECK-NEXT:                                TypeParameterInstantiation
// CHECK-NEXT:                                    GenericTypeAnnotation
// CHECK-NEXT:                                        Id 'D'
// CHECK-NEXT:                    TypeAlias
// CHECK-NEXT:                        Id 'D'
// CHECK-NEXT:                        UnionTypeAnnotation
// CHECK-NEXT:                            ArrayTypeAnnotation
// CHECK-NEXT:                                GenericTypeAnnotation
// CHECK-NEXT:                                    Id 'B'
// CHECK-NEXT:                                    TypeParameterInstantiation
// CHECK-NEXT:                                        GenericTypeAnnotation
// CHECK-NEXT:                                            Id 'D'
// CHECK-NEXT:                            GenericTypeAnnotation
// CHECK-NEXT:                                Id 'C'
// CHECK-NEXT:                                TypeParameterInstantiation
// CHECK-NEXT:                                    NumberTypeAnnotation
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            Id 'd' [D:E:%d.2 'd']
// CHECK-NEXT:            ObjectExpression : %object.3
