/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -fno-std-globals --typed --dump-sema %s | %FileCheckOrRegen %s --match-full-lines

// Test that an assignment with a narrower RHS into a wider LHS takes the
// narrower type.
function f(a: any, u: number|string, n: number) {
  n = u = 5;
  let n1: number;
  n1 = n = a;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%union.2 = union(string | number)
// CHECK-NEXT:%function.3 = function(a: any, u: %union.2, n: number): any

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'f' ScopedFunction : %function.3
// CHECK-NEXT:            Decl %d.3 'arguments' Var Arguments
// CHECK-NEXT:            hoistedFunction f
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:                Decl %d.4 'a' Parameter : any
// CHECK-NEXT:                Decl %d.5 'u' Parameter : %union.2
// CHECK-NEXT:                Decl %d.6 'n' Parameter : number
// CHECK-NEXT:                Decl %d.7 'n1' Let : number
// CHECK-NEXT:                Decl %d.8 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    FunctionDeclaration : %function.3
// CHECK-NEXT:                        Id 'f' [D:E:%d.2 'f']
// CHECK-NEXT:                        Id 'a' [D:E:%d.4 'a']
// CHECK-NEXT:                        Id 'u' [D:E:%d.5 'u']
// CHECK-NEXT:                        Id 'n' [D:E:%d.6 'n']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            ExpressionStatement
// CHECK-NEXT:                                AssignmentExpression : number
// CHECK-NEXT:                                    Id 'n' [D:E:%d.6 'n'] : number
// CHECK-NEXT:                                    AssignmentExpression : number
// CHECK-NEXT:                                        Id 'u' [D:E:%d.5 'u'] : %union.2
// CHECK-NEXT:                                        NumericLiteral : number
// CHECK-NEXT:                            VariableDeclaration
// CHECK-NEXT:                                VariableDeclarator
// CHECK-NEXT:                                    Id 'n1' [D:E:%d.7 'n1']
// CHECK-NEXT:                            ExpressionStatement
// CHECK-NEXT:                                AssignmentExpression : number
// CHECK-NEXT:                                    Id 'n1' [D:E:%d.7 'n1'] : number
// CHECK-NEXT:                                    AssignmentExpression : number
// CHECK-NEXT:                                        Id 'n' [D:E:%d.6 'n'] : number
// CHECK-NEXT:                                        ImplicitCheckedCast : number
// CHECK-NEXT:                                            Id 'a' [D:E:%d.4 'a'] : any
// CHECK-NEXT:            ObjectExpression
