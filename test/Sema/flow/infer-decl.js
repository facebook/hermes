/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals --typed --dump-sema %s | %FileCheckOrRegen %s --match-full-lines

'use strict';

let x = 0;
let y = x;
let z: number | string = x;
let zz = foo(function name() {});

// Auto-generated content below. Please do not modify manually.

// CHECK:untyped function %t.1 = untyped function ()
// CHECK-NEXT:union %t.2 = union string | number

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'foo' UndeclaredGlobalProperty
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.2 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.3 'x' Let : number
// CHECK-NEXT:            Decl %d.4 'y' Let : number
// CHECK-NEXT:            Decl %d.5 'z' Let : union %t.2
// CHECK-NEXT:            Decl %d.6 'zz' Let : any
// CHECK-NEXT:            Decl %d.7 'arguments' Var Arguments
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:                Decl %d.8 'name' FunctionExprName : untyped function %t.1
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.4
// CHECK-NEXT:                Decl %d.9 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : untyped function %t.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.2 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    ExpressionStatement
// CHECK-NEXT:                        StringLiteral : string
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            NumericLiteral : number
// CHECK-NEXT:                            Id 'x' [D:E:%d.3 'x']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            Id 'x' [D:E:%d.3 'x'] : number
// CHECK-NEXT:                            Id 'y' [D:E:%d.4 'y']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            Id 'x' [D:E:%d.3 'x'] : number
// CHECK-NEXT:                            Id 'z' [D:E:%d.5 'z']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            CallExpression
// CHECK-NEXT:                                Id 'foo' [D:E:%d.1 'foo'] : any
// CHECK-NEXT:                                FunctionExpression : untyped function %t.1 Scope %s.3
// CHECK-NEXT:                                    Id 'name' [D:E:%d.8 'name']
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                            Id 'zz' [D:E:%d.6 'zz']
// CHECK-NEXT:            ObjectExpression
