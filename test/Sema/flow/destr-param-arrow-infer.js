/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals --typed --dump-sema %s | %FileCheckOrRegen %s --match-full-lines

// A destructuring arrow parameter needs no annotation when a constraint
// (contextual type) determines its type; the binding types are inferred from
// the constraint. See destr-param-arrow.js for the annotated patterns and
// destr-param-arrow-error.js for the no-constraint error.

'use strict';

// Constraint from an annotated const.
const c: ({x: number}) => number = ({x}) => x;

// Constraint from a call argument.
function apply(cb: ({x: number}) => void): void {}
apply(({x}) => {});

// Tuple pattern.
const t: ([number, string]) => string = ([n, s]) => s;

// Default value, checked against the constraint.
const d: ({x: number}) => number = ({x} = {x: 5}) => x;

// Nested pattern.
const n: ({a: [number, number]}) => number = ({a: [p, q]}) => p + q;

// A parameter after the pattern keeps its own constraint type.
const after: ({x: number}, string) => string = ({x}, s) => s;

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%object.2 = object({
// CHECK-NEXT:  x: number
// CHECK-NEXT:})
// CHECK-NEXT:%function.3 = function(%object.2): number
// CHECK-NEXT:%tuple.4 = tuple(number, string)
// CHECK-NEXT:%function.5 = function(%tuple.4): string
// CHECK-NEXT:%tuple.6 = tuple(number, number)
// CHECK-NEXT:%object.7 = object({
// CHECK-NEXT:  a: %tuple.6
// CHECK-NEXT:})
// CHECK-NEXT:%function.8 = function(%object.7): number
// CHECK-NEXT:%function.9 = function(%object.2, string): string
// CHECK-NEXT:%function.10 = function(%object.2): void
// CHECK-NEXT:%function.11 = function(cb: %function.10): void
// CHECK-NEXT:%function.12 = function(%object.2): number

// CHECK:SemContext
// CHECK-NEXT:Func strict mayReachImplicitReturn
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'c' Const : %function.3
// CHECK-NEXT:        Decl %d.3 'apply' Var : %function.11
// CHECK-NEXT:        Decl %d.4 't' Const : %function.5
// CHECK-NEXT:        Decl %d.5 'd' Const : %function.3
// CHECK-NEXT:        Decl %d.6 'n' Const : %function.8
// CHECK-NEXT:        Decl %d.7 'after' Const : %function.9
// CHECK-NEXT:        Decl %d.8 'arguments' Var Arguments
// CHECK-NEXT:        hoistedFunction apply
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.9 'x' Parameter : number
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.3
// CHECK-NEXT:            Decl %d.10 'cb' Parameter : %function.10
// CHECK-NEXT:            Decl %d.11 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.4
// CHECK-NEXT:            Decl %d.12 'x' Parameter : number
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.5
// CHECK-NEXT:            Decl %d.13 'n' Parameter : number
// CHECK-NEXT:            Decl %d.14 's' Parameter : string
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.6
// CHECK-NEXT:            Decl %d.15 'x' Parameter : number
// CHECK-NEXT:            Scope %s.7
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.8
// CHECK-NEXT:            Decl %d.16 'p' Parameter : number
// CHECK-NEXT:            Decl %d.17 'q' Parameter : number
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.9
// CHECK-NEXT:            Decl %d.18 'x' Parameter : number
// CHECK-NEXT:            Decl %d.19 's' Parameter : string

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            StringLiteral : string
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                ArrowFunctionExpression : %function.3
// CHECK-NEXT:                    ObjectPattern : %object.2
// CHECK-NEXT:                        Property
// CHECK-NEXT:                            Id 'x'
// CHECK-NEXT:                            Id 'x' [D:E:%d.9 'x'] : number
// CHECK-NEXT:                    BlockStatement
// CHECK-NEXT:                        ReturnStatement
// CHECK-NEXT:                            Id 'x' [D:E:%d.9 'x'] : number
// CHECK-NEXT:                Id 'c' [D:E:%d.2 'c']
// CHECK-NEXT:        FunctionDeclaration : %function.11
// CHECK-NEXT:            Id 'apply' [D:E:%d.3 'apply']
// CHECK-NEXT:            Id 'cb' [D:E:%d.10 'cb']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            CallExpression : void
// CHECK-NEXT:                Id 'apply' [D:E:%d.3 'apply'] : %function.11
// CHECK-NEXT:                ArrowFunctionExpression : %function.10
// CHECK-NEXT:                    ObjectPattern : %object.2
// CHECK-NEXT:                        Property
// CHECK-NEXT:                            Id 'x'
// CHECK-NEXT:                            Id 'x' [D:E:%d.12 'x'] : number
// CHECK-NEXT:                    BlockStatement
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                ArrowFunctionExpression : %function.5
// CHECK-NEXT:                    ArrayPattern : %tuple.4
// CHECK-NEXT:                        Id 'n' [D:E:%d.13 'n'] : number
// CHECK-NEXT:                        Id 's' [D:E:%d.14 's'] : string
// CHECK-NEXT:                    BlockStatement
// CHECK-NEXT:                        ReturnStatement
// CHECK-NEXT:                            Id 's' [D:E:%d.14 's'] : string
// CHECK-NEXT:                Id 't' [D:E:%d.4 't']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                ArrowFunctionExpression : %function.12
// CHECK-NEXT:                    AssignmentPattern
// CHECK-NEXT:                        ObjectPattern : %object.2
// CHECK-NEXT:                            Property
// CHECK-NEXT:                                Id 'x'
// CHECK-NEXT:                                Id 'x' [D:E:%d.15 'x'] : number
// CHECK-NEXT:                        ObjectExpression : %object.2
// CHECK-NEXT:                            Property
// CHECK-NEXT:                                Id 'x'
// CHECK-NEXT:                                NumericLiteral : number
// CHECK-NEXT:                    BlockStatement
// CHECK-NEXT:                        ReturnStatement
// CHECK-NEXT:                            Id 'x' [D:E:%d.15 'x'] : number
// CHECK-NEXT:                Id 'd' [D:E:%d.5 'd']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                ArrowFunctionExpression : %function.8
// CHECK-NEXT:                    ObjectPattern : %object.7
// CHECK-NEXT:                        Property
// CHECK-NEXT:                            Id 'a'
// CHECK-NEXT:                            ArrayPattern : %tuple.6
// CHECK-NEXT:                                Id 'p' [D:E:%d.16 'p'] : number
// CHECK-NEXT:                                Id 'q' [D:E:%d.17 'q'] : number
// CHECK-NEXT:                    BlockStatement
// CHECK-NEXT:                        ReturnStatement
// CHECK-NEXT:                            BinaryExpression : number
// CHECK-NEXT:                                Id 'p' [D:E:%d.16 'p'] : number
// CHECK-NEXT:                                BinOp +
// CHECK-NEXT:                                Id 'q' [D:E:%d.17 'q'] : number
// CHECK-NEXT:                Id 'n' [D:E:%d.6 'n']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                ArrowFunctionExpression : %function.9
// CHECK-NEXT:                    ObjectPattern : %object.2
// CHECK-NEXT:                        Property
// CHECK-NEXT:                            Id 'x'
// CHECK-NEXT:                            Id 'x' [D:E:%d.18 'x'] : number
// CHECK-NEXT:                    Id 's' [D:E:%d.19 's']
// CHECK-NEXT:                    BlockStatement
// CHECK-NEXT:                        ReturnStatement
// CHECK-NEXT:                            Id 's' [D:E:%d.19 's'] : string
// CHECK-NEXT:                Id 'after' [D:E:%d.7 'after']
