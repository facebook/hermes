/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror --typed --dump-sema -fno-std-globals %s | %FileCheckOrRegen %s --match-full-lines

'use strict';

// Flow/TS-style discriminated unions: objects with a string-literal-typed tag.
type Circle = {tag: 'circle', radius: number};
type Square = {tag: 'square', side: number};
type Shape = Circle | Square;

let c: Circle = {tag: 'circle', radius: 1};
let s: Square = {tag: 'square', side: 2};

let shape: Shape = c;

// The discriminant of a single arm has the literal type.
let cc: Circle = {tag: 'circle', radius: 3};
let tag: 'circle' = cc.tag;

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%object.2 = object({
// CHECK-NEXT:  tag: "circle"
// CHECK-NEXT:  radius: number
// CHECK-NEXT:})
// CHECK-NEXT:%object.3 = object({
// CHECK-NEXT:  tag: "square"
// CHECK-NEXT:  side: number
// CHECK-NEXT:})
// CHECK-NEXT:%union.4 = union(%object.2 | %object.3)

// CHECK:SemContext
// CHECK-NEXT:Func strict mayReachImplicitReturn
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'c' Let : %object.2
// CHECK-NEXT:        Decl %d.3 's' Let : %object.3
// CHECK-NEXT:        Decl %d.4 'shape' Let : %union.4
// CHECK-NEXT:        Decl %d.5 'cc' Let : %object.2
// CHECK-NEXT:        Decl %d.6 'tag' Let : "circle"
// CHECK-NEXT:        Decl %d.7 'arguments' Var Arguments

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            StringLiteral : "use strict"
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'Circle'
// CHECK-NEXT:            ObjectTypeAnnotation
// CHECK-NEXT:                ObjectTypeProperty
// CHECK-NEXT:                    Id 'tag'
// CHECK-NEXT:                    StringLiteralTypeAnnotation
// CHECK-NEXT:                ObjectTypeProperty
// CHECK-NEXT:                    Id 'radius'
// CHECK-NEXT:                    NumberTypeAnnotation
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'Square'
// CHECK-NEXT:            ObjectTypeAnnotation
// CHECK-NEXT:                ObjectTypeProperty
// CHECK-NEXT:                    Id 'tag'
// CHECK-NEXT:                    StringLiteralTypeAnnotation
// CHECK-NEXT:                ObjectTypeProperty
// CHECK-NEXT:                    Id 'side'
// CHECK-NEXT:                    NumberTypeAnnotation
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'Shape'
// CHECK-NEXT:            UnionTypeAnnotation
// CHECK-NEXT:                GenericTypeAnnotation
// CHECK-NEXT:                    Id 'Circle'
// CHECK-NEXT:                GenericTypeAnnotation
// CHECK-NEXT:                    Id 'Square'
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                ObjectExpression : %object.2
// CHECK-NEXT:                    Property
// CHECK-NEXT:                        Id 'tag'
// CHECK-NEXT:                        StringLiteral : "circle"
// CHECK-NEXT:                    Property
// CHECK-NEXT:                        Id 'radius'
// CHECK-NEXT:                        NumericLiteral : 1
// CHECK-NEXT:                Id 'c' [D:E:%d.2 'c']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                ObjectExpression : %object.3
// CHECK-NEXT:                    Property
// CHECK-NEXT:                        Id 'tag'
// CHECK-NEXT:                        StringLiteral : "square"
// CHECK-NEXT:                    Property
// CHECK-NEXT:                        Id 'side'
// CHECK-NEXT:                        NumericLiteral : 2
// CHECK-NEXT:                Id 's' [D:E:%d.3 's']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                Id 'c' [D:E:%d.2 'c'] : %object.2
// CHECK-NEXT:                Id 'shape' [D:E:%d.4 'shape']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                ObjectExpression : %object.2
// CHECK-NEXT:                    Property
// CHECK-NEXT:                        Id 'tag'
// CHECK-NEXT:                        StringLiteral : "circle"
// CHECK-NEXT:                    Property
// CHECK-NEXT:                        Id 'radius'
// CHECK-NEXT:                        NumericLiteral : 3
// CHECK-NEXT:                Id 'cc' [D:E:%d.5 'cc']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                MemberExpression : "circle"
// CHECK-NEXT:                    Id 'cc' [D:E:%d.5 'cc'] : %object.2
// CHECK-NEXT:                    Id 'tag'
// CHECK-NEXT:                Id 'tag' [D:E:%d.6 'tag']
