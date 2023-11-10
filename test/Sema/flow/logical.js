/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals --typed --dump-sema %s | %FileCheckOrRegen %s --match-full-lines

(function() {
  function foo(x: ?number, y: number|null, z: number) {
    x && y;
    x || y;
    x ?? y;
    x && z;
    x || z;
    x ?? z;
    z && z;
  }
})

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%union.2 = union(void | null | number)
// CHECK-NEXT:%union.3 = union(null | number)
// CHECK-NEXT:%function.4 = function(x: %union.2, y: %union.3, z: number): any

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:                Decl %d.3 'foo' ScopedFunction : %function.4
// CHECK-NEXT:                Decl %d.4 'arguments' Var Arguments
// CHECK-NEXT:                hoistedFunction foo
// CHECK-NEXT:            Func strict
// CHECK-NEXT:                Scope %s.4
// CHECK-NEXT:                    Decl %d.5 'x' Parameter : %union.2
// CHECK-NEXT:                    Decl %d.6 'y' Parameter : %union.3
// CHECK-NEXT:                    Decl %d.7 'z' Parameter : number
// CHECK-NEXT:                    Decl %d.8 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    ExpressionStatement
// CHECK-NEXT:                        FunctionExpression : %untyped_function.1
// CHECK-NEXT:                            BlockStatement
// CHECK-NEXT:                                FunctionDeclaration : %function.4
// CHECK-NEXT:                                    Id 'foo' [D:E:%d.3 'foo']
// CHECK-NEXT:                                    Id 'x' [D:E:%d.5 'x']
// CHECK-NEXT:                                    Id 'y' [D:E:%d.6 'y']
// CHECK-NEXT:                                    Id 'z' [D:E:%d.7 'z']
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        ExpressionStatement
// CHECK-NEXT:                                            LogicalExpression : %union.2
// CHECK-NEXT:                                                Id 'x' [D:E:%d.5 'x'] : %union.2
// CHECK-NEXT:                                                Id 'y' [D:E:%d.6 'y'] : %union.3
// CHECK-NEXT:                                        ExpressionStatement
// CHECK-NEXT:                                            LogicalExpression : %union.3
// CHECK-NEXT:                                                Id 'x' [D:E:%d.5 'x'] : %union.2
// CHECK-NEXT:                                                Id 'y' [D:E:%d.6 'y'] : %union.3
// CHECK-NEXT:                                        ExpressionStatement
// CHECK-NEXT:                                            LogicalExpression : %union.3
// CHECK-NEXT:                                                Id 'x' [D:E:%d.5 'x'] : %union.2
// CHECK-NEXT:                                                Id 'y' [D:E:%d.6 'y'] : %union.3
// CHECK-NEXT:                                        ExpressionStatement
// CHECK-NEXT:                                            LogicalExpression : %union.2
// CHECK-NEXT:                                                Id 'x' [D:E:%d.5 'x'] : %union.2
// CHECK-NEXT:                                                Id 'z' [D:E:%d.7 'z'] : number
// CHECK-NEXT:                                        ExpressionStatement
// CHECK-NEXT:                                            LogicalExpression : number
// CHECK-NEXT:                                                Id 'x' [D:E:%d.5 'x'] : %union.2
// CHECK-NEXT:                                                Id 'z' [D:E:%d.7 'z'] : number
// CHECK-NEXT:                                        ExpressionStatement
// CHECK-NEXT:                                            LogicalExpression : number
// CHECK-NEXT:                                                Id 'x' [D:E:%d.5 'x'] : %union.2
// CHECK-NEXT:                                                Id 'z' [D:E:%d.7 'z'] : number
// CHECK-NEXT:                                        ExpressionStatement
// CHECK-NEXT:                                            LogicalExpression : number
// CHECK-NEXT:                                                Id 'z' [D:E:%d.7 'z'] : number
// CHECK-NEXT:                                                Id 'z' [D:E:%d.7 'z'] : number
// CHECK-NEXT:            ObjectExpression
