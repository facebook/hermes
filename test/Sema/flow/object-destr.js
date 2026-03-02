/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals --typed --dump-sema %s | %FileCheckOrRegen %s --match-full-lines

'use strict';

// Basic object destructuring
let obj: {x: number, y: string} = {x: 1, y: 'hello'};
let {x, y} = obj;

// Renamed properties
let {x: a, y: b} = obj;

// Nested object destructuring
let nested: {outer: {inner: number}} = {outer: {inner: 42}};
let {outer: {inner}} = nested;

// Destructuring from any
let anyVar: any = obj;
let {x: anyX, y: anyY}: any = anyVar;

// Explicit type annotation on pattern
let {x: typedX, y: typedY}: {x: number, y: string} = anyVar;
let typedXNum: number = typedX;
let typedYNum: string = typedY;

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%object.2 = object({
// CHECK-NEXT:  x: number
// CHECK-NEXT:  y: string
// CHECK-NEXT:})
// CHECK-NEXT:%object.3 = object({
// CHECK-NEXT:  inner: number
// CHECK-NEXT:})
// CHECK-NEXT:%object.4 = object({
// CHECK-NEXT:  outer: %object.3
// CHECK-NEXT:})
// CHECK-NEXT:%object.5 = object({
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'obj' Let : %object.2
// CHECK-NEXT:            Decl %d.3 'x' Let : number
// CHECK-NEXT:            Decl %d.4 'y' Let : string
// CHECK-NEXT:            Decl %d.5 'a' Let : number
// CHECK-NEXT:            Decl %d.6 'b' Let : string
// CHECK-NEXT:            Decl %d.7 'nested' Let : %object.4
// CHECK-NEXT:            Decl %d.8 'inner' Let : number
// CHECK-NEXT:            Decl %d.9 'anyVar' Let : any
// CHECK-NEXT:            Decl %d.10 'anyX' Let : any
// CHECK-NEXT:            Decl %d.11 'anyY' Let : any
// CHECK-NEXT:            Decl %d.12 'typedX' Let : number
// CHECK-NEXT:            Decl %d.13 'typedY' Let : string
// CHECK-NEXT:            Decl %d.14 'typedXNum' Let : number
// CHECK-NEXT:            Decl %d.15 'typedYNum' Let : string
// CHECK-NEXT:            Decl %d.16 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    ExpressionStatement
// CHECK-NEXT:                        StringLiteral : string
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            ObjectExpression : %object.2
// CHECK-NEXT:                                Property
// CHECK-NEXT:                                    Id 'x'
// CHECK-NEXT:                                    NumericLiteral : number
// CHECK-NEXT:                                Property
// CHECK-NEXT:                                    Id 'y'
// CHECK-NEXT:                                    StringLiteral : string
// CHECK-NEXT:                            Id 'obj' [D:E:%d.2 'obj']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            Id 'obj' [D:E:%d.2 'obj'] : %object.2
// CHECK-NEXT:                            ObjectPattern : %object.2
// CHECK-NEXT:                                Property
// CHECK-NEXT:                                    Id 'x'
// CHECK-NEXT:                                    Id 'x' [D:E:%d.3 'x'] : number
// CHECK-NEXT:                                Property
// CHECK-NEXT:                                    Id 'y'
// CHECK-NEXT:                                    Id 'y' [D:E:%d.4 'y'] : string
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            Id 'obj' [D:E:%d.2 'obj'] : %object.2
// CHECK-NEXT:                            ObjectPattern : %object.2
// CHECK-NEXT:                                Property
// CHECK-NEXT:                                    Id 'x'
// CHECK-NEXT:                                    Id 'a' [D:E:%d.5 'a'] : number
// CHECK-NEXT:                                Property
// CHECK-NEXT:                                    Id 'y'
// CHECK-NEXT:                                    Id 'b' [D:E:%d.6 'b'] : string
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            ObjectExpression : %object.4
// CHECK-NEXT:                                Property
// CHECK-NEXT:                                    Id 'outer'
// CHECK-NEXT:                                    ObjectExpression : %object.3
// CHECK-NEXT:                                        Property
// CHECK-NEXT:                                            Id 'inner'
// CHECK-NEXT:                                            NumericLiteral : number
// CHECK-NEXT:                            Id 'nested' [D:E:%d.7 'nested']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            Id 'nested' [D:E:%d.7 'nested'] : %object.4
// CHECK-NEXT:                            ObjectPattern : %object.4
// CHECK-NEXT:                                Property
// CHECK-NEXT:                                    Id 'outer'
// CHECK-NEXT:                                    ObjectPattern : %object.3
// CHECK-NEXT:                                        Property
// CHECK-NEXT:                                            Id 'inner'
// CHECK-NEXT:                                            Id 'inner' [D:E:%d.8 'inner'] : number
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            Id 'obj' [D:E:%d.2 'obj'] : %object.2
// CHECK-NEXT:                            Id 'anyVar' [D:E:%d.9 'anyVar']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            Id 'anyVar' [D:E:%d.9 'anyVar'] : any
// CHECK-NEXT:                            ObjectPattern : any
// CHECK-NEXT:                                Property
// CHECK-NEXT:                                    Id 'x'
// CHECK-NEXT:                                    Id 'anyX' [D:E:%d.10 'anyX'] : any
// CHECK-NEXT:                                Property
// CHECK-NEXT:                                    Id 'y'
// CHECK-NEXT:                                    Id 'anyY' [D:E:%d.11 'anyY'] : any
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            ImplicitCheckedCast : %object.2
// CHECK-NEXT:                                Id 'anyVar' [D:E:%d.9 'anyVar'] : any
// CHECK-NEXT:                            ObjectPattern : %object.2
// CHECK-NEXT:                                Property
// CHECK-NEXT:                                    Id 'x'
// CHECK-NEXT:                                    Id 'typedX' [D:E:%d.12 'typedX'] : number
// CHECK-NEXT:                                Property
// CHECK-NEXT:                                    Id 'y'
// CHECK-NEXT:                                    Id 'typedY' [D:E:%d.13 'typedY'] : string
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            Id 'typedX' [D:E:%d.12 'typedX'] : number
// CHECK-NEXT:                            Id 'typedXNum' [D:E:%d.14 'typedXNum']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            Id 'typedY' [D:E:%d.13 'typedY'] : string
// CHECK-NEXT:                            Id 'typedYNum' [D:E:%d.15 'typedYNum']
// CHECK-NEXT:            ObjectExpression : %object.5
