/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror --typed --dump-sema -fno-std-globals %s | %FileCheckOrRegen %s --match-full-lines

// Ensure that expressions in non-obvious places are visited and type annotated.

var arr: number[] = [];
var i: number;

// Ensure that the "i" is annotated.
for(i in arr);
for(i of arr);

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%array.2 = array(number)

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'arr' Var : %array.2
// CHECK-NEXT:            Decl %d.3 'i' Var : number
// CHECK-NEXT:            Decl %d.4 'arguments' Var Arguments
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:            Scope %s.4

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            ArrayExpression : %array.2
// CHECK-NEXT:                            Id 'arr' [D:E:%d.2 'arr']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            Id 'i' [D:E:%d.3 'i']
// CHECK-NEXT:                    ForInStatement Scope %s.3
// CHECK-NEXT:                        Id 'i' [D:E:%d.3 'i'] : number
// CHECK-NEXT:                        Id 'arr' [D:E:%d.2 'arr'] : %array.2
// CHECK-NEXT:                        EmptyStatement
// CHECK-NEXT:                    ForOfStatement Scope %s.4
// CHECK-NEXT:                        Id 'i' [D:E:%d.3 'i'] : number
// CHECK-NEXT:                        Id 'arr' [D:E:%d.2 'arr'] : %array.2
// CHECK-NEXT:                        EmptyStatement
// CHECK-NEXT:            ObjectExpression
