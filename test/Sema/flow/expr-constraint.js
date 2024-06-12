/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals -typed -dump-sema %s | %FileCheckOrRegen %s --match-full-lines

function f1(x: [number, string]) {}
f1([1, "abc"]);
let v1: [number, string] = flag ? [1, "abc"] : [2, "def"];
let v2: [number, string] = flag || [3, "def"];
let v3: [number, string];
v3 = [4, "def"];

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%tuple.2 = tuple(number, string)
// CHECK-NEXT:%function.3 = function(x: %tuple.2): any
// CHECK-NEXT:%union.4 = union(any | %tuple.2)
// CHECK-NEXT:%object.5 = object({
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'flag' UndeclaredGlobalProperty
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.2 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.3 'f1' Var : %function.3
// CHECK-NEXT:            Decl %d.4 'v1' Let : %tuple.2
// CHECK-NEXT:            Decl %d.5 'v2' Let : %tuple.2
// CHECK-NEXT:            Decl %d.6 'v3' Let : %tuple.2
// CHECK-NEXT:            Decl %d.7 'arguments' Var Arguments
// CHECK-NEXT:            hoistedFunction f1
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:                Decl %d.8 'x' Parameter : %tuple.2
// CHECK-NEXT:                Decl %d.9 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.2 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    FunctionDeclaration : %function.3
// CHECK-NEXT:                        Id 'f1' [D:E:%d.3 'f1']
// CHECK-NEXT:                        Id 'x' [D:E:%d.8 'x']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                    ExpressionStatement
// CHECK-NEXT:                        CallExpression : any
// CHECK-NEXT:                            Id 'f1' [D:E:%d.3 'f1'] : %function.3
// CHECK-NEXT:                            ArrayExpression : %tuple.2
// CHECK-NEXT:                                NumericLiteral : number
// CHECK-NEXT:                                StringLiteral : string
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            ConditionalExpression : %tuple.2
// CHECK-NEXT:                                Id 'flag' [D:E:%d.1 'flag'] : any
// CHECK-NEXT:                                ArrayExpression : %tuple.2
// CHECK-NEXT:                                    NumericLiteral : number
// CHECK-NEXT:                                    StringLiteral : string
// CHECK-NEXT:                                ArrayExpression : %tuple.2
// CHECK-NEXT:                                    NumericLiteral : number
// CHECK-NEXT:                                    StringLiteral : string
// CHECK-NEXT:                            Id 'v1' [D:E:%d.4 'v1']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            ImplicitCheckedCast : %tuple.2
// CHECK-NEXT:                                LogicalExpression : %union.4
// CHECK-NEXT:                                    Id 'flag' [D:E:%d.1 'flag'] : any
// CHECK-NEXT:                                    ArrayExpression : %tuple.2
// CHECK-NEXT:                                        NumericLiteral : number
// CHECK-NEXT:                                        StringLiteral : string
// CHECK-NEXT:                            Id 'v2' [D:E:%d.5 'v2']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            Id 'v3' [D:E:%d.6 'v3']
// CHECK-NEXT:                    ExpressionStatement
// CHECK-NEXT:                        AssignmentExpression : %tuple.2
// CHECK-NEXT:                            Id 'v3' [D:E:%d.6 'v3'] : %tuple.2
// CHECK-NEXT:                            ArrayExpression : %tuple.2
// CHECK-NEXT:                                NumericLiteral : number
// CHECK-NEXT:                                StringLiteral : string
// CHECK-NEXT:            ObjectExpression : %object.5
