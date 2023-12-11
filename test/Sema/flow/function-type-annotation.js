/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -typed -dump-sema -fno-std-globals %s | %FileCheckOrRegen %s --match-full-lines

(function foo(
  a: (x: number) => string,
  b: number => string,
  c: (this: number, x: number) => string,
) {
});

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%function.2 = function(x: number): string
// CHECK-NEXT:%function.3 = function(this: number, x: number): string
// CHECK-NEXT:%function.4 = function(a: %function.2, b: %function.2, c: %function.3): any

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'arguments' Var Arguments
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:                Decl %d.3 'foo' FunctionExprName : %function.4
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.4
// CHECK-NEXT:                Decl %d.4 'a' Parameter : %function.2
// CHECK-NEXT:                Decl %d.5 'b' Parameter : %function.2
// CHECK-NEXT:                Decl %d.6 'c' Parameter : %function.3
// CHECK-NEXT:                Decl %d.7 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    ExpressionStatement
// CHECK-NEXT:                        FunctionExpression : %function.4 Scope %s.3
// CHECK-NEXT:                            Id 'foo' [D:E:%d.3 'foo']
// CHECK-NEXT:                            Id 'a' [D:E:%d.4 'a']
// CHECK-NEXT:                            Id 'b' [D:E:%d.5 'b']
// CHECK-NEXT:                            Id 'c' [D:E:%d.6 'c']
// CHECK-NEXT:                            BlockStatement
// CHECK-NEXT:            ObjectExpression
