/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals --typed --dump-sema %s | %FileCheckOrRegen %s --match-full-lines

(function() {

function f(x: any, n: number) {
  // These need ImplicitCheckedCast on the LHS.
  n += x;
  n -= x;
}

});

// Auto-generated content below. Please do not modify manually.

// CHECK:untyped function %t.1 = untyped function ()
// CHECK-NEXT:function %t.2 = function (x: any, n: number): any

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:                Decl %d.3 'f' ScopedFunction : function %t.2
// CHECK-NEXT:                Decl %d.4 'arguments' Var Arguments
// CHECK-NEXT:                hoistedFunction f
// CHECK-NEXT:            Func strict
// CHECK-NEXT:                Scope %s.4
// CHECK-NEXT:                    Decl %d.5 'x' Parameter : any
// CHECK-NEXT:                    Decl %d.6 'n' Parameter : number
// CHECK-NEXT:                    Decl %d.7 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : untyped function %t.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    ExpressionStatement
// CHECK-NEXT:                        FunctionExpression : untyped function %t.1
// CHECK-NEXT:                            BlockStatement
// CHECK-NEXT:                                FunctionDeclaration : function %t.2
// CHECK-NEXT:                                    Id 'f' [D:E:%d.3 'f']
// CHECK-NEXT:                                    Id 'x' [D:E:%d.5 'x']
// CHECK-NEXT:                                    Id 'n' [D:E:%d.6 'n']
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        ExpressionStatement
// CHECK-NEXT:                                            AssignmentExpression : any
// CHECK-NEXT:                                                ImplicitCheckedCast : number
// CHECK-NEXT:                                                    Id 'n' [D:E:%d.6 'n'] : number
// CHECK-NEXT:                                                Id 'x' [D:E:%d.5 'x'] : any
// CHECK-NEXT:                                        ExpressionStatement
// CHECK-NEXT:                                            AssignmentExpression : any
// CHECK-NEXT:                                                ImplicitCheckedCast : number
// CHECK-NEXT:                                                    Id 'n' [D:E:%d.6 'n'] : number
// CHECK-NEXT:                                                Id 'x' [D:E:%d.5 'x'] : any
// CHECK-NEXT:            ObjectExpression
