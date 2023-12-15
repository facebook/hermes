/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals --typed --dump-sema %s | %FileCheckOrRegen %s --match-full-lines

'use strict';

let x: [number, bool] = [1, true]
let y: number = x[0]
let z: bool = x[1]

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%tuple.2 = tuple(number, boolean)

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'x' Let : %tuple.2
// CHECK-NEXT:            Decl %d.3 'y' Let : number
// CHECK-NEXT:            Decl %d.4 'z' Let : boolean
// CHECK-NEXT:            Decl %d.5 'arguments' Var Arguments

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
// CHECK-NEXT:                                BooleanLiteral : boolean
// CHECK-NEXT:                            Id 'x' [D:E:%d.2 'x']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            MemberExpression : number
// CHECK-NEXT:                                Id 'x' [D:E:%d.2 'x'] : %tuple.2
// CHECK-NEXT:                                NumericLiteral : number
// CHECK-NEXT:                            Id 'y' [D:E:%d.3 'y']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            MemberExpression : boolean
// CHECK-NEXT:                                Id 'x' [D:E:%d.2 'x'] : %tuple.2
// CHECK-NEXT:                                NumericLiteral : number
// CHECK-NEXT:                            Id 'z' [D:E:%d.4 'z']
// CHECK-NEXT:            ObjectExpression
