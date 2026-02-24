/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals -typed -dump-sema %s | %FileCheckOrRegen %s --match-full-lines

function head<T>(x: T[]): T {
  return x[0];
}
let v1 = head([1, 2, 3]);
let v2 = head(['a', 'b', 'c']);
let b: boolean[] = [true, true, false];
let v3 = head(b);

function first<T, U>(x: [T, U]): T {
  return x[0];
}
let v4 = first([1, 2]);
let c: [boolean, boolean] = [true, true];
let v5 = first(c);

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%array.2 = array(boolean)
// CHECK-NEXT:%tuple.3 = tuple(boolean, boolean)
// CHECK-NEXT:%array.4 = array(number)
// CHECK-NEXT:%function.5 = function(x: %array.4): number
// CHECK-NEXT:%array.6 = array(string)
// CHECK-NEXT:%function.7 = function(x: %array.6): string
// CHECK-NEXT:%function.8 = function(x: %array.2): boolean
// CHECK-NEXT:%tuple.9 = tuple(number, number)
// CHECK-NEXT:%function.10 = function(x: %tuple.9): number
// CHECK-NEXT:%function.11 = function(x: %tuple.3): boolean
// CHECK-NEXT:%object.12 = object({
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'head' Var
// CHECK-NEXT:            Decl %d.3 'v1' Let : number
// CHECK-NEXT:            Decl %d.4 'v2' Let : string
// CHECK-NEXT:            Decl %d.5 'b' Let : %array.2
// CHECK-NEXT:            Decl %d.6 'v3' Let : boolean
// CHECK-NEXT:            Decl %d.7 'first' Var
// CHECK-NEXT:            Decl %d.8 'v4' Let : number
// CHECK-NEXT:            Decl %d.9 'c' Let : %tuple.3
// CHECK-NEXT:            Decl %d.10 'v5' Let : boolean
// CHECK-NEXT:            Decl %d.11 'arguments' Var Arguments
// CHECK-NEXT:            Decl %d.12 'head' Var : %function.5
// CHECK-NEXT:            Decl %d.13 'head' Var : %function.7
// CHECK-NEXT:            Decl %d.14 'head' Var : %function.8
// CHECK-NEXT:            Decl %d.15 'first' Var : %function.10
// CHECK-NEXT:            Decl %d.16 'first' Var : %function.11
// CHECK-NEXT:            hoistedFunction head
// CHECK-NEXT:            hoistedFunction first
// CHECK-NEXT:            hoistedFunction head
// CHECK-NEXT:            hoistedFunction head
// CHECK-NEXT:            hoistedFunction head
// CHECK-NEXT:            hoistedFunction first
// CHECK-NEXT:            hoistedFunction first
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:                Decl %d.17 'x' Parameter
// CHECK-NEXT:                Decl %d.18 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.4
// CHECK-NEXT:                Decl %d.19 'x' Parameter
// CHECK-NEXT:                Decl %d.20 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.5
// CHECK-NEXT:                Decl %d.21 'x' Parameter : %array.4
// CHECK-NEXT:                Decl %d.22 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.6
// CHECK-NEXT:                Decl %d.23 'x' Parameter : %array.6
// CHECK-NEXT:                Decl %d.24 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.7
// CHECK-NEXT:                Decl %d.25 'x' Parameter : %array.2
// CHECK-NEXT:                Decl %d.26 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.8
// CHECK-NEXT:                Decl %d.27 'x' Parameter : %tuple.9
// CHECK-NEXT:                Decl %d.28 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.9
// CHECK-NEXT:                Decl %d.29 'x' Parameter : %tuple.3
// CHECK-NEXT:                Decl %d.30 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    FunctionDeclaration : %function.5
// CHECK-NEXT:                        Id 'head' [D:E:%d.12 'head']
// CHECK-NEXT:                        Id 'x' [D:E:%d.21 'x']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            ReturnStatement
// CHECK-NEXT:                                MemberExpression : number
// CHECK-NEXT:                                    Id 'x' [D:E:%d.21 'x'] : %array.4
// CHECK-NEXT:                                    NumericLiteral : number
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                    FunctionDeclaration : %function.7
// CHECK-NEXT:                        Id 'head' [D:E:%d.13 'head']
// CHECK-NEXT:                        Id 'x' [D:E:%d.23 'x']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            ReturnStatement
// CHECK-NEXT:                                MemberExpression : string
// CHECK-NEXT:                                    Id 'x' [D:E:%d.23 'x'] : %array.6
// CHECK-NEXT:                                    NumericLiteral : number
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                    FunctionDeclaration : %function.8
// CHECK-NEXT:                        Id 'head' [D:E:%d.14 'head']
// CHECK-NEXT:                        Id 'x' [D:E:%d.25 'x']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            ReturnStatement
// CHECK-NEXT:                                MemberExpression : boolean
// CHECK-NEXT:                                    Id 'x' [D:E:%d.25 'x'] : %array.2
// CHECK-NEXT:                                    NumericLiteral : number
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                    FunctionDeclaration
// CHECK-NEXT:                        Id 'head' [D:E:%d.2 'head']
// CHECK-NEXT:                        Id 'x' [D:E:%d.17 'x']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            ReturnStatement
// CHECK-NEXT:                                MemberExpression
// CHECK-NEXT:                                    Id 'x' [D:E:%d.17 'x']
// CHECK-NEXT:                                    NumericLiteral
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            CallExpression : number
// CHECK-NEXT:                                Id 'head' [D:E:%d.12 'head'] : %function.5
// CHECK-NEXT:                                ArrayExpression : %array.4
// CHECK-NEXT:                                    NumericLiteral : number
// CHECK-NEXT:                                    NumericLiteral : number
// CHECK-NEXT:                                    NumericLiteral : number
// CHECK-NEXT:                            Id 'v1' [D:E:%d.3 'v1']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            CallExpression : string
// CHECK-NEXT:                                Id 'head' [D:E:%d.13 'head'] : %function.7
// CHECK-NEXT:                                ArrayExpression : %array.6
// CHECK-NEXT:                                    StringLiteral : string
// CHECK-NEXT:                                    StringLiteral : string
// CHECK-NEXT:                                    StringLiteral : string
// CHECK-NEXT:                            Id 'v2' [D:E:%d.4 'v2']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            ArrayExpression : %array.2
// CHECK-NEXT:                                BooleanLiteral : boolean
// CHECK-NEXT:                                BooleanLiteral : boolean
// CHECK-NEXT:                                BooleanLiteral : boolean
// CHECK-NEXT:                            Id 'b' [D:E:%d.5 'b']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            CallExpression : boolean
// CHECK-NEXT:                                Id 'head' [D:E:%d.14 'head'] : %function.8
// CHECK-NEXT:                                Id 'b' [D:E:%d.5 'b'] : %array.2
// CHECK-NEXT:                            Id 'v3' [D:E:%d.6 'v3']
// CHECK-NEXT:                    FunctionDeclaration : %function.10
// CHECK-NEXT:                        Id 'first' [D:E:%d.15 'first']
// CHECK-NEXT:                        Id 'x' [D:E:%d.27 'x']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            ReturnStatement
// CHECK-NEXT:                                MemberExpression : number
// CHECK-NEXT:                                    Id 'x' [D:E:%d.27 'x'] : %tuple.9
// CHECK-NEXT:                                    NumericLiteral : number
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                    FunctionDeclaration : %function.11
// CHECK-NEXT:                        Id 'first' [D:E:%d.16 'first']
// CHECK-NEXT:                        Id 'x' [D:E:%d.29 'x']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            ReturnStatement
// CHECK-NEXT:                                MemberExpression : boolean
// CHECK-NEXT:                                    Id 'x' [D:E:%d.29 'x'] : %tuple.3
// CHECK-NEXT:                                    NumericLiteral : number
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                    FunctionDeclaration
// CHECK-NEXT:                        Id 'first' [D:E:%d.7 'first']
// CHECK-NEXT:                        Id 'x' [D:E:%d.19 'x']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            ReturnStatement
// CHECK-NEXT:                                MemberExpression
// CHECK-NEXT:                                    Id 'x' [D:E:%d.19 'x']
// CHECK-NEXT:                                    NumericLiteral
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            CallExpression : number
// CHECK-NEXT:                                Id 'first' [D:E:%d.15 'first'] : %function.10
// CHECK-NEXT:                                ArrayExpression : %tuple.9
// CHECK-NEXT:                                    NumericLiteral : number
// CHECK-NEXT:                                    NumericLiteral : number
// CHECK-NEXT:                            Id 'v4' [D:E:%d.8 'v4']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            ArrayExpression : %tuple.3
// CHECK-NEXT:                                BooleanLiteral : boolean
// CHECK-NEXT:                                BooleanLiteral : boolean
// CHECK-NEXT:                            Id 'c' [D:E:%d.9 'c']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            CallExpression : boolean
// CHECK-NEXT:                                Id 'first' [D:E:%d.16 'first'] : %function.11
// CHECK-NEXT:                                Id 'c' [D:E:%d.9 'c'] : %tuple.3
// CHECK-NEXT:                            Id 'v5' [D:E:%d.10 'v5']
// CHECK-NEXT:            ObjectExpression : %object.12
