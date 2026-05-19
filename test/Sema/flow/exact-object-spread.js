/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -fno-std-globals -typed -dump-sema %s | %FileCheckOrRegen %s --match-full-lines

let src: {x: number, y: string} = {x: 1, y: "hi"};

// Pure spread.
let pure: {x: number, y: string} = {...src};

// Spread then override.
let over: {x: number, y: string} = {...src, x: 99};

// Override then spread (spread wins on conflict).
let pre: {x: number, y: string} = {x: 99, ...src};

// Two spreads merged.
let a: {x: number} = {x: 1};
let b: {y: string} = {y: "hi"};
let merged: {x: number, y: string} = {...a, ...b};

// Empty spread source.
let empty: {} = {};
let copy: {x: number} = {...empty, x: 1};

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%object.2 = object({
// CHECK-NEXT:  x: number
// CHECK-NEXT:  y: string
// CHECK-NEXT:})
// CHECK-NEXT:%object.3 = object({
// CHECK-NEXT:  x: number
// CHECK-NEXT:})
// CHECK-NEXT:%object.4 = object({
// CHECK-NEXT:  y: string
// CHECK-NEXT:})
// CHECK-NEXT:%object.5 = object({
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'src' Let : %object.2
// CHECK-NEXT:        Decl %d.3 'pure' Let : %object.2
// CHECK-NEXT:        Decl %d.4 'over' Let : %object.2
// CHECK-NEXT:        Decl %d.5 'pre' Let : %object.2
// CHECK-NEXT:        Decl %d.6 'a' Let : %object.3
// CHECK-NEXT:        Decl %d.7 'b' Let : %object.4
// CHECK-NEXT:        Decl %d.8 'merged' Let : %object.2
// CHECK-NEXT:        Decl %d.9 'empty' Let : %object.5
// CHECK-NEXT:        Decl %d.10 'copy' Let : %object.3
// CHECK-NEXT:        Decl %d.11 'arguments' Var Arguments

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                ObjectExpression : %object.2
// CHECK-NEXT:                    Property
// CHECK-NEXT:                        Id 'x'
// CHECK-NEXT:                        NumericLiteral : number
// CHECK-NEXT:                    Property
// CHECK-NEXT:                        Id 'y'
// CHECK-NEXT:                        StringLiteral : string
// CHECK-NEXT:                Id 'src' [D:E:%d.2 'src']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                ObjectExpression : %object.2
// CHECK-NEXT:                    SpreadElement
// CHECK-NEXT:                        Id 'src' [D:E:%d.2 'src'] : %object.2
// CHECK-NEXT:                Id 'pure' [D:E:%d.3 'pure']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                ObjectExpression : %object.2
// CHECK-NEXT:                    SpreadElement
// CHECK-NEXT:                        Id 'src' [D:E:%d.2 'src'] : %object.2
// CHECK-NEXT:                    Property
// CHECK-NEXT:                        Id 'x'
// CHECK-NEXT:                        NumericLiteral : number
// CHECK-NEXT:                Id 'over' [D:E:%d.4 'over']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                ObjectExpression : %object.2
// CHECK-NEXT:                    Property
// CHECK-NEXT:                        Id 'x'
// CHECK-NEXT:                        NumericLiteral : number
// CHECK-NEXT:                    SpreadElement
// CHECK-NEXT:                        Id 'src' [D:E:%d.2 'src'] : %object.2
// CHECK-NEXT:                Id 'pre' [D:E:%d.5 'pre']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                ObjectExpression : %object.3
// CHECK-NEXT:                    Property
// CHECK-NEXT:                        Id 'x'
// CHECK-NEXT:                        NumericLiteral : number
// CHECK-NEXT:                Id 'a' [D:E:%d.6 'a']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                ObjectExpression : %object.4
// CHECK-NEXT:                    Property
// CHECK-NEXT:                        Id 'y'
// CHECK-NEXT:                        StringLiteral : string
// CHECK-NEXT:                Id 'b' [D:E:%d.7 'b']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                ObjectExpression : %object.2
// CHECK-NEXT:                    SpreadElement
// CHECK-NEXT:                        Id 'a' [D:E:%d.6 'a'] : %object.3
// CHECK-NEXT:                    SpreadElement
// CHECK-NEXT:                        Id 'b' [D:E:%d.7 'b'] : %object.4
// CHECK-NEXT:                Id 'merged' [D:E:%d.8 'merged']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                ObjectExpression : %object.5
// CHECK-NEXT:                Id 'empty' [D:E:%d.9 'empty']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                ObjectExpression : %object.3
// CHECK-NEXT:                    SpreadElement
// CHECK-NEXT:                        Id 'empty' [D:E:%d.9 'empty'] : %object.5
// CHECK-NEXT:                    Property
// CHECK-NEXT:                        Id 'x'
// CHECK-NEXT:                        NumericLiteral : number
// CHECK-NEXT:                Id 'copy' [D:E:%d.10 'copy']
