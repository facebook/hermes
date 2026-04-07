/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes --typed --dump-sema %s | %FileCheckOrRegen %s --match-full-lines

'use strict';

function foo(c: [number, string][]) {
  // a, b aren't typed yet, just make sure it doesn't crash.
  for (const [a, b] of c) {
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%class.2 = class(Array<%tuple.4>)
// CHECK-NEXT:%function.3 = function(c: %class.2): any
// CHECK-NEXT:%tuple.4 = tuple(number, string)

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'foo' Var : %function.3
// CHECK-NEXT:        Decl %d.3 'arguments' Var Arguments
// CHECK-NEXT:        hoistedFunction foo
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.4 'c' Parameter : %class.2
// CHECK-NEXT:            Decl %d.5 'arguments' Var Arguments
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:                Decl %d.6 'a' Const : any
// CHECK-NEXT:                Decl %d.7 'b' Const : any
// CHECK-NEXT:                Scope %s.4

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            StringLiteral : string
// CHECK-NEXT:        FunctionDeclaration : %function.3
// CHECK-NEXT:            Id 'foo' [D:E:%d.2 'foo']
// CHECK-NEXT:            Id 'c' [D:E:%d.4 'c']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ForOfStatement Scope %s.3
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            ArrayPattern : any
// CHECK-NEXT:                                Id 'a' [D:E:%d.6 'a'] : any
// CHECK-NEXT:                                Id 'b' [D:E:%d.7 'b'] : any
// CHECK-NEXT:                    Id 'c' [D:E:%d.4 'c'] : %class.2
// CHECK-NEXT:                    BlockStatement Scope %s.4
