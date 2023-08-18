/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -dump-sema -fno-std-globals %s | %FileCheckOrRegen %s --match-full-lines

(function foo(
  a: (x: number) => string,
  b: number => string,
  c: (this: number, x: number) => string,
) {
});

// Auto-generated content below. Please do not modify manually.

// CHECK:function %t.1 = function (x: number): string
// CHECK-NEXT:function %t.2 = function (this: number, x: number): string
// CHECK-NEXT:function %t.3 = function (a: function %t.1, b: function %t.1, c: function %t.2): any

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'foo' FunctionExprName : function %t.3
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.3
// CHECK-NEXT:            Decl %d.2 'a' Parameter : function %t.1
// CHECK-NEXT:            Decl %d.3 'b' Parameter : function %t.1
// CHECK-NEXT:            Decl %d.4 'c' Parameter : function %t.2
// CHECK-NEXT:            Decl %d.5 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        FunctionExpression : function %t.3 Scope %s.2
// CHECK-NEXT:            Id 'foo' [D:E:%d.1 'foo']
// CHECK-NEXT:            Id 'a' [D:E:%d.2 'a']
// CHECK-NEXT:            Id 'b' [D:E:%d.3 'b']
// CHECK-NEXT:            Id 'c' [D:E:%d.4 'c']
// CHECK-NEXT:            BlockStatement
