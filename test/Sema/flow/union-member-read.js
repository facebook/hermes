/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror --typed --dump-sema -fno-std-globals %s | %FileCheckOrRegen %s --match-full-lines

'use strict';

// Reading a field that exists in every arm of a union is allowed (regardless
// of slot); the result type is the union of the per-arm field types.

type Circle = {tag: 'circle', radius: number};
type Square = {tag: 'square', side: number};

// Common discriminant: result is the union of the literal arm types.
function tagOf(s: Circle | Square): 'circle' | 'square' {
  return s.tag;
}

// Identical field types collapse to the single type.
function widthOf(p: {n: number, a: number} | {n: number, b: string}): number {
  return p.n;
}

// Differing field types at the same slot yield a union result.
function valueOf(p: {k: number} | {k: string}): number | string {
  return p.k;
}

// The field sits at a different slot in each arm. Still allowed: only presence
// is required, not a shared slot.
function slotOf(p: {tag: string, x: number} | {x: number, tag: string}): string {
  return p.tag;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%union.2 = union(%object.12 | %object.13)
// CHECK-NEXT:%function.3 = function(s: %union.2): %union.10
// CHECK-NEXT:%union.4 = union(%object.14 | %object.15)
// CHECK-NEXT:%function.5 = function(p: %union.4): number
// CHECK-NEXT:%union.6 = union(%object.16 | %object.17)
// CHECK-NEXT:%function.7 = function(p: %union.6): %union.11
// CHECK-NEXT:%union.8 = union(%object.18 | %object.19)
// CHECK-NEXT:%function.9 = function(p: %union.8): string
// CHECK-NEXT:%union.10 = union("circle" | "square")
// CHECK-NEXT:%union.11 = union(string | number)
// CHECK-NEXT:%object.12 = object({
// CHECK-NEXT:  tag: "circle"
// CHECK-NEXT:  radius: number
// CHECK-NEXT:})
// CHECK-NEXT:%object.13 = object({
// CHECK-NEXT:  tag: "square"
// CHECK-NEXT:  side: number
// CHECK-NEXT:})
// CHECK-NEXT:%object.14 = object({
// CHECK-NEXT:  n: number
// CHECK-NEXT:  a: number
// CHECK-NEXT:})
// CHECK-NEXT:%object.15 = object({
// CHECK-NEXT:  n: number
// CHECK-NEXT:  b: string
// CHECK-NEXT:})
// CHECK-NEXT:%object.16 = object({
// CHECK-NEXT:  k: string
// CHECK-NEXT:})
// CHECK-NEXT:%object.17 = object({
// CHECK-NEXT:  k: number
// CHECK-NEXT:})
// CHECK-NEXT:%object.18 = object({
// CHECK-NEXT:  tag: string
// CHECK-NEXT:  x: number
// CHECK-NEXT:})
// CHECK-NEXT:%object.19 = object({
// CHECK-NEXT:  x: number
// CHECK-NEXT:  tag: string
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict mayReachImplicitReturn
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'tagOf' Var : %function.3
// CHECK-NEXT:        Decl %d.3 'widthOf' Var : %function.5
// CHECK-NEXT:        Decl %d.4 'valueOf' Var : %function.7
// CHECK-NEXT:        Decl %d.5 'slotOf' Var : %function.9
// CHECK-NEXT:        Decl %d.6 'arguments' Var Arguments
// CHECK-NEXT:        hoistedFunction tagOf
// CHECK-NEXT:        hoistedFunction widthOf
// CHECK-NEXT:        hoistedFunction valueOf
// CHECK-NEXT:        hoistedFunction slotOf
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.7 's' Parameter : %union.2
// CHECK-NEXT:            Decl %d.8 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.3
// CHECK-NEXT:            Decl %d.9 'p' Parameter : %union.4
// CHECK-NEXT:            Decl %d.10 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.4
// CHECK-NEXT:            Decl %d.11 'p' Parameter : %union.6
// CHECK-NEXT:            Decl %d.12 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.5
// CHECK-NEXT:            Decl %d.13 'p' Parameter : %union.8
// CHECK-NEXT:            Decl %d.14 'arguments' Var Arguments

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            StringLiteral : "use strict"
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'Circle'
// CHECK-NEXT:            ObjectTypeAnnotation
// CHECK-NEXT:                ObjectTypeProperty
// CHECK-NEXT:                    Id 'tag'
// CHECK-NEXT:                    StringLiteralTypeAnnotation
// CHECK-NEXT:                ObjectTypeProperty
// CHECK-NEXT:                    Id 'radius'
// CHECK-NEXT:                    NumberTypeAnnotation
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'Square'
// CHECK-NEXT:            ObjectTypeAnnotation
// CHECK-NEXT:                ObjectTypeProperty
// CHECK-NEXT:                    Id 'tag'
// CHECK-NEXT:                    StringLiteralTypeAnnotation
// CHECK-NEXT:                ObjectTypeProperty
// CHECK-NEXT:                    Id 'side'
// CHECK-NEXT:                    NumberTypeAnnotation
// CHECK-NEXT:        FunctionDeclaration : %function.3
// CHECK-NEXT:            Id 'tagOf' [D:E:%d.2 'tagOf']
// CHECK-NEXT:            Id 's' [D:E:%d.7 's']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    MemberExpression : %union.10
// CHECK-NEXT:                        Id 's' [D:E:%d.7 's'] : %union.2
// CHECK-NEXT:                        Id 'tag'
// CHECK-NEXT:        FunctionDeclaration : %function.5
// CHECK-NEXT:            Id 'widthOf' [D:E:%d.3 'widthOf']
// CHECK-NEXT:            Id 'p' [D:E:%d.9 'p']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    MemberExpression : number
// CHECK-NEXT:                        Id 'p' [D:E:%d.9 'p'] : %union.4
// CHECK-NEXT:                        Id 'n'
// CHECK-NEXT:        FunctionDeclaration : %function.7
// CHECK-NEXT:            Id 'valueOf' [D:E:%d.4 'valueOf']
// CHECK-NEXT:            Id 'p' [D:E:%d.11 'p']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    MemberExpression : %union.11
// CHECK-NEXT:                        Id 'p' [D:E:%d.11 'p'] : %union.6
// CHECK-NEXT:                        Id 'k'
// CHECK-NEXT:        FunctionDeclaration : %function.9
// CHECK-NEXT:            Id 'slotOf' [D:E:%d.5 'slotOf']
// CHECK-NEXT:            Id 'p' [D:E:%d.13 'p']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    MemberExpression : string
// CHECK-NEXT:                        Id 'p' [D:E:%d.13 'p'] : %union.8
// CHECK-NEXT:                        Id 'tag'
