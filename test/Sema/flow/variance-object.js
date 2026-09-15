/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -fno-std-globals -typed -dump-sema %s | %FileCheckOrRegen %s --match-full-lines

type RO = {+x: number};
type WO = {-y: number};
type RW = {+r: number, -w: number, z: number};

function readRO(o: RO): number {
  return o.x;
}

function writeWO(o: WO): void {
  o.y = 1;
}

function rw(o: RW): void {
  let _r: number = o.r;
  o.w = 0;
  o.z = o.z + 1;
}

// Spread builds a fresh exact object whose fields are always invariant,
// regardless of the source's variance. The result can therefore be
// returned as `{x: number}`.
function spreadDropsVariance(o: RO): {x: number} {
  return {...o};
}

// Declaration destructuring of a readonly field is just a read.
function letDestrReadonly(o: RO): number {
  let {x} = o;
  return x;
}

// An invariant source field flows into a ReadOnly target field with type
// widening: only the target needs the right variance. `{x: number}` is a
// subtype of `{+x: number | string}` because the source gives up its write
// capability and the field type is covariantly widened.
function invariantToReadOnlyWiden(o: {x: number}): {+x: number | string} {
  return o;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%object.2 = object({
// CHECK-NEXT:  +x: number
// CHECK-NEXT:})
// CHECK-NEXT:%object.3 = object({
// CHECK-NEXT:  -y: number
// CHECK-NEXT:})
// CHECK-NEXT:%object.4 = object({
// CHECK-NEXT:  +r: number
// CHECK-NEXT:  -w: number
// CHECK-NEXT:  z: number
// CHECK-NEXT:})
// CHECK-NEXT:%function.5 = function(o: %object.2): number
// CHECK-NEXT:%function.6 = function(o: %object.3): void
// CHECK-NEXT:%function.7 = function(o: %object.4): void
// CHECK-NEXT:%function.8 = function(o: %object.2): %object.9
// CHECK-NEXT:%object.9 = object({
// CHECK-NEXT:  x: number
// CHECK-NEXT:})
// CHECK-NEXT:%function.10 = function(o: %object.9): %object.11
// CHECK-NEXT:%object.11 = object({
// CHECK-NEXT:  +x: %union.12
// CHECK-NEXT:})
// CHECK-NEXT:%union.12 = union(string | number)

// CHECK:SemContext
// CHECK-NEXT:Func strict mayReachImplicitReturn
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'readRO' Var : %function.5
// CHECK-NEXT:        Decl %d.3 'writeWO' Var : %function.6
// CHECK-NEXT:        Decl %d.4 'rw' Var : %function.7
// CHECK-NEXT:        Decl %d.5 'spreadDropsVariance' Var : %function.8
// CHECK-NEXT:        Decl %d.6 'letDestrReadonly' Var : %function.5
// CHECK-NEXT:        Decl %d.7 'invariantToReadOnlyWiden' Var : %function.10
// CHECK-NEXT:        Decl %d.8 'arguments' Var Arguments
// CHECK-NEXT:        hoistedFunction readRO
// CHECK-NEXT:        hoistedFunction writeWO
// CHECK-NEXT:        hoistedFunction rw
// CHECK-NEXT:        hoistedFunction spreadDropsVariance
// CHECK-NEXT:        hoistedFunction letDestrReadonly
// CHECK-NEXT:        hoistedFunction invariantToReadOnlyWiden
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.9 'o' Parameter : %object.2
// CHECK-NEXT:            Decl %d.10 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.3
// CHECK-NEXT:            Decl %d.11 'o' Parameter : %object.3
// CHECK-NEXT:            Decl %d.12 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.4
// CHECK-NEXT:            Decl %d.13 'o' Parameter : %object.4
// CHECK-NEXT:            Decl %d.14 '_r' Let : number
// CHECK-NEXT:            Decl %d.15 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.5
// CHECK-NEXT:            Decl %d.16 'o' Parameter : %object.2
// CHECK-NEXT:            Decl %d.17 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.6
// CHECK-NEXT:            Decl %d.18 'o' Parameter : %object.2
// CHECK-NEXT:            Decl %d.19 'x' Let : number
// CHECK-NEXT:            Decl %d.20 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.7
// CHECK-NEXT:            Decl %d.21 'o' Parameter : %object.9
// CHECK-NEXT:            Decl %d.22 'arguments' Var Arguments

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'RO'
// CHECK-NEXT:            ObjectTypeAnnotation
// CHECK-NEXT:                ObjectTypeProperty
// CHECK-NEXT:                    Id 'x'
// CHECK-NEXT:                    NumberTypeAnnotation
// CHECK-NEXT:                    Variance
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'WO'
// CHECK-NEXT:            ObjectTypeAnnotation
// CHECK-NEXT:                ObjectTypeProperty
// CHECK-NEXT:                    Id 'y'
// CHECK-NEXT:                    NumberTypeAnnotation
// CHECK-NEXT:                    Variance
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'RW'
// CHECK-NEXT:            ObjectTypeAnnotation
// CHECK-NEXT:                ObjectTypeProperty
// CHECK-NEXT:                    Id 'r'
// CHECK-NEXT:                    NumberTypeAnnotation
// CHECK-NEXT:                    Variance
// CHECK-NEXT:                ObjectTypeProperty
// CHECK-NEXT:                    Id 'w'
// CHECK-NEXT:                    NumberTypeAnnotation
// CHECK-NEXT:                    Variance
// CHECK-NEXT:                ObjectTypeProperty
// CHECK-NEXT:                    Id 'z'
// CHECK-NEXT:                    NumberTypeAnnotation
// CHECK-NEXT:        FunctionDeclaration : %function.5
// CHECK-NEXT:            Id 'readRO' [D:E:%d.2 'readRO']
// CHECK-NEXT:            Id 'o' [D:E:%d.9 'o']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    MemberExpression : number
// CHECK-NEXT:                        Id 'o' [D:E:%d.9 'o'] : %object.2
// CHECK-NEXT:                        Id 'x'
// CHECK-NEXT:        FunctionDeclaration : %function.6
// CHECK-NEXT:            Id 'writeWO' [D:E:%d.3 'writeWO']
// CHECK-NEXT:            Id 'o' [D:E:%d.11 'o']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    AssignmentExpression : number
// CHECK-NEXT:                        MemberExpression : number
// CHECK-NEXT:                            Id 'o' [D:E:%d.11 'o'] : %object.3
// CHECK-NEXT:                            Id 'y'
// CHECK-NEXT:                        NumericLiteral : number
// CHECK-NEXT:        FunctionDeclaration : %function.7
// CHECK-NEXT:            Id 'rw' [D:E:%d.4 'rw']
// CHECK-NEXT:            Id 'o' [D:E:%d.13 'o']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                VariableDeclaration
// CHECK-NEXT:                    VariableDeclarator
// CHECK-NEXT:                        MemberExpression : number
// CHECK-NEXT:                            Id 'o' [D:E:%d.13 'o'] : %object.4
// CHECK-NEXT:                            Id 'r'
// CHECK-NEXT:                        Id '_r' [D:E:%d.14 '_r']
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    AssignmentExpression : number
// CHECK-NEXT:                        MemberExpression : number
// CHECK-NEXT:                            Id 'o' [D:E:%d.13 'o'] : %object.4
// CHECK-NEXT:                            Id 'w'
// CHECK-NEXT:                        NumericLiteral : number
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    AssignmentExpression : number
// CHECK-NEXT:                        MemberExpression : number
// CHECK-NEXT:                            Id 'o' [D:E:%d.13 'o'] : %object.4
// CHECK-NEXT:                            Id 'z'
// CHECK-NEXT:                        BinaryExpression : number
// CHECK-NEXT:                            MemberExpression : number
// CHECK-NEXT:                                Id 'o' [D:E:%d.13 'o'] : %object.4
// CHECK-NEXT:                                Id 'z'
// CHECK-NEXT:                            BinOp +
// CHECK-NEXT:                            NumericLiteral : number
// CHECK-NEXT:        FunctionDeclaration : %function.8
// CHECK-NEXT:            Id 'spreadDropsVariance' [D:E:%d.5 'spreadDropsVariance']
// CHECK-NEXT:            Id 'o' [D:E:%d.16 'o']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    ObjectExpression : %object.9
// CHECK-NEXT:                        SpreadElement
// CHECK-NEXT:                            Id 'o' [D:E:%d.16 'o'] : %object.2
// CHECK-NEXT:        FunctionDeclaration : %function.5
// CHECK-NEXT:            Id 'letDestrReadonly' [D:E:%d.6 'letDestrReadonly']
// CHECK-NEXT:            Id 'o' [D:E:%d.18 'o']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                VariableDeclaration
// CHECK-NEXT:                    VariableDeclarator
// CHECK-NEXT:                        Id 'o' [D:E:%d.18 'o'] : %object.2
// CHECK-NEXT:                        ObjectPattern : %object.2
// CHECK-NEXT:                            Property
// CHECK-NEXT:                                Id 'x'
// CHECK-NEXT:                                Id 'x' [D:E:%d.19 'x'] : number
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    Id 'x' [D:E:%d.19 'x'] : number
// CHECK-NEXT:        FunctionDeclaration : %function.10
// CHECK-NEXT:            Id 'invariantToReadOnlyWiden' [D:E:%d.7 'invariantToReadOnlyWiden']
// CHECK-NEXT:            Id 'o' [D:E:%d.21 'o']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    Id 'o' [D:E:%d.21 'o'] : %object.9
