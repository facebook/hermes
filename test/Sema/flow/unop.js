/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals --typed --dump-sema %s | %FileCheckOrRegen %s --match-full-lines

+1;
-1;
-1n;
!1;
~1;
~1n;
typeof 1;
void 1;
let x: number = 1;
++x;
--x;
let y: bigint = 1n;
++y;
--y;

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'x' Let : number
// CHECK-NEXT:            Decl %d.3 'y' Let : bigint
// CHECK-NEXT:            Decl %d.4 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    ExpressionStatement
// CHECK-NEXT:                        UnaryExpression : number
// CHECK-NEXT:                            NumericLiteral : number
// CHECK-NEXT:                    ExpressionStatement
// CHECK-NEXT:                        UnaryExpression : number
// CHECK-NEXT:                            NumericLiteral : number
// CHECK-NEXT:                    ExpressionStatement
// CHECK-NEXT:                        UnaryExpression : bigint
// CHECK-NEXT:                            BigIntLiteral : bigint
// CHECK-NEXT:                    ExpressionStatement
// CHECK-NEXT:                        UnaryExpression : boolean
// CHECK-NEXT:                            NumericLiteral : number
// CHECK-NEXT:                    ExpressionStatement
// CHECK-NEXT:                        UnaryExpression : number
// CHECK-NEXT:                            NumericLiteral : number
// CHECK-NEXT:                    ExpressionStatement
// CHECK-NEXT:                        UnaryExpression : bigint
// CHECK-NEXT:                            BigIntLiteral : bigint
// CHECK-NEXT:                    ExpressionStatement
// CHECK-NEXT:                        UnaryExpression : string
// CHECK-NEXT:                            NumericLiteral : number
// CHECK-NEXT:                    ExpressionStatement
// CHECK-NEXT:                        UnaryExpression : void
// CHECK-NEXT:                            NumericLiteral : number
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            NumericLiteral : number
// CHECK-NEXT:                            Id 'x' [D:E:%d.2 'x']
// CHECK-NEXT:                    ExpressionStatement
// CHECK-NEXT:                        UpdateExpression : number
// CHECK-NEXT:                            Id 'x' [D:E:%d.2 'x'] : number
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
// CHECK-NEXT:                    ExpressionStatement
// CHECK-NEXT:                        UpdateExpression : bigint
// CHECK-NEXT:                            Id 'y' [D:E:%d.3 'y'] : bigint
// CHECK-NEXT:            ObjectExpression
