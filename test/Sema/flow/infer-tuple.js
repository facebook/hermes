/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals --typed --dump-sema %s | %FileCheckOrRegen %s --match-full-lines

'use strict';

let a: [number, bool | string] = [1, true];
let b: [number, bool] = [1, true];
let c = ([1, true]: [number, bool]);
function d(): [number, bool] {
  return [1, true];
}

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%union.2 = union(boolean | string)
// CHECK-NEXT:%tuple.3 = tuple(number, %union.2)
// CHECK-NEXT:%tuple.4 = tuple(number, boolean)
// CHECK-NEXT:%function.5 = function(): %tuple.4

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'a' Let : %tuple.3
// CHECK-NEXT:            Decl %d.3 'b' Let : %tuple.4
// CHECK-NEXT:            Decl %d.4 'c' Let : %tuple.4
// CHECK-NEXT:            Decl %d.5 'd' ScopedFunction : %function.5
// CHECK-NEXT:            Decl %d.6 'arguments' Var Arguments
// CHECK-NEXT:            hoistedFunction d
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:                Decl %d.7 'arguments' Var Arguments

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
// CHECK-NEXT:                            ArrayExpression : %tuple.3
// CHECK-NEXT:                                NumericLiteral : number
// CHECK-NEXT:                                BooleanLiteral : boolean
// CHECK-NEXT:                            Id 'a' [D:E:%d.2 'a']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            ArrayExpression : %tuple.4
// CHECK-NEXT:                                NumericLiteral : number
// CHECK-NEXT:                                BooleanLiteral : boolean
// CHECK-NEXT:                            Id 'b' [D:E:%d.3 'b']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            TypeCastExpression : %tuple.4
// CHECK-NEXT:                                ArrayExpression : %tuple.4
// CHECK-NEXT:                                    NumericLiteral : number
// CHECK-NEXT:                                    BooleanLiteral : boolean
// CHECK-NEXT:                            Id 'c' [D:E:%d.4 'c']
// CHECK-NEXT:                    FunctionDeclaration : %function.5
// CHECK-NEXT:                        Id 'd' [D:E:%d.5 'd']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            ReturnStatement
// CHECK-NEXT:                                ArrayExpression : %tuple.4
// CHECK-NEXT:                                    NumericLiteral : number
// CHECK-NEXT:                                    BooleanLiteral : boolean
// CHECK-NEXT:            ObjectExpression
