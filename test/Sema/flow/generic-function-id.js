/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals -typed -dump-sema %s | %FileCheckOrRegen %s --match-full-lines

// Make sure two specializations are generated for number and string.
type A = number;
function id<T>(x: T): T {
  // Test using type aliases from outside the function and Decl type assignment
  // inside the function.
  let y: A = 3;
  return x;
}

id<number>(1);
id<string>('a');

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%function.2 = function(x: number): number
// CHECK-NEXT:%function.3 = function(x: string): string

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
// CHECK-NEXT:            hoistedFunction id
// CHECK-NEXT:            hoistedFunction id
// CHECK-NEXT:            hoistedFunction id
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:                Decl %d.6 'x' Parameter
// CHECK-NEXT:                Decl %d.7 'y' Let
// CHECK-NEXT:                Decl %d.8 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.4
// CHECK-NEXT:                Decl %d.9 'x' Parameter : number
// CHECK-NEXT:                Decl %d.10 'y' Let : number
// CHECK-NEXT:                Decl %d.11 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.5
// CHECK-NEXT:                Decl %d.12 'x' Parameter : string
// CHECK-NEXT:                Decl %d.13 'y' Let : number
// CHECK-NEXT:                Decl %d.14 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    TypeAlias
// CHECK-NEXT:                        Id 'A'
// CHECK-NEXT:                        NumberTypeAnnotation
// CHECK-NEXT:                    FunctionDeclaration : %function.2
// CHECK-NEXT:                        Id 'id' [D:E:%d.4 'id']
// CHECK-NEXT:                        Id 'x' [D:E:%d.9 'x']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            VariableDeclaration
// CHECK-NEXT:                                VariableDeclarator
// CHECK-NEXT:                                    NumericLiteral : number
// CHECK-NEXT:                                    Id 'y' [D:E:%d.10 'y']
// CHECK-NEXT:                            ReturnStatement
// CHECK-NEXT:                                Id 'x' [D:E:%d.9 'x'] : number
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                    FunctionDeclaration : %function.3
// CHECK-NEXT:                        Id 'id' [D:E:%d.5 'id']
// CHECK-NEXT:                        Id 'x' [D:E:%d.12 'x']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            VariableDeclaration
// CHECK-NEXT:                                VariableDeclarator
// CHECK-NEXT:                                    NumericLiteral : number
// CHECK-NEXT:                                    Id 'y' [D:E:%d.13 'y']
// CHECK-NEXT:                            ReturnStatement
// CHECK-NEXT:                                Id 'x' [D:E:%d.12 'x'] : string
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                    FunctionDeclaration
// CHECK-NEXT:                        Id 'id' [D:E:%d.2 'id']
// CHECK-NEXT:                        Id 'x' [D:E:%d.6 'x']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            VariableDeclaration
// CHECK-NEXT:                                VariableDeclarator
// CHECK-NEXT:                                    NumericLiteral
// CHECK-NEXT:                                    Id 'y' [D:E:%d.7 'y']
// CHECK-NEXT:                            ReturnStatement
// CHECK-NEXT:                                Id 'x' [D:E:%d.6 'x']
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
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
// CHECK-NEXT:            ObjectExpression
