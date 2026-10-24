/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals --typed --dump-sema %s | %FileCheckOrRegen %s --match-full-lines

'use strict';

// Object destructuring with defaults at a declaration: the bindings get the
// source property type, and the defaults are checked against it.
let obj: {x: number, y: string} = {x: 1, y: 'hello'};
let {x = 7, y = 'z'} = obj;
let xn: number = x;
let ys: string = y;

// Object-pattern parameter with a default on one property.
function f({a = 5, b}: {a: number, b: string}): void {
  let an: number = a;
  let bs: string = b;
}

// Indexer object: a defaulted binding gets the indexer value type.
function g({k = 'd'}: {+[key: string]: mixed}): mixed {
  return k;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%object.2 = object({
// CHECK-NEXT:  x: number
// CHECK-NEXT:  y: string
// CHECK-NEXT:})
// CHECK-NEXT:%object.3 = object({
// CHECK-NEXT:  a: number
// CHECK-NEXT:  b: string
// CHECK-NEXT:})
// CHECK-NEXT:%function.4 = function(%object.3): void
// CHECK-NEXT:%object.5 = object({
// CHECK-NEXT:  +[string]: mixed
// CHECK-NEXT:})
// CHECK-NEXT:%function.6 = function(%object.5): mixed

// CHECK:SemContext
// CHECK-NEXT:Func strict mayReachImplicitReturn
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'obj' Let : %object.2
// CHECK-NEXT:        Decl %d.3 'x' Let : number
// CHECK-NEXT:        Decl %d.4 'y' Let : string
// CHECK-NEXT:        Decl %d.5 'xn' Let : number
// CHECK-NEXT:        Decl %d.6 'ys' Let : string
// CHECK-NEXT:        Decl %d.7 'f' Var : %function.4
// CHECK-NEXT:        Decl %d.8 'g' Var : %function.6
// CHECK-NEXT:        Decl %d.9 'arguments' Var Arguments
// CHECK-NEXT:        hoistedFunction f
// CHECK-NEXT:        hoistedFunction g
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.10 'a' Parameter : number
// CHECK-NEXT:            Decl %d.11 'b' Parameter : string
// CHECK-NEXT:            Decl %d.12 'arguments' Var Arguments
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:            Scope %s.4
// CHECK-NEXT:                Decl %d.13 'an' Let : number
// CHECK-NEXT:                Decl %d.14 'bs' Let : string
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.5
// CHECK-NEXT:            Decl %d.15 'k' Parameter : mixed
// CHECK-NEXT:            Decl %d.16 'arguments' Var Arguments
// CHECK-NEXT:            Scope %s.6
// CHECK-NEXT:            Scope %s.7

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            StringLiteral : string
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                ObjectExpression : %object.2
// CHECK-NEXT:                    Property
// CHECK-NEXT:                        Id 'x'
// CHECK-NEXT:                        NumericLiteral : number
// CHECK-NEXT:                    Property
// CHECK-NEXT:                        Id 'y'
// CHECK-NEXT:                        StringLiteral : string
// CHECK-NEXT:                Id 'obj' [D:E:%d.2 'obj']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                Id 'obj' [D:E:%d.2 'obj'] : %object.2
// CHECK-NEXT:                ObjectPattern : %object.2
// CHECK-NEXT:                    Property
// CHECK-NEXT:                        Id 'x'
// CHECK-NEXT:                        AssignmentPattern
// CHECK-NEXT:                            Id 'x' [D:E:%d.3 'x'] : number
// CHECK-NEXT:                            NumericLiteral : number
// CHECK-NEXT:                    Property
// CHECK-NEXT:                        Id 'y'
// CHECK-NEXT:                        AssignmentPattern
// CHECK-NEXT:                            Id 'y' [D:E:%d.4 'y'] : string
// CHECK-NEXT:                            StringLiteral : string
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                Id 'x' [D:E:%d.3 'x'] : number
// CHECK-NEXT:                Id 'xn' [D:E:%d.5 'xn']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                Id 'y' [D:E:%d.4 'y'] : string
// CHECK-NEXT:                Id 'ys' [D:E:%d.6 'ys']
// CHECK-NEXT:        FunctionDeclaration : %function.4
// CHECK-NEXT:            Id 'f' [D:E:%d.7 'f']
// CHECK-NEXT:            ObjectPattern : %object.3
// CHECK-NEXT:                Property
// CHECK-NEXT:                    Id 'a'
// CHECK-NEXT:                    AssignmentPattern
// CHECK-NEXT:                        Id 'a' [D:E:%d.10 'a'] : number
// CHECK-NEXT:                        NumericLiteral : number
// CHECK-NEXT:                Property
// CHECK-NEXT:                    Id 'b'
// CHECK-NEXT:                    Id 'b' [D:E:%d.11 'b'] : string
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                VariableDeclaration
// CHECK-NEXT:                    VariableDeclarator
// CHECK-NEXT:                        Id 'a' [D:E:%d.10 'a'] : number
// CHECK-NEXT:                        Id 'an' [D:E:%d.13 'an']
// CHECK-NEXT:                VariableDeclaration
// CHECK-NEXT:                    VariableDeclarator
// CHECK-NEXT:                        Id 'b' [D:E:%d.11 'b'] : string
// CHECK-NEXT:                        Id 'bs' [D:E:%d.14 'bs']
// CHECK-NEXT:        FunctionDeclaration : %function.6
// CHECK-NEXT:            Id 'g' [D:E:%d.8 'g']
// CHECK-NEXT:            ObjectPattern : %object.5
// CHECK-NEXT:                Property
// CHECK-NEXT:                    Id 'k'
// CHECK-NEXT:                    AssignmentPattern
// CHECK-NEXT:                        Id 'k' [D:E:%d.15 'k'] : mixed
// CHECK-NEXT:                        StringLiteral : string
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    Id 'k' [D:E:%d.15 'k'] : mixed
