/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror --typed --dump-sema %s | %FileCheckOrRegen %s --match-full-lines

'use strict';

let a: number[][] = [[1, 2], [3, 4], [], ...[[10, 11], [12, 13]]];

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%class.2 = class(Array {
// CHECK-NEXT:  %homeObject: %class.4
// CHECK-NEXT:})
// CHECK-NEXT:%class.3 = class(Array {
// CHECK-NEXT:  %homeObject: %class.5
// CHECK-NEXT:})
// CHECK-NEXT:%class.4 = class( {
// CHECK-NEXT:  map [final]: generic
// CHECK-NEXT:  toString [final]: %function.6
// CHECK-NEXT:})
// CHECK-NEXT:%class.5 = class( {
// CHECK-NEXT:  map [final]: generic
// CHECK-NEXT:  toString [final]: %function.7
// CHECK-NEXT:})
// CHECK-NEXT:%function.6 = function(this: %class.2): string
// CHECK-NEXT:%function.7 = function(this: %class.3): string

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'a' Let : %class.3
// CHECK-NEXT:        Decl %d.3 'arguments' Var Arguments

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            StringLiteral : string
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                ArrayExpression : %class.3
// CHECK-NEXT:                    ArrayExpression : %class.2
// CHECK-NEXT:                        NumericLiteral : number
// CHECK-NEXT:                        NumericLiteral : number
// CHECK-NEXT:                    ArrayExpression : %class.2
// CHECK-NEXT:                        NumericLiteral : number
// CHECK-NEXT:                        NumericLiteral : number
// CHECK-NEXT:                    ArrayExpression : %class.2
// CHECK-NEXT:                    SpreadElement
// CHECK-NEXT:                        ArrayExpression : %class.3
// CHECK-NEXT:                            ArrayExpression : %class.2
// CHECK-NEXT:                                NumericLiteral : number
// CHECK-NEXT:                                NumericLiteral : number
// CHECK-NEXT:                            ArrayExpression : %class.2
// CHECK-NEXT:                                NumericLiteral : number
// CHECK-NEXT:                                NumericLiteral : number
// CHECK-NEXT:                Id 'a' [D:E:%d.2 'a']
