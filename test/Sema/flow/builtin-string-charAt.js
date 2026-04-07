/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -dump-sema %s | %FileCheckOrRegen %s --match-full-lines

// Test that string.charAt builtin method is correctly typechecked.

function test(s: string): string {
  return s.charAt(0);
}

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%function.2 = function(s: string): string
// CHECK-NEXT:%function.3 = function(this: string, pos: number): string

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'test' Var : %function.2
// CHECK-NEXT:        Decl %d.3 'arguments' Var Arguments
// CHECK-NEXT:        hoistedFunction test
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.4 's' Parameter : string
// CHECK-NEXT:            Decl %d.5 'arguments' Var Arguments

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        FunctionDeclaration : %function.2
// CHECK-NEXT:            Id 'test' [D:E:%d.2 'test']
// CHECK-NEXT:            Id 's' [D:E:%d.4 's']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    CallExpression : string
// CHECK-NEXT:                        MemberExpression : %function.3
// CHECK-NEXT:                            Id 's' [D:E:%d.4 's'] : string
// CHECK-NEXT:                            Id 'charAt'
// CHECK-NEXT:                        NumericLiteral : number
