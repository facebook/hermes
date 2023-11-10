/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals -typed -dump-sema %s | %FileCheckOrRegen %s --match-full-lines

(function() {
  function foo(b: bool, x: number, y: number, z: string) {
    b ? x : y;
    b ? x : z;
  }
})

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%function.2 = function(b: boolean, x: number, y: number, z: string): any
// CHECK-NEXT:%union.3 = union(string | number)

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:                Decl %d.3 'foo' ScopedFunction : %function.2
// CHECK-NEXT:                Decl %d.4 'arguments' Var Arguments
// CHECK-NEXT:                hoistedFunction foo
// CHECK-NEXT:            Func strict
// CHECK-NEXT:                Scope %s.4
// CHECK-NEXT:                    Decl %d.5 'b' Parameter : boolean
// CHECK-NEXT:                    Decl %d.6 'x' Parameter : number
// CHECK-NEXT:                    Decl %d.7 'y' Parameter : number
// CHECK-NEXT:                    Decl %d.8 'z' Parameter : string
// CHECK-NEXT:                    Decl %d.9 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    ExpressionStatement
// CHECK-NEXT:                        FunctionExpression : %untyped_function.1
// CHECK-NEXT:                            BlockStatement
// CHECK-NEXT:                                FunctionDeclaration : %function.2
// CHECK-NEXT:                                    Id 'foo' [D:E:%d.3 'foo']
// CHECK-NEXT:                                    Id 'b' [D:E:%d.5 'b']
// CHECK-NEXT:                                    Id 'x' [D:E:%d.6 'x']
// CHECK-NEXT:                                    Id 'y' [D:E:%d.7 'y']
// CHECK-NEXT:                                    Id 'z' [D:E:%d.8 'z']
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        ExpressionStatement
// CHECK-NEXT:                                            ConditionalExpression : number
// CHECK-NEXT:                                                Id 'b' [D:E:%d.5 'b'] : boolean
// CHECK-NEXT:                                                Id 'y' [D:E:%d.7 'y'] : number
// CHECK-NEXT:                                                Id 'x' [D:E:%d.6 'x'] : number
// CHECK-NEXT:                                        ExpressionStatement
// CHECK-NEXT:                                            ConditionalExpression : %union.3
// CHECK-NEXT:                                                Id 'b' [D:E:%d.5 'b'] : boolean
// CHECK-NEXT:                                                Id 'z' [D:E:%d.8 'z'] : string
// CHECK-NEXT:                                                Id 'x' [D:E:%d.6 'x'] : number
// CHECK-NEXT:            ObjectExpression
