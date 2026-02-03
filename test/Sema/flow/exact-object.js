/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -fno-std-globals -typed -dump-sema %s | %FileCheckOrRegen %s --match-full-lines

type T = {
  x: number,
};

let t: {x: number} = {x: 3};
let t2: T = t;
let tx: number = t.x;
let t2x: number = t2.x;
t.x = 5;

let tdup: {x: string, y: bool} = {x: '3', x: 'hi', y: true};

type TRec = {
  x: TRec | null,
}

let trec: TRec;

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%object.2 = object({
// CHECK-NEXT:  x: number
// CHECK-NEXT:})
// CHECK-NEXT:%object.3 = object({
// CHECK-NEXT:  x: %union.4
// CHECK-NEXT:})
// CHECK-NEXT:%union.4 = union(null | %object.3)
// CHECK-NEXT:%object.5 = object({
// CHECK-NEXT:  x: string
// CHECK-NEXT:  y: boolean
// CHECK-NEXT:})
// CHECK-NEXT:%object.6 = object({
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 't' Let : %object.2
// CHECK-NEXT:            Decl %d.3 't2' Let : %object.2
// CHECK-NEXT:            Decl %d.4 'tx' Let : number
// CHECK-NEXT:            Decl %d.5 't2x' Let : number
// CHECK-NEXT:            Decl %d.6 'tdup' Let : %object.5
// CHECK-NEXT:            Decl %d.7 'trec' Let : %object.3
// CHECK-NEXT:            Decl %d.8 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    TypeAlias
// CHECK-NEXT:                        Id 'T'
// CHECK-NEXT:                        ObjectTypeAnnotation
// CHECK-NEXT:                            ObjectTypeProperty
// CHECK-NEXT:                                Id 'x'
// CHECK-NEXT:                                NumberTypeAnnotation
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            ObjectExpression : %object.2
// CHECK-NEXT:                                Property
// CHECK-NEXT:                                    Id 'x'
// CHECK-NEXT:                                    NumericLiteral : number
// CHECK-NEXT:                            Id 't' [D:E:%d.2 't']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            Id 't' [D:E:%d.2 't'] : %object.2
// CHECK-NEXT:                            Id 't2' [D:E:%d.3 't2']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            MemberExpression : number
// CHECK-NEXT:                                Id 't' [D:E:%d.2 't'] : %object.2
// CHECK-NEXT:                                Id 'x'
// CHECK-NEXT:                            Id 'tx' [D:E:%d.4 'tx']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            MemberExpression : number
// CHECK-NEXT:                                Id 't2' [D:E:%d.3 't2'] : %object.2
// CHECK-NEXT:                                Id 'x'
// CHECK-NEXT:                            Id 't2x' [D:E:%d.5 't2x']
// CHECK-NEXT:                    ExpressionStatement
// CHECK-NEXT:                        AssignmentExpression : number
// CHECK-NEXT:                            MemberExpression : number
// CHECK-NEXT:                                Id 't' [D:E:%d.2 't'] : %object.2
// CHECK-NEXT:                                Id 'x'
// CHECK-NEXT:                            NumericLiteral : number
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            ObjectExpression : %object.5
// CHECK-NEXT:                                Property
// CHECK-NEXT:                                    Id 'x'
// CHECK-NEXT:                                    StringLiteral : string
// CHECK-NEXT:                                Property
// CHECK-NEXT:                                    Id 'x'
// CHECK-NEXT:                                    StringLiteral : string
// CHECK-NEXT:                                Property
// CHECK-NEXT:                                    Id 'y'
// CHECK-NEXT:                                    BooleanLiteral : boolean
// CHECK-NEXT:                            Id 'tdup' [D:E:%d.6 'tdup']
// CHECK-NEXT:                    TypeAlias
// CHECK-NEXT:                        Id 'TRec'
// CHECK-NEXT:                        ObjectTypeAnnotation
// CHECK-NEXT:                            ObjectTypeProperty
// CHECK-NEXT:                                Id 'x'
// CHECK-NEXT:                                UnionTypeAnnotation
// CHECK-NEXT:                                    GenericTypeAnnotation
// CHECK-NEXT:                                        Id 'TRec'
// CHECK-NEXT:                                    NullLiteralTypeAnnotation
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            Id 'trec' [D:E:%d.7 'trec']
// CHECK-NEXT:            ObjectExpression : %object.6
