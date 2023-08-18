/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -dump-sema -fno-std-globals %s | %FileCheckOrRegen %s --match-full-lines

// Typed main.
(function main(): void {
  // Untyped foo.
  function foo() {}

  // Untyped callsite.
  foo(1);
})();

// Auto-generated content below. Please do not modify manually.

// CHECK:function %t.1 = function (): void
// CHECK-NEXT:untyped function %t.2 = untyped function ()

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'main' FunctionExprName : function %t.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.3
// CHECK-NEXT:            Decl %d.2 'foo' ScopedFunction : untyped function %t.2
// CHECK-NEXT:            Decl %d.3 'arguments' Var Arguments
// CHECK-NEXT:            hoistedFunction foo
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.4
// CHECK-NEXT:                Decl %d.4 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : void
// CHECK-NEXT:            FunctionExpression : function %t.1 Scope %s.2
// CHECK-NEXT:                Id 'main' [D:E:%d.1 'main']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    FunctionDeclaration : untyped function %t.2
// CHECK-NEXT:                        Id 'foo' [D:E:%d.2 'foo']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                    ExpressionStatement
// CHECK-NEXT:                        CallExpression : any
// CHECK-NEXT:                            Id 'foo' [D:E:%d.2 'foo'] : untyped function %t.2
// CHECK-NEXT:                            NumericLiteral : number
