/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -typed -dump-sema -fno-std-globals %s | %FileCheckOrRegen %s --match-full-lines

try {} catch (e) { let x: number = 1; }

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%object.2 = object({
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'arguments' Var Arguments
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:            Scope %s.4
// CHECK-NEXT:                Decl %d.3 'e' ES5Catch : any
// CHECK-NEXT:                Scope %s.5
// CHECK-NEXT:                    Decl %d.4 'x' Let : number

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    TryStatement
// CHECK-NEXT:                        BlockStatement Scope %s.3
// CHECK-NEXT:                        CatchClause Scope %s.4
// CHECK-NEXT:                            Id 'e' [D:E:%d.3 'e']
// CHECK-NEXT:                            BlockStatement Scope %s.5
// CHECK-NEXT:                                VariableDeclaration
// CHECK-NEXT:                                    VariableDeclarator
// CHECK-NEXT:                                        NumericLiteral : number
// CHECK-NEXT:                                        Id 'x' [D:E:%d.4 'x']
// CHECK-NEXT:            ObjectExpression : %object.2
