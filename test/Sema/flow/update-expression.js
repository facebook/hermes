/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -fno-std-globals --typed --dump-sema %s | %FileCheckOrRegen %s --match-full-lines

'use strict';

let x: number = 0;
x++;

let y: bigint = 0n;
y++;

let z: bigint|number = 0;
z++;

let w: any = 0;
w++;

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%union.2 = union(number | bigint)

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'x' Let : number
// CHECK-NEXT:            Decl %d.3 'y' Let : bigint
// CHECK-NEXT:            Decl %d.4 'z' Let : %union.2
// CHECK-NEXT:            Decl %d.5 'w' Let : any
// CHECK-NEXT:            Decl %d.6 'arguments' Var Arguments

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
// CHECK-NEXT:                            NumericLiteral : number
// CHECK-NEXT:                            Id 'x' [D:E:%d.2 'x']
// CHECK-NEXT:                    ExpressionStatement
// CHECK-NEXT:                        UpdateExpression : number
// CHECK-NEXT:                            Id 'x' [D:E:%d.2 'x'] : number
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            BigIntLiteral : bigint
// CHECK-NEXT:                            Id 'y' [D:E:%d.3 'y']
// CHECK-NEXT:                    ExpressionStatement
// CHECK-NEXT:                        UpdateExpression : bigint
// CHECK-NEXT:                            Id 'y' [D:E:%d.3 'y'] : bigint
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            NumericLiteral : number
// CHECK-NEXT:                            Id 'z' [D:E:%d.4 'z']
// CHECK-NEXT:                    ExpressionStatement
// CHECK-NEXT:                        UpdateExpression : %union.2
// CHECK-NEXT:                            Id 'z' [D:E:%d.4 'z'] : %union.2
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            NumericLiteral : number
// CHECK-NEXT:                            Id 'w' [D:E:%d.5 'w']
// CHECK-NEXT:                    ExpressionStatement
// CHECK-NEXT:                        UpdateExpression : any
// CHECK-NEXT:                            Id 'w' [D:E:%d.5 'w'] : any
// CHECK-NEXT:            ObjectExpression
