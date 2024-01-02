/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals --typed --dump-sema %s | %FileCheckOrRegen %s --match-full-lines

'use strict';

function foo(c: [number, string][]) {
  // a, b aren't typed yet, just make sure it doesn't crash.
  for (const [a, b] of c) {
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%tuple.2 = tuple(number, string)
// CHECK-NEXT:%array.3 = array(%tuple.2)
// CHECK-NEXT:%function.4 = function(c: %array.3): any

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'foo' ScopedFunction : %function.4
// CHECK-NEXT:            Decl %d.3 'arguments' Var Arguments
// CHECK-NEXT:            hoistedFunction foo
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:                Decl %d.4 'c' Parameter : %array.3
// CHECK-NEXT:                Decl %d.5 'arguments' Var Arguments
// CHECK-NEXT:                Scope %s.4
// CHECK-NEXT:                    Decl %d.6 'a' Const
// CHECK-NEXT:                    Decl %d.7 'b' Const
// CHECK-NEXT:                    Scope %s.5

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    ExpressionStatement
// CHECK-NEXT:                        StringLiteral : string
// CHECK-NEXT:                    FunctionDeclaration : %function.4
// CHECK-NEXT:                        Id 'foo' [D:E:%d.2 'foo']
// CHECK-NEXT:                        Id 'c' [D:E:%d.4 'c']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            ForOfStatement Scope %s.4
// CHECK-NEXT:                                VariableDeclaration
// CHECK-NEXT:                                    VariableDeclarator
// CHECK-NEXT:                                        ArrayPattern
// CHECK-NEXT:                                            Id 'a' [D:E:%d.6 'a']
// CHECK-NEXT:                                            Id 'b' [D:E:%d.7 'b']
// CHECK-NEXT:                                Id 'c' [D:E:%d.4 'c'] : %array.3
// CHECK-NEXT:                                BlockStatement Scope %s.5
// CHECK-NEXT:            ObjectExpression
