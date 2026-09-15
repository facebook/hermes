/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -typed -dump-sema -fno-std-globals %s | %FileCheckOrRegen %s --match-full-lines

// Param typed `void` is callable with no argument.
function f1(x: void): void {}
f1();

// Param typed `string | void` is callable with no argument or with a string.
function f2(x: string | void): void {}
f2();
f2('a');

// Param typed `?string` (== string | null | void) is callable with no
// argument, with null, or with a string.
function f3(x: ?string): void {}
f3();
f3(null);
f3('a');

// Trailing void-typed param is omittable; non-void leading param is still
// required.
function f4(x: number, y: void): void {}
f4(1);

// `mixed` and `any` accept void, so they are also omittable.
function f5(x: mixed): void {}
f5();
function f6(x: any): void {}
f6();

// Generic placeholder: omitting the arg infers T = void.
function fg<T>(x: T): T {
  return x;
}
let v: void = fg();
let s: string = fg('hi');

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%function.2 = function(x: void): void
// CHECK-NEXT:%union.3 = union(void | string)
// CHECK-NEXT:%function.4 = function(x: %union.3): void
// CHECK-NEXT:%union.5 = union(void | null | string)
// CHECK-NEXT:%function.6 = function(x: %union.5): void
// CHECK-NEXT:%function.7 = function(x: number, y: void): void
// CHECK-NEXT:%function.8 = function(x: mixed): void
// CHECK-NEXT:%function.9 = function(x: any): void
// CHECK-NEXT:%function.10 = function(x: string): string

// CHECK:SemContext
// CHECK-NEXT:Func strict mayReachImplicitReturn
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'f1' Var : %function.2
// CHECK-NEXT:        Decl %d.3 'f2' Var : %function.4
// CHECK-NEXT:        Decl %d.4 'f3' Var : %function.6
// CHECK-NEXT:        Decl %d.5 'f4' Var : %function.7
// CHECK-NEXT:        Decl %d.6 'f5' Var : %function.8
// CHECK-NEXT:        Decl %d.7 'f6' Var : %function.9
// CHECK-NEXT:        Decl %d.8 'fg' Var
// CHECK-NEXT:        Decl %d.9 'v' Let : void
// CHECK-NEXT:        Decl %d.10 's' Let : string
// CHECK-NEXT:        Decl %d.11 'arguments' Var Arguments
// CHECK-NEXT:        Decl %d.12 'fg' Var : %function.2
// CHECK-NEXT:        Decl %d.13 'fg' Var : %function.10
// CHECK-NEXT:        hoistedFunction f1
// CHECK-NEXT:        hoistedFunction f2
// CHECK-NEXT:        hoistedFunction f3
// CHECK-NEXT:        hoistedFunction f4
// CHECK-NEXT:        hoistedFunction f5
// CHECK-NEXT:        hoistedFunction f6
// CHECK-NEXT:        hoistedFunction fg
// CHECK-NEXT:        hoistedFunction fg
// CHECK-NEXT:        hoistedFunction fg
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.14 'x' Parameter : void
// CHECK-NEXT:            Decl %d.15 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.3
// CHECK-NEXT:            Decl %d.16 'x' Parameter : %union.3
// CHECK-NEXT:            Decl %d.17 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.4
// CHECK-NEXT:            Decl %d.18 'x' Parameter : %union.5
// CHECK-NEXT:            Decl %d.19 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.5
// CHECK-NEXT:            Decl %d.20 'x' Parameter : number
// CHECK-NEXT:            Decl %d.21 'y' Parameter : void
// CHECK-NEXT:            Decl %d.22 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.6
// CHECK-NEXT:            Decl %d.23 'x' Parameter : mixed
// CHECK-NEXT:            Decl %d.24 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.7
// CHECK-NEXT:            Decl %d.25 'x' Parameter : any
// CHECK-NEXT:            Decl %d.26 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.8
// CHECK-NEXT:            Decl %d.27 'x' Parameter
// CHECK-NEXT:            Decl %d.28 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.9
// CHECK-NEXT:            Decl %d.29 'x' Parameter : void
// CHECK-NEXT:            Decl %d.30 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.10
// CHECK-NEXT:            Decl %d.31 'x' Parameter : string
// CHECK-NEXT:            Decl %d.32 'arguments' Var Arguments

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        FunctionDeclaration : %function.2
// CHECK-NEXT:            Id 'f1' [D:E:%d.2 'f1']
// CHECK-NEXT:            Id 'x' [D:E:%d.14 'x']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            CallExpression : void
// CHECK-NEXT:                Id 'f1' [D:E:%d.2 'f1'] : %function.2
// CHECK-NEXT:        FunctionDeclaration : %function.4
// CHECK-NEXT:            Id 'f2' [D:E:%d.3 'f2']
// CHECK-NEXT:            Id 'x' [D:E:%d.16 'x']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            CallExpression : void
// CHECK-NEXT:                Id 'f2' [D:E:%d.3 'f2'] : %function.4
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            CallExpression : void
// CHECK-NEXT:                Id 'f2' [D:E:%d.3 'f2'] : %function.4
// CHECK-NEXT:                StringLiteral : string
// CHECK-NEXT:        FunctionDeclaration : %function.6
// CHECK-NEXT:            Id 'f3' [D:E:%d.4 'f3']
// CHECK-NEXT:            Id 'x' [D:E:%d.18 'x']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            CallExpression : void
// CHECK-NEXT:                Id 'f3' [D:E:%d.4 'f3'] : %function.6
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            CallExpression : void
// CHECK-NEXT:                Id 'f3' [D:E:%d.4 'f3'] : %function.6
// CHECK-NEXT:                NullLiteral : null
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            CallExpression : void
// CHECK-NEXT:                Id 'f3' [D:E:%d.4 'f3'] : %function.6
// CHECK-NEXT:                StringLiteral : string
// CHECK-NEXT:        FunctionDeclaration : %function.7
// CHECK-NEXT:            Id 'f4' [D:E:%d.5 'f4']
// CHECK-NEXT:            Id 'x' [D:E:%d.20 'x']
// CHECK-NEXT:            Id 'y' [D:E:%d.21 'y']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            CallExpression : void
// CHECK-NEXT:                Id 'f4' [D:E:%d.5 'f4'] : %function.7
// CHECK-NEXT:                NumericLiteral : number
// CHECK-NEXT:        FunctionDeclaration : %function.8
// CHECK-NEXT:            Id 'f5' [D:E:%d.6 'f5']
// CHECK-NEXT:            Id 'x' [D:E:%d.23 'x']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            CallExpression : void
// CHECK-NEXT:                Id 'f5' [D:E:%d.6 'f5'] : %function.8
// CHECK-NEXT:        FunctionDeclaration : %function.9
// CHECK-NEXT:            Id 'f6' [D:E:%d.7 'f6']
// CHECK-NEXT:            Id 'x' [D:E:%d.25 'x']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            CallExpression : void
// CHECK-NEXT:                Id 'f6' [D:E:%d.7 'f6'] : %function.9
// CHECK-NEXT:        FunctionDeclaration : %function.2
// CHECK-NEXT:            Id 'fg' [D:E:%d.12 'fg']
// CHECK-NEXT:            Id 'x' [D:E:%d.29 'x']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    Id 'x' [D:E:%d.29 'x'] : void
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:        FunctionDeclaration : %function.10
// CHECK-NEXT:            Id 'fg' [D:E:%d.13 'fg']
// CHECK-NEXT:            Id 'x' [D:E:%d.31 'x']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    Id 'x' [D:E:%d.31 'x'] : string
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:        FunctionDeclaration
// CHECK-NEXT:            Id 'fg' [D:E:%d.8 'fg']
// CHECK-NEXT:            Id 'x' [D:E:%d.27 'x']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    Id 'x' [D:E:%d.27 'x']
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                CallExpression : void
// CHECK-NEXT:                    Id 'fg' [D:E:%d.12 'fg'] : %function.2
// CHECK-NEXT:                Id 'v' [D:E:%d.9 'v']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                CallExpression : string
// CHECK-NEXT:                    Id 'fg' [D:E:%d.13 'fg'] : %function.10
// CHECK-NEXT:                    StringLiteral : string
// CHECK-NEXT:                Id 's' [D:E:%d.10 's']
