/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -typed -dump-sema -fno-std-globals %s | %FileCheckOrRegen %s --match-full-lines

// Typed main.
(function main(): void {
  // Untyped foo.
  function foo() {}

  // Untyped callsite.
  foo(1);
})();

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%function.2 = function(): void

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'arguments' Var Arguments
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:                Decl %d.3 'main' FunctionExprName : %function.2
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.4
// CHECK-NEXT:                Decl %d.4 'foo' ScopedFunction : %untyped_function.1
// CHECK-NEXT:                Decl %d.5 'arguments' Var Arguments
// CHECK-NEXT:                hoistedFunction foo
// CHECK-NEXT:            Func strict
// CHECK-NEXT:                Scope %s.5
// CHECK-NEXT:                    Decl %d.6 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    ExpressionStatement
// CHECK-NEXT:                        CallExpression : void
// CHECK-NEXT:                            FunctionExpression : %function.2 Scope %s.3
// CHECK-NEXT:                                Id 'main' [D:E:%d.3 'main']
// CHECK-NEXT:                                BlockStatement
// CHECK-NEXT:                                    FunctionDeclaration : %untyped_function.1
// CHECK-NEXT:                                        Id 'foo' [D:E:%d.4 'foo']
// CHECK-NEXT:                                        BlockStatement
// CHECK-NEXT:                                    ExpressionStatement
// CHECK-NEXT:                                        CallExpression : any
// CHECK-NEXT:                                            Id 'foo' [D:E:%d.4 'foo'] : %untyped_function.1
// CHECK-NEXT:                                            NumericLiteral : number
// CHECK-NEXT:            ObjectExpression
