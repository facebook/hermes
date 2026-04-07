/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes --typed --dump-sema %s | %FileCheckOrRegen %s --match-full-lines

'use strict';

let x = 0;
let y = x;
let z: number | string = x;
let zz = foo(function name() {});

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%union.2 = union(string | number)

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'x' Let : number
// CHECK-NEXT:        Decl %d.3 'y' Let : number
// CHECK-NEXT:        Decl %d.4 'z' Let : %union.2
// CHECK-NEXT:        Decl %d.5 'zz' Let : any
// CHECK-NEXT:        Decl %d.6 'arguments' Var Arguments
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.7 'name' FunctionExprName : %untyped_function.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.3
// CHECK-NEXT:            Decl %d.8 'arguments' Var Arguments

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            StringLiteral : string
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                NumericLiteral : number
// CHECK-NEXT:                Id 'x' [D:E:%d.2 'x']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                Id 'x' [D:E:%d.2 'x'] : number
// CHECK-NEXT:                Id 'y' [D:E:%d.3 'y']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                Id 'x' [D:E:%d.2 'x'] : number
// CHECK-NEXT:                Id 'z' [D:E:%d.4 'z']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                CallExpression
// CHECK-NEXT:                    Id 'foo' [D:E:%d.9 'foo'] : any
// CHECK-NEXT:                    FunctionExpression : %untyped_function.1 Scope %s.2
// CHECK-NEXT:                        Id 'name' [D:E:%d.7 'name']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                Id 'zz' [D:E:%d.5 'zz']
