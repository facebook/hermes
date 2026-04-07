/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals --typed --dump-sema %s | %FileCheckOrRegen %s --match-full-lines

'use strict';

let a: {p: number | null} = {p: 10};
let b: {p: {q: number | null}} = {p: {q: 10}};

type T = {x: T | null};
let t1: T = {x: null};

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%object.2 = object({
// CHECK-NEXT:  x: %union.6
// CHECK-NEXT:})
// CHECK-NEXT:%object.3 = object({
// CHECK-NEXT:  p: %union.7
// CHECK-NEXT:})
// CHECK-NEXT:%object.4 = object({
// CHECK-NEXT:  p: %object.5
// CHECK-NEXT:})
// CHECK-NEXT:%object.5 = object({
// CHECK-NEXT:  q: %union.7
// CHECK-NEXT:})
// CHECK-NEXT:%union.6 = union(null | %object.2)
// CHECK-NEXT:%union.7 = union(null | number)

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'a' Let : %object.3
// CHECK-NEXT:        Decl %d.3 'b' Let : %object.4
// CHECK-NEXT:        Decl %d.4 't1' Let : %object.2
// CHECK-NEXT:        Decl %d.5 'arguments' Var Arguments

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            StringLiteral : string
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                ObjectExpression : %object.3
// CHECK-NEXT:                    Property
// CHECK-NEXT:                        Id 'p'
// CHECK-NEXT:                        NumericLiteral : number
// CHECK-NEXT:                Id 'a' [D:E:%d.2 'a']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                ObjectExpression : %object.4
// CHECK-NEXT:                    Property
// CHECK-NEXT:                        Id 'p'
// CHECK-NEXT:                        ObjectExpression : %object.5
// CHECK-NEXT:                            Property
// CHECK-NEXT:                                Id 'q'
// CHECK-NEXT:                                NumericLiteral : number
// CHECK-NEXT:                Id 'b' [D:E:%d.3 'b']
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'T'
// CHECK-NEXT:            ObjectTypeAnnotation
// CHECK-NEXT:                ObjectTypeProperty
// CHECK-NEXT:                    Id 'x'
// CHECK-NEXT:                    UnionTypeAnnotation
// CHECK-NEXT:                        GenericTypeAnnotation
// CHECK-NEXT:                            Id 'T'
// CHECK-NEXT:                        NullLiteralTypeAnnotation
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                ObjectExpression : %object.2
// CHECK-NEXT:                    Property
// CHECK-NEXT:                        Id 'x'
// CHECK-NEXT:                        NullLiteral : null
// CHECK-NEXT:                Id 't1' [D:E:%d.4 't1']
