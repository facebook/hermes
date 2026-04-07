/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -fno-std-globals -typed -dump-sema %s | %FileCheckOrRegen %s --match-full-lines

'use strict';

let s: string = "hello";
let len: number = s.length;
let ch: string = s[0];

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 's' Let : string
// CHECK-NEXT:        Decl %d.3 'len' Let : number
// CHECK-NEXT:        Decl %d.4 'ch' Let : string
// CHECK-NEXT:        Decl %d.5 'arguments' Var Arguments

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            StringLiteral : string
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                StringLiteral : string
// CHECK-NEXT:                Id 's' [D:E:%d.2 's']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                MemberExpression : number
// CHECK-NEXT:                    Id 's' [D:E:%d.2 's'] : string
// CHECK-NEXT:                    Id 'length'
// CHECK-NEXT:                Id 'len' [D:E:%d.3 'len']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                MemberExpression : string
// CHECK-NEXT:                    Id 's' [D:E:%d.2 's'] : string
// CHECK-NEXT:                    NumericLiteral : number
// CHECK-NEXT:                Id 'ch' [D:E:%d.4 'ch']
