/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -fno-std-globals --typed --dump-sema %s | %FileCheckOrRegen %s --match-full-lines

'use strict';

let a: number[][] = [[1, 2], [3, 4], [], ...[[10, 11], [12, 13]]];

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%array.2 = array(number)
// CHECK-NEXT:%array.3 = array(%array.2)
// CHECK-NEXT:%object.4 = object({
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'a' Let : %array.3
// CHECK-NEXT:            Decl %d.3 'arguments' Var Arguments

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
// CHECK-NEXT:                            ArrayExpression : %array.3
// CHECK-NEXT:                                ArrayExpression : %array.2
// CHECK-NEXT:                                    NumericLiteral : number
// CHECK-NEXT:                                    NumericLiteral : number
// CHECK-NEXT:                                ArrayExpression : %array.2
// CHECK-NEXT:                                    NumericLiteral : number
// CHECK-NEXT:                                    NumericLiteral : number
// CHECK-NEXT:                                ArrayExpression : %array.2
// CHECK-NEXT:                                SpreadElement
// CHECK-NEXT:                                    ArrayExpression : %array.3
// CHECK-NEXT:                                        ArrayExpression : %array.2
// CHECK-NEXT:                                            NumericLiteral : number
// CHECK-NEXT:                                            NumericLiteral : number
// CHECK-NEXT:                                        ArrayExpression : %array.2
// CHECK-NEXT:                                            NumericLiteral : number
// CHECK-NEXT:                                            NumericLiteral : number
// CHECK-NEXT:                            Id 'a' [D:E:%d.2 'a']
// CHECK-NEXT:            ObjectExpression : %object.4
