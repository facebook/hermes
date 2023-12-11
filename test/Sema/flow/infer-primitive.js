/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -fno-std-globals --typed --dump-sema %s | %FileCheckOrRegen %s --match-full-lines

'use strict';

let v1 = 1;
let v2 = 1n;
let v3 = '1';
let v4 = `1`;
let v5 = true;
let v6 = null;
let v7 = /1/;

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'v1' Let : number
// CHECK-NEXT:            Decl %d.3 'v2' Let : bigint
// CHECK-NEXT:            Decl %d.4 'v3' Let : string
// CHECK-NEXT:            Decl %d.5 'v4' Let : string
// CHECK-NEXT:            Decl %d.6 'v5' Let : boolean
// CHECK-NEXT:            Decl %d.7 'v6' Let : null
// CHECK-NEXT:            Decl %d.8 'v7' Let : any
// CHECK-NEXT:            Decl %d.9 'arguments' Var Arguments

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
// CHECK-NEXT:                            NumericLiteral : number
// CHECK-NEXT:                            Id 'v1' [D:E:%d.2 'v1']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            BigIntLiteral : bigint
// CHECK-NEXT:                            Id 'v2' [D:E:%d.3 'v2']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            StringLiteral : string
// CHECK-NEXT:                            Id 'v3' [D:E:%d.4 'v3']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            TemplateLiteral : string
// CHECK-NEXT:                                TemplateElement
// CHECK-NEXT:                            Id 'v4' [D:E:%d.5 'v4']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            BooleanLiteral : boolean
// CHECK-NEXT:                            Id 'v5' [D:E:%d.6 'v5']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            NullLiteral : null
// CHECK-NEXT:                            Id 'v6' [D:E:%d.7 'v6']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            RegExpLiteral : any
// CHECK-NEXT:                            Id 'v7' [D:E:%d.8 'v7']
// CHECK-NEXT:            ObjectExpression
