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

// CHECK:untyped function %t.1 = untyped function ()
// CHECK-NEXT:union %t.2 = union void | null | number
// CHECK-NEXT:union %t.3 = union null | number
// CHECK-NEXT:function %t.4 = function (x: union %t.2, y: union %t.3, z: number): any

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'foo' ScopedFunction : function %t.4
// CHECK-NEXT:            Decl %d.2 'arguments' Var Arguments
// CHECK-NEXT:            hoistedFunction foo
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:                Decl %d.3 'x' Parameter : union %t.2
// CHECK-NEXT:                Decl %d.4 'y' Parameter : union %t.3
// CHECK-NEXT:                Decl %d.5 'z' Parameter : number
// CHECK-NEXT:                Decl %d.6 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        FunctionExpression : untyped function %t.1
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                FunctionDeclaration : function %t.4
// CHECK-NEXT:                    Id 'foo' [D:E:%d.1 'foo']
// CHECK-NEXT:                    Id 'x' [D:E:%d.3 'x']
// CHECK-NEXT:                    Id 'y' [D:E:%d.4 'y']
// CHECK-NEXT:                    Id 'z' [D:E:%d.5 'z']
// CHECK-NEXT:                    BlockStatement
// CHECK-NEXT:                        ExpressionStatement
// CHECK-NEXT:                            LogicalExpression : union %t.2
// CHECK-NEXT:                                Id 'x' [D:E:%d.3 'x'] : union %t.2
// CHECK-NEXT:                                Id 'y' [D:E:%d.4 'y'] : union %t.3
// CHECK-NEXT:                        ExpressionStatement
// CHECK-NEXT:                            LogicalExpression : union %t.3
// CHECK-NEXT:                                Id 'x' [D:E:%d.3 'x'] : union %t.2
// CHECK-NEXT:                                Id 'y' [D:E:%d.4 'y'] : union %t.3
// CHECK-NEXT:                        ExpressionStatement
// CHECK-NEXT:                            LogicalExpression : union %t.3
// CHECK-NEXT:                                Id 'x' [D:E:%d.3 'x'] : union %t.2
// CHECK-NEXT:                                Id 'y' [D:E:%d.4 'y'] : union %t.3
// CHECK-NEXT:                        ExpressionStatement
// CHECK-NEXT:                            LogicalExpression : union %t.2
// CHECK-NEXT:                                Id 'x' [D:E:%d.3 'x'] : union %t.2
// CHECK-NEXT:                                Id 'z' [D:E:%d.5 'z'] : number
// CHECK-NEXT:                        ExpressionStatement
// CHECK-NEXT:                            LogicalExpression : number
// CHECK-NEXT:                                Id 'x' [D:E:%d.3 'x'] : union %t.2
// CHECK-NEXT:                                Id 'z' [D:E:%d.5 'z'] : number
// CHECK-NEXT:                        ExpressionStatement
// CHECK-NEXT:                            LogicalExpression : number
// CHECK-NEXT:                                Id 'x' [D:E:%d.3 'x'] : union %t.2
// CHECK-NEXT:                                Id 'z' [D:E:%d.5 'z'] : number
// CHECK-NEXT:                        ExpressionStatement
// CHECK-NEXT:                            LogicalExpression : number
// CHECK-NEXT:                                Id 'z' [D:E:%d.5 'z'] : number
// CHECK-NEXT:                                Id 'z' [D:E:%d.5 'z'] : number
