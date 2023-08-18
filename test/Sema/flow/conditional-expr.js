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

// CHECK:untyped function %t.1 = untyped function ()
// CHECK-NEXT:function %t.2 = function (b: boolean, x: number, y: number, z: string): any
// CHECK-NEXT:union %t.3 = union string | number

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'foo' ScopedFunction : function %t.2
// CHECK-NEXT:            Decl %d.2 'arguments' Var Arguments
// CHECK-NEXT:            hoistedFunction foo
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:                Decl %d.3 'b' Parameter : boolean
// CHECK-NEXT:                Decl %d.4 'x' Parameter : number
// CHECK-NEXT:                Decl %d.5 'y' Parameter : number
// CHECK-NEXT:                Decl %d.6 'z' Parameter : string
// CHECK-NEXT:                Decl %d.7 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        FunctionExpression : untyped function %t.1
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                FunctionDeclaration : function %t.2
// CHECK-NEXT:                    Id 'foo' [D:E:%d.1 'foo']
// CHECK-NEXT:                    Id 'b' [D:E:%d.3 'b']
// CHECK-NEXT:                    Id 'x' [D:E:%d.4 'x']
// CHECK-NEXT:                    Id 'y' [D:E:%d.5 'y']
// CHECK-NEXT:                    Id 'z' [D:E:%d.6 'z']
// CHECK-NEXT:                    BlockStatement
// CHECK-NEXT:                        ExpressionStatement
// CHECK-NEXT:                            ConditionalExpression : number
// CHECK-NEXT:                                Id 'b' [D:E:%d.3 'b'] : boolean
// CHECK-NEXT:                                Id 'y' [D:E:%d.5 'y'] : number
// CHECK-NEXT:                                Id 'x' [D:E:%d.4 'x'] : number
// CHECK-NEXT:                        ExpressionStatement
// CHECK-NEXT:                            ConditionalExpression : union %t.3
// CHECK-NEXT:                                Id 'b' [D:E:%d.3 'b'] : boolean
// CHECK-NEXT:                                Id 'z' [D:E:%d.6 'z'] : string
// CHECK-NEXT:                                Id 'x' [D:E:%d.4 'x'] : number
