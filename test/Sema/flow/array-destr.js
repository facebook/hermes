/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -typed -dump-sema %s | %FileCheckOrRegen %s --match-full-lines

'use strict';

let arr: Array<number> = [1, 2, 3, 4];

// Array destructuring without rest.
let [a, b]: Array<number> = arr;

// Array destructuring with rest.
let [c, d, ...rest]: Array<number> = arr;

// Rest only.
let [...all]: Array<number> = arr;

// T[] syntax.
let [e, ...erest]: number[] = arr;

// Array destructuring as function parameter.
function f([h, ...t]: Array<string>): void {}

// Assignment-expression destructuring from Array<T>: the LHS pattern is
// typed against the RHS, so each non-rest slot must accept the Array
// element type and any rest binding must accept Array<T>.
let x: number = 0;
let r: Array<number> = [];
[x, ...r] = arr;
[...r] = arr;
// Without a rest binding, the LHS is also typed as Array<T>.
let y: number = 0;
[x, y] = arr;

// Nested array destructuring at declaration.
let nested: Array<Array<number>> = [[1, 2]];
let [[na, nb]]: Array<Array<number>> = nested;

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%class.2 = class(Array<number>)
// CHECK-NEXT:%class.3 = class(Array<%class.2>)
// CHECK-NEXT:%class.4 = class(Array<string>)
// CHECK-NEXT:%function.5 = function(%class.4): void

// CHECK:SemContext
// CHECK-NEXT:Func strict mayReachImplicitReturn
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'arr' Let : %class.2
// CHECK-NEXT:        Decl %d.3 'a' Let : number
// CHECK-NEXT:        Decl %d.4 'b' Let : number
// CHECK-NEXT:        Decl %d.5 'c' Let : number
// CHECK-NEXT:        Decl %d.6 'd' Let : number
// CHECK-NEXT:        Decl %d.7 'rest' Let : %class.2
// CHECK-NEXT:        Decl %d.8 'all' Let : %class.2
// CHECK-NEXT:        Decl %d.9 'e' Let : number
// CHECK-NEXT:        Decl %d.10 'erest' Let : %class.2
// CHECK-NEXT:        Decl %d.11 'f' Var : %function.5
// CHECK-NEXT:        Decl %d.12 'x' Let : number
// CHECK-NEXT:        Decl %d.13 'r' Let : %class.2
// CHECK-NEXT:        Decl %d.14 'y' Let : number
// CHECK-NEXT:        Decl %d.15 'nested' Let : %class.3
// CHECK-NEXT:        Decl %d.16 'na' Let : number
// CHECK-NEXT:        Decl %d.17 'nb' Let : number
// CHECK-NEXT:        Decl %d.18 'arguments' Var Arguments
// CHECK-NEXT:        hoistedFunction f
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.19 'h' Parameter : string
// CHECK-NEXT:            Decl %d.20 't' Parameter : %class.4
// CHECK-NEXT:            Decl %d.21 'arguments' Var Arguments

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            StringLiteral : string
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                ArrayExpression : %class.2
// CHECK-NEXT:                    NumericLiteral : number
// CHECK-NEXT:                    NumericLiteral : number
// CHECK-NEXT:                    NumericLiteral : number
// CHECK-NEXT:                    NumericLiteral : number
// CHECK-NEXT:                Id 'arr' [D:E:%d.2 'arr']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                Id 'arr' [D:E:%d.2 'arr'] : %class.2
// CHECK-NEXT:                ArrayPattern : %class.2
// CHECK-NEXT:                    Id 'a' [D:E:%d.3 'a'] : number
// CHECK-NEXT:                    Id 'b' [D:E:%d.4 'b'] : number
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                Id 'arr' [D:E:%d.2 'arr'] : %class.2
// CHECK-NEXT:                ArrayPattern : %class.2
// CHECK-NEXT:                    Id 'c' [D:E:%d.5 'c'] : number
// CHECK-NEXT:                    Id 'd' [D:E:%d.6 'd'] : number
// CHECK-NEXT:                    RestElement
// CHECK-NEXT:                        Id 'rest' [D:E:%d.7 'rest'] : %class.2
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                Id 'arr' [D:E:%d.2 'arr'] : %class.2
// CHECK-NEXT:                ArrayPattern : %class.2
// CHECK-NEXT:                    RestElement
// CHECK-NEXT:                        Id 'all' [D:E:%d.8 'all'] : %class.2
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                Id 'arr' [D:E:%d.2 'arr'] : %class.2
// CHECK-NEXT:                ArrayPattern : %class.2
// CHECK-NEXT:                    Id 'e' [D:E:%d.9 'e'] : number
// CHECK-NEXT:                    RestElement
// CHECK-NEXT:                        Id 'erest' [D:E:%d.10 'erest'] : %class.2
// CHECK-NEXT:        FunctionDeclaration : %function.5
// CHECK-NEXT:            Id 'f' [D:E:%d.11 'f']
// CHECK-NEXT:            ArrayPattern : %class.4
// CHECK-NEXT:                Id 'h' [D:E:%d.19 'h'] : string
// CHECK-NEXT:                RestElement
// CHECK-NEXT:                    Id 't' [D:E:%d.20 't'] : %class.4
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                NumericLiteral : number
// CHECK-NEXT:                Id 'x' [D:E:%d.12 'x']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                ArrayExpression : %class.2
// CHECK-NEXT:                Id 'r' [D:E:%d.13 'r']
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            AssignmentExpression : %class.2
// CHECK-NEXT:                ArrayPattern : %class.2
// CHECK-NEXT:                    Id 'x' [D:E:%d.12 'x'] : number
// CHECK-NEXT:                    RestElement
// CHECK-NEXT:                        Id 'r' [D:E:%d.13 'r'] : %class.2
// CHECK-NEXT:                Id 'arr' [D:E:%d.2 'arr'] : %class.2
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            AssignmentExpression : %class.2
// CHECK-NEXT:                ArrayPattern : %class.2
// CHECK-NEXT:                    RestElement
// CHECK-NEXT:                        Id 'r' [D:E:%d.13 'r'] : %class.2
// CHECK-NEXT:                Id 'arr' [D:E:%d.2 'arr'] : %class.2
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                NumericLiteral : number
// CHECK-NEXT:                Id 'y' [D:E:%d.14 'y']
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            AssignmentExpression : %class.2
// CHECK-NEXT:                ArrayPattern : %class.2
// CHECK-NEXT:                    Id 'x' [D:E:%d.12 'x'] : number
// CHECK-NEXT:                    Id 'y' [D:E:%d.14 'y'] : number
// CHECK-NEXT:                Id 'arr' [D:E:%d.2 'arr'] : %class.2
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                ArrayExpression : %class.3
// CHECK-NEXT:                    ArrayExpression : %class.2
// CHECK-NEXT:                        NumericLiteral : number
// CHECK-NEXT:                        NumericLiteral : number
// CHECK-NEXT:                Id 'nested' [D:E:%d.15 'nested']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                Id 'nested' [D:E:%d.15 'nested'] : %class.3
// CHECK-NEXT:                ArrayPattern : %class.3
// CHECK-NEXT:                    ArrayPattern : %class.2
// CHECK-NEXT:                        Id 'na' [D:E:%d.16 'na'] : number
// CHECK-NEXT:                        Id 'nb' [D:E:%d.17 'nb'] : number
