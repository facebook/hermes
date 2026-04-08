/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals --typed --dump-sema %s | %FileCheckOrRegen %s --match-full-lines

// Constraint from a destructuring pattern's type annotation should flow into
// the initializer literal, inferring it as a tuple (not an array).

'use strict';

let [a, b]: [number, string] = [1, "hello"];
let [p, {q}]: [number, {q: string}] = [1, {q: "a"}];

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%tuple.2 = tuple(number, string)
// CHECK-NEXT:%object.3 = object({
// CHECK-NEXT:  q: string
// CHECK-NEXT:})
// CHECK-NEXT:%tuple.4 = tuple(number, %object.3)

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'a' Let : number
// CHECK-NEXT:        Decl %d.3 'b' Let : string
// CHECK-NEXT:        Decl %d.4 'p' Let : number
// CHECK-NEXT:        Decl %d.5 'q' Let : string
// CHECK-NEXT:        Decl %d.6 'arguments' Var Arguments

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            StringLiteral : string
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                ArrayExpression : %tuple.2
// CHECK-NEXT:                    NumericLiteral : number
// CHECK-NEXT:                    StringLiteral : string
// CHECK-NEXT:                ArrayPattern : %tuple.2
// CHECK-NEXT:                    Id 'a' [D:E:%d.2 'a'] : number
// CHECK-NEXT:                    Id 'b' [D:E:%d.3 'b'] : string
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                ArrayExpression : %tuple.4
// CHECK-NEXT:                    NumericLiteral : number
// CHECK-NEXT:                    ObjectExpression : %object.3
// CHECK-NEXT:                        Property
// CHECK-NEXT:                            Id 'q'
// CHECK-NEXT:                            StringLiteral : string
// CHECK-NEXT:                ArrayPattern : %tuple.4
// CHECK-NEXT:                    Id 'p' [D:E:%d.4 'p'] : number
// CHECK-NEXT:                    ObjectPattern : %object.3
// CHECK-NEXT:                        Property
// CHECK-NEXT:                            Id 'q'
// CHECK-NEXT:                            Id 'q' [D:E:%d.5 'q'] : string
