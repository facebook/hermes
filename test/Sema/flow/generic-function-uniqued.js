/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals -typed -dump-sema %s | %FileCheckOrRegen %s --match-full-lines

// Ensure there's only one specialization per type argument.
function id<T>(x: T): T {
  return x;
}

id<number>(1);
id<number>(1);
id<string>('a');
id<string>('a');
id<number | string>(1);
id<number | string>('a');

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%function.2 = function(x: number): number
// CHECK-NEXT:%function.3 = function(x: string): string
// CHECK-NEXT:%union.4 = union(string | number)
// CHECK-NEXT:%function.5 = function(x: %union.4): %union.4

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'id' ScopedFunction
// CHECK-NEXT:            Decl %d.3 'arguments' Var Arguments
// CHECK-NEXT:            Decl %d.4 'id' ScopedFunction : %function.2
// CHECK-NEXT:            Decl %d.5 'id' ScopedFunction : %function.3
// CHECK-NEXT:            Decl %d.6 'id' ScopedFunction : %function.5
// CHECK-NEXT:            hoistedFunction id
// CHECK-NEXT:            hoistedFunction id
// CHECK-NEXT:            hoistedFunction id
// CHECK-NEXT:            hoistedFunction id
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:                Decl %d.7 'x' Parameter
// CHECK-NEXT:                Decl %d.8 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.4
// CHECK-NEXT:                Decl %d.9 'x' Parameter : number
// CHECK-NEXT:                Decl %d.10 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.5
// CHECK-NEXT:                Decl %d.11 'x' Parameter : string
// CHECK-NEXT:                Decl %d.12 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.6
// CHECK-NEXT:                Decl %d.13 'x' Parameter : %union.4
// CHECK-NEXT:                Decl %d.14 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    FunctionDeclaration : %function.2
// CHECK-NEXT:                        Id 'id' [D:E:%d.4 'id']
// CHECK-NEXT:                        Id 'x' [D:E:%d.9 'x']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            ReturnStatement
// CHECK-NEXT:                                Id 'x' [D:E:%d.9 'x'] : number
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                    FunctionDeclaration : %function.3
// CHECK-NEXT:                        Id 'id' [D:E:%d.5 'id']
// CHECK-NEXT:                        Id 'x' [D:E:%d.11 'x']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            ReturnStatement
// CHECK-NEXT:                                Id 'x' [D:E:%d.11 'x'] : string
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                    FunctionDeclaration : %function.5
// CHECK-NEXT:                        Id 'id' [D:E:%d.6 'id']
// CHECK-NEXT:                        Id 'x' [D:E:%d.13 'x']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            ReturnStatement
// CHECK-NEXT:                                Id 'x' [D:E:%d.13 'x'] : %union.4
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                    FunctionDeclaration
// CHECK-NEXT:                        Id 'id' [D:E:%d.2 'id']
// CHECK-NEXT:                        Id 'x' [D:E:%d.7 'x']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            ReturnStatement
// CHECK-NEXT:                                Id 'x' [D:E:%d.7 'x']
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                    ExpressionStatement
// CHECK-NEXT:                        CallExpression : number
// CHECK-NEXT:                            Id 'id' [D:E:%d.4 'id'] : %function.2
// CHECK-NEXT:                            TypeParameterInstantiation
// CHECK-NEXT:                                NumberTypeAnnotation
// CHECK-NEXT:                            NumericLiteral : number
// CHECK-NEXT:                    ExpressionStatement
// CHECK-NEXT:                        CallExpression : number
// CHECK-NEXT:                            Id 'id' [D:E:%d.4 'id'] : %function.2
// CHECK-NEXT:                            TypeParameterInstantiation
// CHECK-NEXT:                                NumberTypeAnnotation
// CHECK-NEXT:                            NumericLiteral : number
// CHECK-NEXT:                    ExpressionStatement
// CHECK-NEXT:                        CallExpression : string
// CHECK-NEXT:                            Id 'id' [D:E:%d.5 'id'] : %function.3
// CHECK-NEXT:                            TypeParameterInstantiation
// CHECK-NEXT:                                StringTypeAnnotation
// CHECK-NEXT:                            StringLiteral : string
// CHECK-NEXT:                    ExpressionStatement
// CHECK-NEXT:                        CallExpression : string
// CHECK-NEXT:                            Id 'id' [D:E:%d.5 'id'] : %function.3
// CHECK-NEXT:                            TypeParameterInstantiation
// CHECK-NEXT:                                StringTypeAnnotation
// CHECK-NEXT:                            StringLiteral : string
// CHECK-NEXT:                    ExpressionStatement
// CHECK-NEXT:                        CallExpression : %union.4
// CHECK-NEXT:                            Id 'id' [D:E:%d.6 'id'] : %function.5
// CHECK-NEXT:                            TypeParameterInstantiation
// CHECK-NEXT:                                UnionTypeAnnotation
// CHECK-NEXT:                                    NumberTypeAnnotation
// CHECK-NEXT:                                    StringTypeAnnotation
// CHECK-NEXT:                            NumericLiteral : number
// CHECK-NEXT:                    ExpressionStatement
// CHECK-NEXT:                        CallExpression : %union.4
// CHECK-NEXT:                            Id 'id' [D:E:%d.6 'id'] : %function.5
// CHECK-NEXT:                            TypeParameterInstantiation
// CHECK-NEXT:                                UnionTypeAnnotation
// CHECK-NEXT:                                    NumberTypeAnnotation
// CHECK-NEXT:                                    StringTypeAnnotation
// CHECK-NEXT:                            StringLiteral : string
// CHECK-NEXT:            ObjectExpression
