/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -typed -dump-sema -fno-std-globals %s | %FileCheckOrRegen %s --match-full-lines

// Optional param in function declaration.
function decl(x: number, y?: number): void {}

// Optional param in function type annotation.
type F = (x: number, y?: number) => void;

// Flow: add optional param (fewer params -> more params with optional).
function addOptional(): void {
  let a: (x: number) => void = decl;
}

// Flow: remove optional param (more params with optional -> fewer params).
function removeOptional(): void {
  let b: (x: number, y?: number) => void = decl;
  let c: (x: number) => void = b;
}

// Flow: fewer params into more non-optional params (checked cast).
function fewerToMore(cb: (x: number) => void): void {
  cb(1);
}
function testFewerToMore(): void {
  fewerToMore(() => {});
}

// Flow: fewer params into more non-optional params via assignment.
function assignFewerToMore(): void {
  let f: (x: number, y: string) => void = (x: number) => {};
}

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%function.2 = function(x: number, y: number): void
// CHECK-NEXT:%function.3 = function(): void
// CHECK-NEXT:%function.4 = function(x: number): void
// CHECK-NEXT:%function.5 = function(cb: %function.4): void
// CHECK-NEXT:%union.6 = union(void | number)
// CHECK-NEXT:%function.7 = function(x: number, y: string): void

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'decl' Var : %function.2
// CHECK-NEXT:        Decl %d.3 'addOptional' Var : %function.3
// CHECK-NEXT:        Decl %d.4 'removeOptional' Var : %function.3
// CHECK-NEXT:        Decl %d.5 'fewerToMore' Var : %function.5
// CHECK-NEXT:        Decl %d.6 'testFewerToMore' Var : %function.3
// CHECK-NEXT:        Decl %d.7 'assignFewerToMore' Var : %function.3
// CHECK-NEXT:        Decl %d.8 'arguments' Var Arguments
// CHECK-NEXT:        hoistedFunction decl
// CHECK-NEXT:        hoistedFunction addOptional
// CHECK-NEXT:        hoistedFunction removeOptional
// CHECK-NEXT:        hoistedFunction fewerToMore
// CHECK-NEXT:        hoistedFunction testFewerToMore
// CHECK-NEXT:        hoistedFunction assignFewerToMore
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.9 'x' Parameter : number
// CHECK-NEXT:            Decl %d.10 'y' Parameter : %union.6
// CHECK-NEXT:            Decl %d.11 'arguments' Var Arguments
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.3
// CHECK-NEXT:            Decl %d.12 'a' Let : %function.4
// CHECK-NEXT:            Decl %d.13 'arguments' Var Arguments
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.4
// CHECK-NEXT:            Decl %d.14 'b' Let : %function.2
// CHECK-NEXT:            Decl %d.15 'c' Let : %function.4
// CHECK-NEXT:            Decl %d.16 'arguments' Var Arguments
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.5
// CHECK-NEXT:            Decl %d.17 'cb' Parameter : %function.4
// CHECK-NEXT:            Decl %d.18 'arguments' Var Arguments
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.6
// CHECK-NEXT:            Decl %d.19 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.7
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.8
// CHECK-NEXT:            Decl %d.20 'f' Let : %function.7
// CHECK-NEXT:            Decl %d.21 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.9
// CHECK-NEXT:                Decl %d.22 'x' Parameter : number

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        FunctionDeclaration : %function.2
// CHECK-NEXT:            Id 'decl' [D:E:%d.2 'decl']
// CHECK-NEXT:            Id 'x' [D:E:%d.9 'x']
// CHECK-NEXT:            Id 'y' [D:E:%d.10 'y']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'F'
// CHECK-NEXT:            FunctionTypeAnnotation
// CHECK-NEXT:                FunctionTypeParam
// CHECK-NEXT:                    Id 'x'
// CHECK-NEXT:                    NumberTypeAnnotation
// CHECK-NEXT:                FunctionTypeParam
// CHECK-NEXT:                    Id 'y'
// CHECK-NEXT:                    NumberTypeAnnotation
// CHECK-NEXT:                VoidTypeAnnotation
// CHECK-NEXT:        FunctionDeclaration : %function.3
// CHECK-NEXT:            Id 'addOptional' [D:E:%d.3 'addOptional']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                VariableDeclaration
// CHECK-NEXT:                    VariableDeclarator
// CHECK-NEXT:                        ImplicitCheckedCast : %function.4
// CHECK-NEXT:                            Id 'decl' [D:E:%d.2 'decl'] : %function.2
// CHECK-NEXT:                        Id 'a' [D:E:%d.12 'a']
// CHECK-NEXT:        FunctionDeclaration : %function.3
// CHECK-NEXT:            Id 'removeOptional' [D:E:%d.4 'removeOptional']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                VariableDeclaration
// CHECK-NEXT:                    VariableDeclarator
// CHECK-NEXT:                        Id 'decl' [D:E:%d.2 'decl'] : %function.2
// CHECK-NEXT:                        Id 'b' [D:E:%d.14 'b']
// CHECK-NEXT:                VariableDeclaration
// CHECK-NEXT:                    VariableDeclarator
// CHECK-NEXT:                        ImplicitCheckedCast : %function.4
// CHECK-NEXT:                            Id 'b' [D:E:%d.14 'b'] : %function.2
// CHECK-NEXT:                        Id 'c' [D:E:%d.15 'c']
// CHECK-NEXT:        FunctionDeclaration : %function.5
// CHECK-NEXT:            Id 'fewerToMore' [D:E:%d.5 'fewerToMore']
// CHECK-NEXT:            Id 'cb' [D:E:%d.17 'cb']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    CallExpression : void
// CHECK-NEXT:                        Id 'cb' [D:E:%d.17 'cb'] : %function.4
// CHECK-NEXT:                        NumericLiteral : number
// CHECK-NEXT:        FunctionDeclaration : %function.3
// CHECK-NEXT:            Id 'testFewerToMore' [D:E:%d.6 'testFewerToMore']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    CallExpression : void
// CHECK-NEXT:                        Id 'fewerToMore' [D:E:%d.5 'fewerToMore'] : %function.5
// CHECK-NEXT:                        ImplicitCheckedCast : %function.4
// CHECK-NEXT:                            ArrowFunctionExpression : %function.3
// CHECK-NEXT:                                BlockStatement
// CHECK-NEXT:        FunctionDeclaration : %function.3
// CHECK-NEXT:            Id 'assignFewerToMore' [D:E:%d.7 'assignFewerToMore']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                VariableDeclaration
// CHECK-NEXT:                    VariableDeclarator
// CHECK-NEXT:                        ImplicitCheckedCast : %function.7
// CHECK-NEXT:                            ArrowFunctionExpression : %function.4
// CHECK-NEXT:                                Id 'x' [D:E:%d.22 'x']
// CHECK-NEXT:                                BlockStatement
// CHECK-NEXT:                        Id 'f' [D:E:%d.20 'f']
