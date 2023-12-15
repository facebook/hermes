/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals --typed --dump-sema %s | %FileCheckOrRegen %s --match-full-lines

'use strict';

let inner: [number, string] = [2, 'asdf'];
let outer: [number, bool, [number, string]] = [1, true, inner];
let [x, y, [a, b]] = outer;
let [t, u, v] = outer;
let i: number;
let j: string;
[i, j] = inner;
let anyVar: any = inner;
let [anyNumber, anyString]: [number, string] = anyVar; // implicit checked cast

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%tuple.2 = tuple(number, string)
// CHECK-NEXT:%tuple.3 = tuple(number, boolean, %tuple.2)

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'inner' Let : %tuple.2
// CHECK-NEXT:            Decl %d.3 'outer' Let : %tuple.3
// CHECK-NEXT:            Decl %d.4 'x' Let : number
// CHECK-NEXT:            Decl %d.5 'y' Let : boolean
// CHECK-NEXT:            Decl %d.6 'a' Let : number
// CHECK-NEXT:            Decl %d.7 'b' Let : string
// CHECK-NEXT:            Decl %d.8 't' Let : number
// CHECK-NEXT:            Decl %d.9 'u' Let : boolean
// CHECK-NEXT:            Decl %d.10 'v' Let : %tuple.2
// CHECK-NEXT:            Decl %d.11 'i' Let : number
// CHECK-NEXT:            Decl %d.12 'j' Let : string
// CHECK-NEXT:            Decl %d.13 'anyVar' Let : any
// CHECK-NEXT:            Decl %d.14 'anyNumber' Let : number
// CHECK-NEXT:            Decl %d.15 'anyString' Let : string
// CHECK-NEXT:            Decl %d.16 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    ExpressionStatement
// CHECK-NEXT:                        StringLiteral : string
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            ArrayExpression : %tuple.2
// CHECK-NEXT:                                NumericLiteral : number
// CHECK-NEXT:                                StringLiteral : string
// CHECK-NEXT:                            Id 'inner' [D:E:%d.2 'inner']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            ArrayExpression : %tuple.3
// CHECK-NEXT:                                NumericLiteral : number
// CHECK-NEXT:                                BooleanLiteral : boolean
// CHECK-NEXT:                                Id 'inner' [D:E:%d.2 'inner'] : %tuple.2
// CHECK-NEXT:                            Id 'outer' [D:E:%d.3 'outer']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            Id 'outer' [D:E:%d.3 'outer'] : %tuple.3
// CHECK-NEXT:                            ArrayPattern : %tuple.3
// CHECK-NEXT:                                Id 'x' [D:E:%d.4 'x'] : number
// CHECK-NEXT:                                Id 'y' [D:E:%d.5 'y'] : boolean
// CHECK-NEXT:                                ArrayPattern : %tuple.2
// CHECK-NEXT:                                    Id 'a' [D:E:%d.6 'a'] : number
// CHECK-NEXT:                                    Id 'b' [D:E:%d.7 'b'] : string
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            Id 'outer' [D:E:%d.3 'outer'] : %tuple.3
// CHECK-NEXT:                            ArrayPattern : %tuple.3
// CHECK-NEXT:                                Id 't' [D:E:%d.8 't'] : number
// CHECK-NEXT:                                Id 'u' [D:E:%d.9 'u'] : boolean
// CHECK-NEXT:                                Id 'v' [D:E:%d.10 'v'] : %tuple.2
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            Id 'i' [D:E:%d.11 'i']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            Id 'j' [D:E:%d.12 'j']
// CHECK-NEXT:                    ExpressionStatement
// CHECK-NEXT:                        AssignmentExpression : %tuple.2
// CHECK-NEXT:                            ArrayPattern : %tuple.2
// CHECK-NEXT:                                Id 'i' [D:E:%d.11 'i'] : number
// CHECK-NEXT:                                Id 'j' [D:E:%d.12 'j'] : string
// CHECK-NEXT:                            Id 'inner' [D:E:%d.2 'inner'] : %tuple.2
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            Id 'inner' [D:E:%d.2 'inner'] : %tuple.2
// CHECK-NEXT:                            Id 'anyVar' [D:E:%d.13 'anyVar']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            ImplicitCheckedCast : %tuple.2
// CHECK-NEXT:                                Id 'anyVar' [D:E:%d.13 'anyVar'] : any
// CHECK-NEXT:                            ArrayPattern : %tuple.2
// CHECK-NEXT:                                Id 'anyNumber' [D:E:%d.14 'anyNumber'] : number
// CHECK-NEXT:                                Id 'anyString' [D:E:%d.15 'anyString'] : string
// CHECK-NEXT:            ObjectExpression
