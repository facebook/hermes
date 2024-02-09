/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror --typed --dump-sema -fno-std-globals %s | %FileCheckOrRegen %s --match-full-lines

function foo(x: any) {
  let y: number;
  y = x;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%function.2 = function(x: any): any

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'foo' ScopedFunction : %function.2
// CHECK-NEXT:            Decl %d.3 'arguments' Var Arguments
// CHECK-NEXT:            hoistedFunction foo
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:                Decl %d.4 'x' Parameter : any
// CHECK-NEXT:                Decl %d.5 'y' Let : number
// CHECK-NEXT:                Decl %d.6 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    FunctionDeclaration : %function.2
// CHECK-NEXT:                        Id 'foo' [D:E:%d.2 'foo']
// CHECK-NEXT:                        Id 'x' [D:E:%d.4 'x']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            VariableDeclaration
// CHECK-NEXT:                                VariableDeclarator
// CHECK-NEXT:                                    Id 'y' [D:E:%d.5 'y']
// CHECK-NEXT:                            ExpressionStatement
// CHECK-NEXT:                                AssignmentExpression : number
// CHECK-NEXT:                                    Id 'y' [D:E:%d.5 'y'] : number
// CHECK-NEXT:                                    ImplicitCheckedCast : number
// CHECK-NEXT:                                        Id 'x' [D:E:%d.4 'x'] : any
// CHECK-NEXT:            ObjectExpression
