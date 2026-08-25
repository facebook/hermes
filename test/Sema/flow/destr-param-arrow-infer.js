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

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%object.2 = object({
// CHECK-NEXT:  x: number
// CHECK-NEXT:})
// CHECK-NEXT:%function.3 = function(%object.2): number
// CHECK-NEXT:%function.4 = function(%object.2): void
// CHECK-NEXT:%function.5 = function(cb: %function.4): void

// CHECK:SemContext
// CHECK-NEXT:Func strict mayReachImplicitReturn
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'c' Const : %function.3
// CHECK-NEXT:        Decl %d.3 'apply' Var : %function.5
// CHECK-NEXT:        Decl %d.4 'arguments' Var Arguments
// CHECK-NEXT:        hoistedFunction apply
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.5 'x' Parameter : number
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.3
// CHECK-NEXT:            Decl %d.6 'cb' Parameter : %function.4
// CHECK-NEXT:            Decl %d.7 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.4
// CHECK-NEXT:            Decl %d.8 'x' Parameter : number

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
// CHECK-NEXT:                            Id 'x' [D:E:%d.5 'x'] : number
// CHECK-NEXT:                    BlockStatement
// CHECK-NEXT:                        ReturnStatement
// CHECK-NEXT:                            Id 'x' [D:E:%d.5 'x'] : number
// CHECK-NEXT:                Id 'c' [D:E:%d.2 'c']
// CHECK-NEXT:        FunctionDeclaration : %function.5
// CHECK-NEXT:            Id 'apply' [D:E:%d.3 'apply']
// CHECK-NEXT:            Id 'cb' [D:E:%d.6 'cb']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            CallExpression : void
// CHECK-NEXT:                Id 'apply' [D:E:%d.3 'apply'] : %function.5
// CHECK-NEXT:                ArrowFunctionExpression : %function.4
// CHECK-NEXT:                    ObjectPattern : %object.2
// CHECK-NEXT:                        Property
// CHECK-NEXT:                            Id 'x'
// CHECK-NEXT:                            Id 'x' [D:E:%d.8 'x'] : number
// CHECK-NEXT:                    BlockStatement
