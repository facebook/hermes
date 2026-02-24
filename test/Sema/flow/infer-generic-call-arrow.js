/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals -typed -dump-sema %s | %FileCheckOrRegen %s --match-full-lines

function map<T, U>(x: T[], f: T => U): U[] {
  var result: U[] = [];
  for (var i = 0; i < x.length; ++i) {
    result.push(f(x[i]));
  }
  return result;
}

var arr: number[] = [1, 2, 3];
var r1: number[] = map(arr, elem => elem);
var r2: boolean[] = map(arr, elem => !elem);

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%array.2 = array(number)
// CHECK-NEXT:%array.3 = array(boolean)
// CHECK-NEXT:%function.4 = function(number): number
// CHECK-NEXT:%function.5 = function(x: %array.2, f: %function.4): %array.2
// CHECK-NEXT:%function.6 = function(number): boolean
// CHECK-NEXT:%function.7 = function(x: %array.2, f: %function.6): %array.3
// CHECK-NEXT:%object.8 = object({
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'map' Var
// CHECK-NEXT:            Decl %d.3 'arr' Var : %array.2
// CHECK-NEXT:            Decl %d.4 'r1' Var : %array.2
// CHECK-NEXT:            Decl %d.5 'r2' Var : %array.3
// CHECK-NEXT:            Decl %d.6 'arguments' Var Arguments
// CHECK-NEXT:            Decl %d.7 'map' Var : %function.5
// CHECK-NEXT:            Decl %d.8 'map' Var : %function.7
// CHECK-NEXT:            hoistedFunction map
// CHECK-NEXT:            hoistedFunction map
// CHECK-NEXT:            hoistedFunction map
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:                Decl %d.9 'x' Parameter
// CHECK-NEXT:                Decl %d.10 'f' Parameter
// CHECK-NEXT:                Decl %d.11 'result' Var
// CHECK-NEXT:                Decl %d.12 'i' Var
// CHECK-NEXT:                Decl %d.13 'arguments' Var Arguments
// CHECK-NEXT:                Scope %s.4
// CHECK-NEXT:                    Scope %s.5
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.6
// CHECK-NEXT:                Decl %d.14 'elem' Parameter : number
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.7
// CHECK-NEXT:                Decl %d.15 'elem' Parameter : number
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.8
// CHECK-NEXT:                Decl %d.16 'x' Parameter : %array.2
// CHECK-NEXT:                Decl %d.17 'f' Parameter : %function.4
// CHECK-NEXT:                Decl %d.18 'result' Var : %array.2
// CHECK-NEXT:                Decl %d.19 'i' Var : number
// CHECK-NEXT:                Decl %d.20 'arguments' Var Arguments
// CHECK-NEXT:                Scope %s.9
// CHECK-NEXT:                    Scope %s.10
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.11
// CHECK-NEXT:                Decl %d.21 'x' Parameter : %array.2
// CHECK-NEXT:                Decl %d.22 'f' Parameter : %function.6
// CHECK-NEXT:                Decl %d.23 'result' Var : %array.3
// CHECK-NEXT:                Decl %d.24 'i' Var : number
// CHECK-NEXT:                Decl %d.25 'arguments' Var Arguments
// CHECK-NEXT:                Scope %s.12
// CHECK-NEXT:                    Scope %s.13

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    FunctionDeclaration : %function.5
// CHECK-NEXT:                        Id 'map' [D:E:%d.7 'map']
// CHECK-NEXT:                        Id 'x' [D:E:%d.16 'x']
// CHECK-NEXT:                        Id 'f' [D:E:%d.17 'f']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            VariableDeclaration
// CHECK-NEXT:                                VariableDeclarator
// CHECK-NEXT:                                    ArrayExpression : %array.2
// CHECK-NEXT:                                    Id 'result' [D:E:%d.18 'result']
// CHECK-NEXT:                            ForStatement Scope %s.9
// CHECK-NEXT:                                VariableDeclaration
// CHECK-NEXT:                                    VariableDeclarator
// CHECK-NEXT:                                        NumericLiteral : number
// CHECK-NEXT:                                        Id 'i' [D:E:%d.19 'i']
// CHECK-NEXT:                                BinaryExpression : boolean
// CHECK-NEXT:                                    Id 'i' [D:E:%d.19 'i'] : number
// CHECK-NEXT:                                    MemberExpression : number
// CHECK-NEXT:                                        Id 'x' [D:E:%d.16 'x'] : %array.2
// CHECK-NEXT:                                        Id 'length'
// CHECK-NEXT:                                UpdateExpression : number
// CHECK-NEXT:                                    Id 'i' [D:E:%d.19 'i'] : number
// CHECK-NEXT:                                BlockStatement Scope %s.10
// CHECK-NEXT:                                    ExpressionStatement
// CHECK-NEXT:                                        CallExpression
// CHECK-NEXT:                                            MemberExpression : any
// CHECK-NEXT:                                                SHBuiltin
// CHECK-NEXT:                                                Id '?fastArrayPush'
// CHECK-NEXT:                                            Id 'result' [D:E:%d.18 'result'] : %array.2
// CHECK-NEXT:                                            CallExpression : number
// CHECK-NEXT:                                                Id 'f' [D:E:%d.17 'f'] : %function.4
// CHECK-NEXT:                                                MemberExpression : number
// CHECK-NEXT:                                                    Id 'x' [D:E:%d.16 'x'] : %array.2
// CHECK-NEXT:                                                    Id 'i' [D:E:%d.19 'i'] : number
// CHECK-NEXT:                            ReturnStatement
// CHECK-NEXT:                                Id 'result' [D:E:%d.18 'result'] : %array.2
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                    FunctionDeclaration : %function.7
// CHECK-NEXT:                        Id 'map' [D:E:%d.8 'map']
// CHECK-NEXT:                        Id 'x' [D:E:%d.21 'x']
// CHECK-NEXT:                        Id 'f' [D:E:%d.22 'f']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            VariableDeclaration
// CHECK-NEXT:                                VariableDeclarator
// CHECK-NEXT:                                    ArrayExpression : %array.3
// CHECK-NEXT:                                    Id 'result' [D:E:%d.23 'result']
// CHECK-NEXT:                            ForStatement Scope %s.12
// CHECK-NEXT:                                VariableDeclaration
// CHECK-NEXT:                                    VariableDeclarator
// CHECK-NEXT:                                        NumericLiteral : number
// CHECK-NEXT:                                        Id 'i' [D:E:%d.24 'i']
// CHECK-NEXT:                                BinaryExpression : boolean
// CHECK-NEXT:                                    Id 'i' [D:E:%d.24 'i'] : number
// CHECK-NEXT:                                    MemberExpression : number
// CHECK-NEXT:                                        Id 'x' [D:E:%d.21 'x'] : %array.2
// CHECK-NEXT:                                        Id 'length'
// CHECK-NEXT:                                UpdateExpression : number
// CHECK-NEXT:                                    Id 'i' [D:E:%d.24 'i'] : number
// CHECK-NEXT:                                BlockStatement Scope %s.13
// CHECK-NEXT:                                    ExpressionStatement
// CHECK-NEXT:                                        CallExpression
// CHECK-NEXT:                                            MemberExpression : any
// CHECK-NEXT:                                                SHBuiltin
// CHECK-NEXT:                                                Id '?fastArrayPush'
// CHECK-NEXT:                                            Id 'result' [D:E:%d.23 'result'] : %array.3
// CHECK-NEXT:                                            CallExpression : boolean
// CHECK-NEXT:                                                Id 'f' [D:E:%d.22 'f'] : %function.6
// CHECK-NEXT:                                                MemberExpression : number
// CHECK-NEXT:                                                    Id 'x' [D:E:%d.21 'x'] : %array.2
// CHECK-NEXT:                                                    Id 'i' [D:E:%d.24 'i'] : number
// CHECK-NEXT:                            ReturnStatement
// CHECK-NEXT:                                Id 'result' [D:E:%d.23 'result'] : %array.3
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                    FunctionDeclaration
// CHECK-NEXT:                        Id 'map' [D:E:%d.2 'map']
// CHECK-NEXT:                        Id 'x' [D:E:%d.9 'x']
// CHECK-NEXT:                        Id 'f' [D:E:%d.10 'f']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            VariableDeclaration
// CHECK-NEXT:                                VariableDeclarator
// CHECK-NEXT:                                    ArrayExpression
// CHECK-NEXT:                                    Id 'result' [D:E:%d.11 'result']
// CHECK-NEXT:                            ForStatement Scope %s.4
// CHECK-NEXT:                                VariableDeclaration
// CHECK-NEXT:                                    VariableDeclarator
// CHECK-NEXT:                                        NumericLiteral
// CHECK-NEXT:                                        Id 'i' [D:E:%d.12 'i']
// CHECK-NEXT:                                BinaryExpression
// CHECK-NEXT:                                    Id 'i' [D:E:%d.12 'i']
// CHECK-NEXT:                                    MemberExpression
// CHECK-NEXT:                                        Id 'x' [D:E:%d.9 'x']
// CHECK-NEXT:                                        Id 'length'
// CHECK-NEXT:                                UpdateExpression
// CHECK-NEXT:                                    Id 'i' [D:E:%d.12 'i']
// CHECK-NEXT:                                BlockStatement Scope %s.5
// CHECK-NEXT:                                    ExpressionStatement
// CHECK-NEXT:                                        CallExpression
// CHECK-NEXT:                                            MemberExpression
// CHECK-NEXT:                                                Id 'result' [D:E:%d.11 'result']
// CHECK-NEXT:                                                Id 'push'
// CHECK-NEXT:                                            CallExpression
// CHECK-NEXT:                                                Id 'f' [D:E:%d.10 'f']
// CHECK-NEXT:                                                MemberExpression
// CHECK-NEXT:                                                    Id 'x' [D:E:%d.9 'x']
// CHECK-NEXT:                                                    Id 'i' [D:E:%d.12 'i']
// CHECK-NEXT:                            ReturnStatement
// CHECK-NEXT:                                Id 'result' [D:E:%d.11 'result']
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            ArrayExpression : %array.2
// CHECK-NEXT:                                NumericLiteral : number
// CHECK-NEXT:                                NumericLiteral : number
// CHECK-NEXT:                                NumericLiteral : number
// CHECK-NEXT:                            Id 'arr' [D:E:%d.3 'arr']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            CallExpression : %array.2
// CHECK-NEXT:                                Id 'map' [D:E:%d.7 'map'] : %function.5
// CHECK-NEXT:                                Id 'arr' [D:E:%d.3 'arr'] : %array.2
// CHECK-NEXT:                                ArrowFunctionExpression : %function.4
// CHECK-NEXT:                                    Id 'elem' [D:E:%d.14 'elem']
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        ReturnStatement
// CHECK-NEXT:                                            Id 'elem' [D:E:%d.14 'elem'] : number
// CHECK-NEXT:                            Id 'r1' [D:E:%d.4 'r1']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            CallExpression : %array.3
// CHECK-NEXT:                                Id 'map' [D:E:%d.8 'map'] : %function.7
// CHECK-NEXT:                                Id 'arr' [D:E:%d.3 'arr'] : %array.2
// CHECK-NEXT:                                ArrowFunctionExpression : %function.6
// CHECK-NEXT:                                    Id 'elem' [D:E:%d.15 'elem']
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        ReturnStatement
// CHECK-NEXT:                                            UnaryExpression : boolean
// CHECK-NEXT:                                                Id 'elem' [D:E:%d.15 'elem'] : number
// CHECK-NEXT:                            Id 'r2' [D:E:%d.5 'r2']
// CHECK-NEXT:            ObjectExpression : %object.8
