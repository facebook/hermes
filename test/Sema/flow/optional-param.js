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

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%function.2 = function(x: number, y: number): void
// CHECK-NEXT:%function.3 = function(): void
// CHECK-NEXT:%union.4 = union(void | number)
// CHECK-NEXT:%function.5 = function(x: number): void
// CHECK-NEXT:%object.6 = object({
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'decl' Var : %function.2
// CHECK-NEXT:            Decl %d.3 'addOptional' Var : %function.3
// CHECK-NEXT:            Decl %d.4 'removeOptional' Var : %function.3
// CHECK-NEXT:            Decl %d.5 'arguments' Var Arguments
// CHECK-NEXT:            hoistedFunction decl
// CHECK-NEXT:            hoistedFunction addOptional
// CHECK-NEXT:            hoistedFunction removeOptional
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:                Decl %d.6 'x' Parameter : number
// CHECK-NEXT:                Decl %d.7 'y' Parameter : %union.4
// CHECK-NEXT:                Decl %d.8 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.4
// CHECK-NEXT:                Decl %d.9 'a' Let : %function.5
// CHECK-NEXT:                Decl %d.10 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.5
// CHECK-NEXT:                Decl %d.11 'b' Let : %function.2
// CHECK-NEXT:                Decl %d.12 'c' Let : %function.5
// CHECK-NEXT:                Decl %d.13 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    FunctionDeclaration : %function.2
// CHECK-NEXT:                        Id 'decl' [D:E:%d.2 'decl']
// CHECK-NEXT:                        Id 'x' [D:E:%d.6 'x']
// CHECK-NEXT:                        Id 'y' [D:E:%d.7 'y']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                    TypeAlias
// CHECK-NEXT:                        Id 'F'
// CHECK-NEXT:                        FunctionTypeAnnotation
// CHECK-NEXT:                            FunctionTypeParam
// CHECK-NEXT:                                Id 'x'
// CHECK-NEXT:                                NumberTypeAnnotation
// CHECK-NEXT:                            FunctionTypeParam
// CHECK-NEXT:                                Id 'y'
// CHECK-NEXT:                                NumberTypeAnnotation
// CHECK-NEXT:                            VoidTypeAnnotation
// CHECK-NEXT:                    FunctionDeclaration : %function.3
// CHECK-NEXT:                        Id 'addOptional' [D:E:%d.3 'addOptional']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            VariableDeclaration
// CHECK-NEXT:                                VariableDeclarator
// CHECK-NEXT:                                    ImplicitCheckedCast : %function.5
// CHECK-NEXT:                                        Id 'decl' [D:E:%d.2 'decl'] : %function.2
// CHECK-NEXT:                                    Id 'a' [D:E:%d.9 'a']
// CHECK-NEXT:                    FunctionDeclaration : %function.3
// CHECK-NEXT:                        Id 'removeOptional' [D:E:%d.4 'removeOptional']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            VariableDeclaration
// CHECK-NEXT:                                VariableDeclarator
// CHECK-NEXT:                                    Id 'decl' [D:E:%d.2 'decl'] : %function.2
// CHECK-NEXT:                                    Id 'b' [D:E:%d.11 'b']
// CHECK-NEXT:                            VariableDeclaration
// CHECK-NEXT:                                VariableDeclarator
// CHECK-NEXT:                                    ImplicitCheckedCast : %function.5
// CHECK-NEXT:                                        Id 'b' [D:E:%d.11 'b'] : %function.2
// CHECK-NEXT:                                    Id 'c' [D:E:%d.12 'c']
// CHECK-NEXT:            ObjectExpression : %object.6
