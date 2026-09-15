/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -fno-std-globals -typed -dump-sema %s | %FileCheckOrRegen %s --match-full-lines

// Covariant: {+x: number} flows into {+x: number | string} (widening).
function widenRO(a: {+x: number}): {+x: number | string} {
  return a;
}

// Contravariant: {-x: number | string} flows into {-x: number} (narrowing).
function narrowWO(a: {-x: number | string}): {-x: number} {
  return a;
}

// Same variance, equal types: always flows.
function sameRO(a: {+x: number}): {+x: number} {
  return a;
}
function sameWO(a: {-y: string}): {-y: string} {
  return a;
}

// Invariant source can flow into ReadOnly (drop write capability).
function dropWrite(a: {x: number}): {+x: number} {
  return a;
}
// Invariant source can flow into WriteOnly (drop read capability).
function dropRead(a: {x: number}): {-x: number} {
  return a;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%object.2 = object({
// CHECK-NEXT:  +x: number
// CHECK-NEXT:})
// CHECK-NEXT:%function.3 = function(a: %object.2): %object.12
// CHECK-NEXT:%object.4 = object({
// CHECK-NEXT:  -x: %union.13
// CHECK-NEXT:})
// CHECK-NEXT:%function.5 = function(a: %object.4): %object.14
// CHECK-NEXT:%function.6 = function(a: %object.2): %object.2
// CHECK-NEXT:%object.7 = object({
// CHECK-NEXT:  -y: string
// CHECK-NEXT:})
// CHECK-NEXT:%function.8 = function(a: %object.7): %object.7
// CHECK-NEXT:%object.9 = object({
// CHECK-NEXT:  x: number
// CHECK-NEXT:})
// CHECK-NEXT:%function.10 = function(a: %object.9): %object.2
// CHECK-NEXT:%function.11 = function(a: %object.9): %object.14
// CHECK-NEXT:%object.12 = object({
// CHECK-NEXT:  +x: %union.13
// CHECK-NEXT:})
// CHECK-NEXT:%union.13 = union(string | number)
// CHECK-NEXT:%object.14 = object({
// CHECK-NEXT:  -x: number
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict mayReachImplicitReturn
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'widenRO' Var : %function.3
// CHECK-NEXT:        Decl %d.3 'narrowWO' Var : %function.5
// CHECK-NEXT:        Decl %d.4 'sameRO' Var : %function.6
// CHECK-NEXT:        Decl %d.5 'sameWO' Var : %function.8
// CHECK-NEXT:        Decl %d.6 'dropWrite' Var : %function.10
// CHECK-NEXT:        Decl %d.7 'dropRead' Var : %function.11
// CHECK-NEXT:        Decl %d.8 'arguments' Var Arguments
// CHECK-NEXT:        hoistedFunction widenRO
// CHECK-NEXT:        hoistedFunction narrowWO
// CHECK-NEXT:        hoistedFunction sameRO
// CHECK-NEXT:        hoistedFunction sameWO
// CHECK-NEXT:        hoistedFunction dropWrite
// CHECK-NEXT:        hoistedFunction dropRead
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.9 'a' Parameter : %object.2
// CHECK-NEXT:            Decl %d.10 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.3
// CHECK-NEXT:            Decl %d.11 'a' Parameter : %object.4
// CHECK-NEXT:            Decl %d.12 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.4
// CHECK-NEXT:            Decl %d.13 'a' Parameter : %object.2
// CHECK-NEXT:            Decl %d.14 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.5
// CHECK-NEXT:            Decl %d.15 'a' Parameter : %object.7
// CHECK-NEXT:            Decl %d.16 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.6
// CHECK-NEXT:            Decl %d.17 'a' Parameter : %object.9
// CHECK-NEXT:            Decl %d.18 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.7
// CHECK-NEXT:            Decl %d.19 'a' Parameter : %object.9
// CHECK-NEXT:            Decl %d.20 'arguments' Var Arguments

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        FunctionDeclaration : %function.3
// CHECK-NEXT:            Id 'widenRO' [D:E:%d.2 'widenRO']
// CHECK-NEXT:            Id 'a' [D:E:%d.9 'a']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    Id 'a' [D:E:%d.9 'a'] : %object.2
// CHECK-NEXT:        FunctionDeclaration : %function.5
// CHECK-NEXT:            Id 'narrowWO' [D:E:%d.3 'narrowWO']
// CHECK-NEXT:            Id 'a' [D:E:%d.11 'a']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    Id 'a' [D:E:%d.11 'a'] : %object.4
// CHECK-NEXT:        FunctionDeclaration : %function.6
// CHECK-NEXT:            Id 'sameRO' [D:E:%d.4 'sameRO']
// CHECK-NEXT:            Id 'a' [D:E:%d.13 'a']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    Id 'a' [D:E:%d.13 'a'] : %object.2
// CHECK-NEXT:        FunctionDeclaration : %function.8
// CHECK-NEXT:            Id 'sameWO' [D:E:%d.5 'sameWO']
// CHECK-NEXT:            Id 'a' [D:E:%d.15 'a']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    Id 'a' [D:E:%d.15 'a'] : %object.7
// CHECK-NEXT:        FunctionDeclaration : %function.10
// CHECK-NEXT:            Id 'dropWrite' [D:E:%d.6 'dropWrite']
// CHECK-NEXT:            Id 'a' [D:E:%d.17 'a']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    Id 'a' [D:E:%d.17 'a'] : %object.9
// CHECK-NEXT:        FunctionDeclaration : %function.11
// CHECK-NEXT:            Id 'dropRead' [D:E:%d.7 'dropRead']
// CHECK-NEXT:            Id 'a' [D:E:%d.19 'a']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    Id 'a' [D:E:%d.19 'a'] : %object.9
