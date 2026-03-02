/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals --typed --dump-sema %s | %FileCheckOrRegen %s --match-full-lines

type A<T> = [A<number>[] | number];
type B<T> = [A<number>[] | number];
type X = B<number> | A<number>;
var x: A<number>;
var y: B<number>;
var z: A<number> | B<number>;

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%tuple.2 = tuple(%union.3)
// CHECK-NEXT:%array.4 = array(%tuple.2)
// CHECK-NEXT:%union.3 = union(number | %array.4)
// CHECK-NEXT:%object.5 = object({
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'x' Var : %tuple.2
// CHECK-NEXT:            Decl %d.3 'y' Var : %tuple.2
// CHECK-NEXT:            Decl %d.4 'z' Var : %tuple.2
// CHECK-NEXT:            Decl %d.5 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    TypeAlias
// CHECK-NEXT:                        Id 'A'
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                        TupleTypeAnnotation
// CHECK-NEXT:                            UnionTypeAnnotation
// CHECK-NEXT:                                ArrayTypeAnnotation
// CHECK-NEXT:                                    GenericTypeAnnotation
// CHECK-NEXT:                                        Id 'A'
// CHECK-NEXT:                                        TypeParameterInstantiation
// CHECK-NEXT:                                            NumberTypeAnnotation
// CHECK-NEXT:                                NumberTypeAnnotation
// CHECK-NEXT:                    TypeAlias
// CHECK-NEXT:                        Id 'B'
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                        TupleTypeAnnotation
// CHECK-NEXT:                            UnionTypeAnnotation
// CHECK-NEXT:                                ArrayTypeAnnotation
// CHECK-NEXT:                                    GenericTypeAnnotation
// CHECK-NEXT:                                        Id 'A'
// CHECK-NEXT:                                        TypeParameterInstantiation
// CHECK-NEXT:                                            NumberTypeAnnotation
// CHECK-NEXT:                                NumberTypeAnnotation
// CHECK-NEXT:                    TypeAlias
// CHECK-NEXT:                        Id 'X'
// CHECK-NEXT:                        UnionTypeAnnotation
// CHECK-NEXT:                            GenericTypeAnnotation
// CHECK-NEXT:                                Id 'B'
// CHECK-NEXT:                                TypeParameterInstantiation
// CHECK-NEXT:                                    NumberTypeAnnotation
// CHECK-NEXT:                            GenericTypeAnnotation
// CHECK-NEXT:                                Id 'A'
// CHECK-NEXT:                                TypeParameterInstantiation
// CHECK-NEXT:                                    NumberTypeAnnotation
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            Id 'x' [D:E:%d.2 'x']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            Id 'y' [D:E:%d.3 'y']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            Id 'z' [D:E:%d.4 'z']
// CHECK-NEXT:            ObjectExpression : %object.5
