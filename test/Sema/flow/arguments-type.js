/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -fno-std-globals -typed -dump-sema %s | %FileCheckOrRegen %s --match-full-lines

// Test that arguments.length can be used in typed functions without warnings.

function foo(x: number, y: string): void {
  var len = arguments.length;
}

function bar(): void {
  var len = arguments.length;
}

function baz(a: number): bool {
  return arguments.length > 1;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%function.2 = function(x: number, y: string): void
// CHECK-NEXT:%function.3 = function(): void
// CHECK-NEXT:%function.4 = function(a: number): boolean

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'foo' Var : %function.2
// CHECK-NEXT:        Decl %d.3 'bar' Var : %function.3
// CHECK-NEXT:        Decl %d.4 'baz' Var : %function.4
// CHECK-NEXT:        Decl %d.5 'arguments' Var Arguments
// CHECK-NEXT:        hoistedFunction foo
// CHECK-NEXT:        hoistedFunction bar
// CHECK-NEXT:        hoistedFunction baz
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.6 'x' Parameter : number
// CHECK-NEXT:            Decl %d.7 'y' Parameter : string
// CHECK-NEXT:            Decl %d.8 'len' Var : any
// CHECK-NEXT:            Decl %d.9 'arguments' Var Arguments
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.3
// CHECK-NEXT:            Decl %d.10 'len' Var : any
// CHECK-NEXT:            Decl %d.11 'arguments' Var Arguments
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.4
// CHECK-NEXT:            Decl %d.12 'a' Parameter : number
// CHECK-NEXT:            Decl %d.13 'arguments' Var Arguments

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        FunctionDeclaration : %function.2
// CHECK-NEXT:            Id 'foo' [D:E:%d.2 'foo']
// CHECK-NEXT:            Id 'x' [D:E:%d.6 'x']
// CHECK-NEXT:            Id 'y' [D:E:%d.7 'y']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                VariableDeclaration
// CHECK-NEXT:                    VariableDeclarator
// CHECK-NEXT:                        MemberExpression : any
// CHECK-NEXT:                            Id 'arguments' [D:E:%d.9 'arguments'] : any
// CHECK-NEXT:                            Id 'length'
// CHECK-NEXT:                        Id 'len' [D:E:%d.8 'len']
// CHECK-NEXT:        FunctionDeclaration : %function.3
// CHECK-NEXT:            Id 'bar' [D:E:%d.3 'bar']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                VariableDeclaration
// CHECK-NEXT:                    VariableDeclarator
// CHECK-NEXT:                        MemberExpression : any
// CHECK-NEXT:                            Id 'arguments' [D:E:%d.11 'arguments'] : any
// CHECK-NEXT:                            Id 'length'
// CHECK-NEXT:                        Id 'len' [D:E:%d.10 'len']
// CHECK-NEXT:        FunctionDeclaration : %function.4
// CHECK-NEXT:            Id 'baz' [D:E:%d.4 'baz']
// CHECK-NEXT:            Id 'a' [D:E:%d.12 'a']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    BinaryExpression : boolean
// CHECK-NEXT:                        MemberExpression : any
// CHECK-NEXT:                            Id 'arguments' [D:E:%d.13 'arguments'] : any
// CHECK-NEXT:                            Id 'length'
// CHECK-NEXT:                        NumericLiteral : number
