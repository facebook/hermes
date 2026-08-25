/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -fno-std-globals -typed -dump-sema %s | %FileCheckOrRegen %s --match-full-lines

// A computed property produces an indexer object with a string key.
function f1(k: string) {
  return {[k]: 1};
}

// Multiple computed properties: the value type is the union of all the
// computed value types.
function f2(k1: string, k2: string) {
  return {[k1]: 1, [k2]: "s"};
}

// The computed value is checked against a target indexer's value type.
function f3(k: string): {[string]: number} {
  return {[k]: 2};
}

// A computed property combined with an indexer spread folds into one indexer.
function f4(o: {[string]: number}, k: string) {
  return {...o, [k]: 2};
}

// A non-string computed key contributes its type to the indexer key, unioned
// with string (computed keys coerce to string at runtime).
function f5(n: number) {
  return {[n]: 1};
}

// A number-keyed spread indexer combined with a computed key widens the key
// type to the union of both, instead of erroring.
function f6(o: {[number]: string}, k: string) {
  return {...o, [k]: 2};
}

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%function.2 = function(k: string): any
// CHECK-NEXT:%function.3 = function(k1: string, k2: string): any
// CHECK-NEXT:%function.4 = function(k: string): %object.5
// CHECK-NEXT:%object.5 = object({
// CHECK-NEXT:  [string]: number
// CHECK-NEXT:})
// CHECK-NEXT:%function.6 = function(o: %object.5, k: string): any
// CHECK-NEXT:%function.7 = function(n: number): any
// CHECK-NEXT:%object.8 = object({
// CHECK-NEXT:  [number]: string
// CHECK-NEXT:})
// CHECK-NEXT:%function.9 = function(o: %object.8, k: string): any
// CHECK-NEXT:%object.10 = object({
// CHECK-NEXT:  [string]: %union.13
// CHECK-NEXT:})
// CHECK-NEXT:%object.11 = object({
// CHECK-NEXT:  [number]: number
// CHECK-NEXT:})
// CHECK-NEXT:%object.12 = object({
// CHECK-NEXT:  [%union.14]: %union.14
// CHECK-NEXT:})
// CHECK-NEXT:%union.13 = union(number | "s")
// CHECK-NEXT:%union.14 = union(string | number)

// CHECK:SemContext
// CHECK-NEXT:Func strict mayReachImplicitReturn
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'f1' Var : %function.2
// CHECK-NEXT:        Decl %d.3 'f2' Var : %function.3
// CHECK-NEXT:        Decl %d.4 'f3' Var : %function.4
// CHECK-NEXT:        Decl %d.5 'f4' Var : %function.6
// CHECK-NEXT:        Decl %d.6 'f5' Var : %function.7
// CHECK-NEXT:        Decl %d.7 'f6' Var : %function.9
// CHECK-NEXT:        Decl %d.8 'arguments' Var Arguments
// CHECK-NEXT:        hoistedFunction f1
// CHECK-NEXT:        hoistedFunction f2
// CHECK-NEXT:        hoistedFunction f3
// CHECK-NEXT:        hoistedFunction f4
// CHECK-NEXT:        hoistedFunction f5
// CHECK-NEXT:        hoistedFunction f6
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.9 'k' Parameter : string
// CHECK-NEXT:            Decl %d.10 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.3
// CHECK-NEXT:            Decl %d.11 'k1' Parameter : string
// CHECK-NEXT:            Decl %d.12 'k2' Parameter : string
// CHECK-NEXT:            Decl %d.13 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.4
// CHECK-NEXT:            Decl %d.14 'k' Parameter : string
// CHECK-NEXT:            Decl %d.15 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.5
// CHECK-NEXT:            Decl %d.16 'o' Parameter : %object.5
// CHECK-NEXT:            Decl %d.17 'k' Parameter : string
// CHECK-NEXT:            Decl %d.18 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.6
// CHECK-NEXT:            Decl %d.19 'n' Parameter : number
// CHECK-NEXT:            Decl %d.20 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.7
// CHECK-NEXT:            Decl %d.21 'o' Parameter : %object.8
// CHECK-NEXT:            Decl %d.22 'k' Parameter : string
// CHECK-NEXT:            Decl %d.23 'arguments' Var Arguments

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        FunctionDeclaration : %function.2
// CHECK-NEXT:            Id 'f1' [D:E:%d.2 'f1']
// CHECK-NEXT:            Id 'k' [D:E:%d.9 'k']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    ObjectExpression : %object.5
// CHECK-NEXT:                        Property
// CHECK-NEXT:                            Id 'k' [D:E:%d.9 'k'] : string
// CHECK-NEXT:                            NumericLiteral : number
// CHECK-NEXT:        FunctionDeclaration : %function.3
// CHECK-NEXT:            Id 'f2' [D:E:%d.3 'f2']
// CHECK-NEXT:            Id 'k1' [D:E:%d.11 'k1']
// CHECK-NEXT:            Id 'k2' [D:E:%d.12 'k2']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    ObjectExpression : %object.10
// CHECK-NEXT:                        Property
// CHECK-NEXT:                            Id 'k1' [D:E:%d.11 'k1'] : string
// CHECK-NEXT:                            NumericLiteral : number
// CHECK-NEXT:                        Property
// CHECK-NEXT:                            Id 'k2' [D:E:%d.12 'k2'] : string
// CHECK-NEXT:                            StringLiteral : "s"
// CHECK-NEXT:        FunctionDeclaration : %function.4
// CHECK-NEXT:            Id 'f3' [D:E:%d.4 'f3']
// CHECK-NEXT:            Id 'k' [D:E:%d.14 'k']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    ObjectExpression : %object.5
// CHECK-NEXT:                        Property
// CHECK-NEXT:                            Id 'k' [D:E:%d.14 'k'] : string
// CHECK-NEXT:                            NumericLiteral : number
// CHECK-NEXT:        FunctionDeclaration : %function.6
// CHECK-NEXT:            Id 'f4' [D:E:%d.5 'f4']
// CHECK-NEXT:            Id 'o' [D:E:%d.16 'o']
// CHECK-NEXT:            Id 'k' [D:E:%d.17 'k']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    ObjectExpression : %object.5
// CHECK-NEXT:                        SpreadElement
// CHECK-NEXT:                            Id 'o' [D:E:%d.16 'o'] : %object.5
// CHECK-NEXT:                        Property
// CHECK-NEXT:                            Id 'k' [D:E:%d.17 'k'] : string
// CHECK-NEXT:                            NumericLiteral : number
// CHECK-NEXT:        FunctionDeclaration : %function.7
// CHECK-NEXT:            Id 'f5' [D:E:%d.6 'f5']
// CHECK-NEXT:            Id 'n' [D:E:%d.19 'n']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    ObjectExpression : %object.11
// CHECK-NEXT:                        Property
// CHECK-NEXT:                            Id 'n' [D:E:%d.19 'n'] : number
// CHECK-NEXT:                            NumericLiteral : number
// CHECK-NEXT:        FunctionDeclaration : %function.9
// CHECK-NEXT:            Id 'f6' [D:E:%d.7 'f6']
// CHECK-NEXT:            Id 'o' [D:E:%d.21 'o']
// CHECK-NEXT:            Id 'k' [D:E:%d.22 'k']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    ObjectExpression : %object.12
// CHECK-NEXT:                        SpreadElement
// CHECK-NEXT:                            Id 'o' [D:E:%d.21 'o'] : %object.8
// CHECK-NEXT:                        Property
// CHECK-NEXT:                            Id 'k' [D:E:%d.22 'k'] : string
// CHECK-NEXT:                            NumericLiteral : number
