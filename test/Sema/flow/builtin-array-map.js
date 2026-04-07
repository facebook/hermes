/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -dump-sema %s | %FileCheckOrRegen %s --match-full-lines

// Test that array.map builtin method is correctly typechecked.

function test(arr: number[]): number[] {
  return arr.map((n: number, i: number, a: number[]): number => n + 1);
}

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%class.2 = class(Array {
// CHECK-NEXT:  %homeObject: %class.6
// CHECK-NEXT:})
// CHECK-NEXT:%function.3 = function(arr: %class.2): %class.2
// CHECK-NEXT:%function.4 = function(n: number, i: number, a: %class.2): number
// CHECK-NEXT:%function.5 = function(this: %class.2, callback: %function.4): %class.2
// CHECK-NEXT:%class.6 = class( {
// CHECK-NEXT:  map [final]: generic
// CHECK-NEXT:  toString [final]: %function.7
// CHECK-NEXT:})
// CHECK-NEXT:%function.7 = function(this: %class.2): string

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'test' Var : %function.3
// CHECK-NEXT:        Decl %d.3 'arguments' Var Arguments
// CHECK-NEXT:        hoistedFunction test
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.4 'arr' Parameter : %class.2
// CHECK-NEXT:            Decl %d.5 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:                Decl %d.6 'n' Parameter : number
// CHECK-NEXT:                Decl %d.7 'i' Parameter : number
// CHECK-NEXT:                Decl %d.8 'a' Parameter : %class.2

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        FunctionDeclaration : %function.3
// CHECK-NEXT:            Id 'test' [D:E:%d.2 'test']
// CHECK-NEXT:            Id 'arr' [D:E:%d.4 'arr']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    CallExpression : %class.2
// CHECK-NEXT:                        MemberExpression : %function.5
// CHECK-NEXT:                            Id 'arr' [D:E:%d.4 'arr'] : %class.2
// CHECK-NEXT:                            Id 'map' [D:E:%d.9 'map']
// CHECK-NEXT:                        ArrowFunctionExpression : %function.4
// CHECK-NEXT:                            Id 'n' [D:E:%d.6 'n']
// CHECK-NEXT:                            Id 'i' [D:E:%d.7 'i']
// CHECK-NEXT:                            Id 'a' [D:E:%d.8 'a']
// CHECK-NEXT:                            BlockStatement
// CHECK-NEXT:                                ReturnStatement
// CHECK-NEXT:                                    BinaryExpression : number
// CHECK-NEXT:                                        Id 'n' [D:E:%d.6 'n'] : number
// CHECK-NEXT:                                        BinOp +
// CHECK-NEXT:                                        NumericLiteral : number
