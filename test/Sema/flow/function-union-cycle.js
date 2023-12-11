/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror --typed --dump-sema -fno-std-globals %s | %FileCheckOrRegen %s --match-full-lines

type A = (a:number) => B;
type B = (a:string) => C;
type C = (a:boolean) => A;

type A1 = (a:number) => B1;
type B1 = (a:string) => C1;
type C1 = (a:boolean) => B1;

// These two types shouldn't be equal and the union should keep 2 arms.
type U = A | A1;

let u: U;

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%function.2 = function(a: number): %function.3
// CHECK-NEXT:%function.3 = function(a: string): %function.4
// CHECK-NEXT:%function.4 = function(a: boolean): %function.2
// CHECK-NEXT:%function.5 = function(a: number): %function.6
// CHECK-NEXT:%function.6 = function(a: string): %function.7
// CHECK-NEXT:%function.7 = function(a: boolean): %function.6
// CHECK-NEXT:%union.8 = union(%function.2 | %function.5)

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'u' Let : %union.8
// CHECK-NEXT:            Decl %d.3 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    TypeAlias
// CHECK-NEXT:                        Id 'A'
// CHECK-NEXT:                        FunctionTypeAnnotation
// CHECK-NEXT:                            FunctionTypeParam
// CHECK-NEXT:                                Id 'a'
// CHECK-NEXT:                                NumberTypeAnnotation
// CHECK-NEXT:                            GenericTypeAnnotation
// CHECK-NEXT:                                Id 'B'
// CHECK-NEXT:                    TypeAlias
// CHECK-NEXT:                        Id 'B'
// CHECK-NEXT:                        FunctionTypeAnnotation
// CHECK-NEXT:                            FunctionTypeParam
// CHECK-NEXT:                                Id 'a'
// CHECK-NEXT:                                StringTypeAnnotation
// CHECK-NEXT:                            GenericTypeAnnotation
// CHECK-NEXT:                                Id 'C'
// CHECK-NEXT:                    TypeAlias
// CHECK-NEXT:                        Id 'C'
// CHECK-NEXT:                        FunctionTypeAnnotation
// CHECK-NEXT:                            FunctionTypeParam
// CHECK-NEXT:                                Id 'a'
// CHECK-NEXT:                                BooleanTypeAnnotation
// CHECK-NEXT:                            GenericTypeAnnotation
// CHECK-NEXT:                                Id 'A'
// CHECK-NEXT:                    TypeAlias
// CHECK-NEXT:                        Id 'A1'
// CHECK-NEXT:                        FunctionTypeAnnotation
// CHECK-NEXT:                            FunctionTypeParam
// CHECK-NEXT:                                Id 'a'
// CHECK-NEXT:                                NumberTypeAnnotation
// CHECK-NEXT:                            GenericTypeAnnotation
// CHECK-NEXT:                                Id 'B1'
// CHECK-NEXT:                    TypeAlias
// CHECK-NEXT:                        Id 'B1'
// CHECK-NEXT:                        FunctionTypeAnnotation
// CHECK-NEXT:                            FunctionTypeParam
// CHECK-NEXT:                                Id 'a'
// CHECK-NEXT:                                StringTypeAnnotation
// CHECK-NEXT:                            GenericTypeAnnotation
// CHECK-NEXT:                                Id 'C1'
// CHECK-NEXT:                    TypeAlias
// CHECK-NEXT:                        Id 'C1'
// CHECK-NEXT:                        FunctionTypeAnnotation
// CHECK-NEXT:                            FunctionTypeParam
// CHECK-NEXT:                                Id 'a'
// CHECK-NEXT:                                BooleanTypeAnnotation
// CHECK-NEXT:                            GenericTypeAnnotation
// CHECK-NEXT:                                Id 'B1'
// CHECK-NEXT:                    TypeAlias
// CHECK-NEXT:                        Id 'U'
// CHECK-NEXT:                        UnionTypeAnnotation
// CHECK-NEXT:                            GenericTypeAnnotation
// CHECK-NEXT:                                Id 'A'
// CHECK-NEXT:                            GenericTypeAnnotation
// CHECK-NEXT:                                Id 'A1'
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            Id 'u' [D:E:%d.2 'u']
// CHECK-NEXT:            ObjectExpression
