/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals --typed --dump-sema %s | %FileCheckOrRegen %s --match-full-lines

'use strict';

// Object destructuring param.
function foo({x, y}: {x: number, y: string}): void {
  let a: number = x;
  let b: string = y;
}

// Array destructuring param (tuple).
function bar([a, b]: [number, string]): void {
  let c: number = a;
  let d: string = b;
}

// Nested destructuring param.
function baz({a: {b}}: {a: {b: number}}): void {
  let c: number = b;
}

// Destructuring with default value.
function qux({x}: {x: number} = {x: 0}): void {
  let a: number = x;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%object.2 = object({
// CHECK-NEXT:  x: number
// CHECK-NEXT:  y: string
// CHECK-NEXT:})
// CHECK-NEXT:%function.3 = function(%object.2): void
// CHECK-NEXT:%tuple.4 = tuple(number, string)
// CHECK-NEXT:%function.5 = function(%tuple.4): void
// CHECK-NEXT:%object.6 = object({
// CHECK-NEXT:  b: number
// CHECK-NEXT:})
// CHECK-NEXT:%object.7 = object({
// CHECK-NEXT:  a: %object.6
// CHECK-NEXT:})
// CHECK-NEXT:%function.8 = function(%object.7): void
// CHECK-NEXT:%object.9 = object({
// CHECK-NEXT:  x: number
// CHECK-NEXT:})
// CHECK-NEXT:%function.10 = function(%object.9): void

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'foo' Var : %function.3
// CHECK-NEXT:        Decl %d.3 'bar' Var : %function.5
// CHECK-NEXT:        Decl %d.4 'baz' Var : %function.8
// CHECK-NEXT:        Decl %d.5 'qux' Var : %function.10
// CHECK-NEXT:        Decl %d.6 'arguments' Var Arguments
// CHECK-NEXT:        hoistedFunction foo
// CHECK-NEXT:        hoistedFunction bar
// CHECK-NEXT:        hoistedFunction baz
// CHECK-NEXT:        hoistedFunction qux
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.7 'x' Parameter : number
// CHECK-NEXT:            Decl %d.8 'y' Parameter : string
// CHECK-NEXT:            Decl %d.9 'a' Let : number
// CHECK-NEXT:            Decl %d.10 'b' Let : string
// CHECK-NEXT:            Decl %d.11 'arguments' Var Arguments
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.3
// CHECK-NEXT:            Decl %d.12 'a' Parameter : number
// CHECK-NEXT:            Decl %d.13 'b' Parameter : string
// CHECK-NEXT:            Decl %d.14 'c' Let : number
// CHECK-NEXT:            Decl %d.15 'd' Let : string
// CHECK-NEXT:            Decl %d.16 'arguments' Var Arguments
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.4
// CHECK-NEXT:            Decl %d.17 'b' Parameter : number
// CHECK-NEXT:            Decl %d.18 'c' Let : number
// CHECK-NEXT:            Decl %d.19 'arguments' Var Arguments
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.5
// CHECK-NEXT:            Decl %d.20 'x' Parameter : number
// CHECK-NEXT:            Decl %d.21 'arguments' Var Arguments
// CHECK-NEXT:            Scope %s.6
// CHECK-NEXT:            Scope %s.7
// CHECK-NEXT:                Decl %d.22 'a' Let : number

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            StringLiteral : string
// CHECK-NEXT:        FunctionDeclaration : %function.3
// CHECK-NEXT:            Id 'foo' [D:E:%d.2 'foo']
// CHECK-NEXT:            ObjectPattern : %object.2
// CHECK-NEXT:                Property
// CHECK-NEXT:                    Id 'x'
// CHECK-NEXT:                    Id 'x' [D:E:%d.7 'x'] : number
// CHECK-NEXT:                Property
// CHECK-NEXT:                    Id 'y'
// CHECK-NEXT:                    Id 'y' [D:E:%d.8 'y'] : string
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                VariableDeclaration
// CHECK-NEXT:                    VariableDeclarator
// CHECK-NEXT:                        Id 'x' [D:E:%d.7 'x'] : number
// CHECK-NEXT:                        Id 'a' [D:E:%d.9 'a']
// CHECK-NEXT:                VariableDeclaration
// CHECK-NEXT:                    VariableDeclarator
// CHECK-NEXT:                        Id 'y' [D:E:%d.8 'y'] : string
// CHECK-NEXT:                        Id 'b' [D:E:%d.10 'b']
// CHECK-NEXT:        FunctionDeclaration : %function.5
// CHECK-NEXT:            Id 'bar' [D:E:%d.3 'bar']
// CHECK-NEXT:            ArrayPattern : %tuple.4
// CHECK-NEXT:                Id 'a' [D:E:%d.12 'a'] : number
// CHECK-NEXT:                Id 'b' [D:E:%d.13 'b'] : string
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                VariableDeclaration
// CHECK-NEXT:                    VariableDeclarator
// CHECK-NEXT:                        Id 'a' [D:E:%d.12 'a'] : number
// CHECK-NEXT:                        Id 'c' [D:E:%d.14 'c']
// CHECK-NEXT:                VariableDeclaration
// CHECK-NEXT:                    VariableDeclarator
// CHECK-NEXT:                        Id 'b' [D:E:%d.13 'b'] : string
// CHECK-NEXT:                        Id 'd' [D:E:%d.15 'd']
// CHECK-NEXT:        FunctionDeclaration : %function.8
// CHECK-NEXT:            Id 'baz' [D:E:%d.4 'baz']
// CHECK-NEXT:            ObjectPattern : %object.7
// CHECK-NEXT:                Property
// CHECK-NEXT:                    Id 'a'
// CHECK-NEXT:                    ObjectPattern : %object.6
// CHECK-NEXT:                        Property
// CHECK-NEXT:                            Id 'b'
// CHECK-NEXT:                            Id 'b' [D:E:%d.17 'b'] : number
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                VariableDeclaration
// CHECK-NEXT:                    VariableDeclarator
// CHECK-NEXT:                        Id 'b' [D:E:%d.17 'b'] : number
// CHECK-NEXT:                        Id 'c' [D:E:%d.18 'c']
// CHECK-NEXT:        FunctionDeclaration : %function.10
// CHECK-NEXT:            Id 'qux' [D:E:%d.5 'qux']
// CHECK-NEXT:            AssignmentPattern
// CHECK-NEXT:                ObjectPattern : %object.9
// CHECK-NEXT:                    Property
// CHECK-NEXT:                        Id 'x'
// CHECK-NEXT:                        Id 'x' [D:E:%d.20 'x'] : number
// CHECK-NEXT:                ObjectExpression : %object.9
// CHECK-NEXT:                    Property
// CHECK-NEXT:                        Id 'x'
// CHECK-NEXT:                        NumericLiteral : number
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                VariableDeclaration
// CHECK-NEXT:                    VariableDeclarator
// CHECK-NEXT:                        Id 'x' [D:E:%d.20 'x'] : number
// CHECK-NEXT:                        Id 'a' [D:E:%d.22 'a']
