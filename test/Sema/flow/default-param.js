/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -typed -dump-sema -fno-std-globals %s | %FileCheckOrRegen %s --match-full-lines

// Basic default param.
function basic(x: number = 1): void {}

// Default param with return type using the param.
function useParam(x: number = 1): number { return x; }

// Flow: function with default param flows into type with fewer params.
function flowFewer(): void {
  let f: (a: number) => void = basic;
}

// Flow: function with default param flows into type with optional param.
function flowOptional(): void {
  let f: (a: number, b?: number) => void =
    function(a: number, b: number = 0): void {};
}

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%function.2 = function(x: number): void
// CHECK-NEXT:%function.3 = function(x: number): number
// CHECK-NEXT:%function.4 = function(): void
// CHECK-NEXT:%function.5 = function(a: number, b: number): void

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'basic' Var : %function.2
// CHECK-NEXT:        Decl %d.3 'useParam' Var : %function.3
// CHECK-NEXT:        Decl %d.4 'flowFewer' Var : %function.4
// CHECK-NEXT:        Decl %d.5 'flowOptional' Var : %function.4
// CHECK-NEXT:        Decl %d.6 'arguments' Var Arguments
// CHECK-NEXT:        hoistedFunction basic
// CHECK-NEXT:        hoistedFunction useParam
// CHECK-NEXT:        hoistedFunction flowFewer
// CHECK-NEXT:        hoistedFunction flowOptional
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.7 'x' Parameter : number
// CHECK-NEXT:            Decl %d.8 'arguments' Var Arguments
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:            Scope %s.4
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.5
// CHECK-NEXT:            Decl %d.9 'x' Parameter : number
// CHECK-NEXT:            Decl %d.10 'arguments' Var Arguments
// CHECK-NEXT:            Scope %s.6
// CHECK-NEXT:            Scope %s.7
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.8
// CHECK-NEXT:            Decl %d.11 'f' Let : %function.2
// CHECK-NEXT:            Decl %d.12 'arguments' Var Arguments
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.9
// CHECK-NEXT:            Decl %d.13 'f' Let : %function.5
// CHECK-NEXT:            Decl %d.14 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.10
// CHECK-NEXT:                Decl %d.15 'a' Parameter : number
// CHECK-NEXT:                Decl %d.16 'b' Parameter : number
// CHECK-NEXT:                Decl %d.17 'arguments' Var Arguments
// CHECK-NEXT:                Scope %s.11
// CHECK-NEXT:                Scope %s.12

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        FunctionDeclaration : %function.2
// CHECK-NEXT:            Id 'basic' [D:E:%d.2 'basic']
// CHECK-NEXT:            AssignmentPattern
// CHECK-NEXT:                Id 'x' [D:E:%d.7 'x']
// CHECK-NEXT:                NumericLiteral : number
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:        FunctionDeclaration : %function.3
// CHECK-NEXT:            Id 'useParam' [D:E:%d.3 'useParam']
// CHECK-NEXT:            AssignmentPattern
// CHECK-NEXT:                Id 'x' [D:E:%d.9 'x']
// CHECK-NEXT:                NumericLiteral : number
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    Id 'x' [D:E:%d.9 'x'] : number
// CHECK-NEXT:        FunctionDeclaration : %function.4
// CHECK-NEXT:            Id 'flowFewer' [D:E:%d.4 'flowFewer']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                VariableDeclaration
// CHECK-NEXT:                    VariableDeclarator
// CHECK-NEXT:                        Id 'basic' [D:E:%d.2 'basic'] : %function.2
// CHECK-NEXT:                        Id 'f' [D:E:%d.11 'f']
// CHECK-NEXT:        FunctionDeclaration : %function.4
// CHECK-NEXT:            Id 'flowOptional' [D:E:%d.5 'flowOptional']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                VariableDeclaration
// CHECK-NEXT:                    VariableDeclarator
// CHECK-NEXT:                        FunctionExpression : %function.5
// CHECK-NEXT:                            Id 'a' [D:E:%d.15 'a']
// CHECK-NEXT:                            AssignmentPattern
// CHECK-NEXT:                                Id 'b' [D:E:%d.16 'b']
// CHECK-NEXT:                                NumericLiteral : number
// CHECK-NEXT:                            BlockStatement
// CHECK-NEXT:                        Id 'f' [D:E:%d.13 'f']
