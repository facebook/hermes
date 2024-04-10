/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals -typed -dump-sema %s | %FileCheckOrRegen %s --match-full-lines

function f1(x: [number, string]) {}
f1([1, "abc"]);

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%tuple.2 = tuple(number, string)
// CHECK-NEXT:%function.3 = function(x: %tuple.2): any
// CHECK-NEXT:%object.4 = object({
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'f1' ScopedFunction : %function.3
// CHECK-NEXT:            Decl %d.3 'arguments' Var Arguments
// CHECK-NEXT:            hoistedFunction f1
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:                Decl %d.4 'x' Parameter : %tuple.2
// CHECK-NEXT:                Decl %d.5 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    FunctionDeclaration : %function.3
// CHECK-NEXT:                        Id 'f1' [D:E:%d.2 'f1']
// CHECK-NEXT:                        Id 'x' [D:E:%d.4 'x']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                    ExpressionStatement
// CHECK-NEXT:                        CallExpression : any
// CHECK-NEXT:                            Id 'f1' [D:E:%d.2 'f1'] : %function.3
// CHECK-NEXT:                            ArrayExpression : %tuple.2
// CHECK-NEXT:                                NumericLiteral : number
// CHECK-NEXT:                                StringLiteral : string
// CHECK-NEXT:            ObjectExpression : %object.4
