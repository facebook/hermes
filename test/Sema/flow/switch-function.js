/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror --typed --dump-sema -fno-std-globals %s | %FileCheckOrRegen %s --match-full-lines

switch (0) {
  default:
    function foo(): void {}
    foo();
}

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%function.2 = function(): void
// CHECK-NEXT:%object.3 = object({
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'arguments' Var Arguments
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:                Decl %d.3 'foo' ScopedFunction : %function.2
// CHECK-NEXT:                hoistedFunction foo
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.4
// CHECK-NEXT:                Decl %d.4 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    SwitchStatement Scope %s.3
// CHECK-NEXT:                        NumericLiteral : number
// CHECK-NEXT:                        SwitchCase
// CHECK-NEXT:                            FunctionDeclaration : %function.2
// CHECK-NEXT:                                Id 'foo' [D:E:%d.3 'foo']
// CHECK-NEXT:                                BlockStatement
// CHECK-NEXT:                            ExpressionStatement
// CHECK-NEXT:                                CallExpression : void
// CHECK-NEXT:                                    Id 'foo' [D:E:%d.3 'foo'] : %function.2
// CHECK-NEXT:            ObjectExpression : %object.3
