/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror --typed --dump-sema -fno-std-globals %s | %FileCheckOrRegen %s --match-full-lines

'use strict';

// A const keeps the literal type, a let/var widens to number.
const k = 1;
let w = 1;
w = 2;

// Explicit numeric literal type annotations.
type T = 1 | 2;
let t: T = 1;
t = 2;

// A union of numeric literals flows into number.
let s: number = t;

// A fresh numeric literal flows into number.
let s2: number = 42;

// An un-annotated object literal widens its fresh literal field types, because
// object fields are mutable (matches let/var widening, so the reassignment
// below is allowed).
let o = {a: 1};
o.a = 2;

// An un-annotated indexer object widens its fresh literal value types too.
function makeIndexer(k1: string, k2: string) {
  let m = {[k1]: 1, [k2]: 'v'};
  m[k1] = 2;
  return m;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%union.2 = union(1 | 2)
// CHECK-NEXT:%object.3 = object({
// CHECK-NEXT:  a: number
// CHECK-NEXT:})
// CHECK-NEXT:%function.4 = function(k1: string, k2: string): any
// CHECK-NEXT:%union.5 = union(string | number)
// CHECK-NEXT:%object.6 = object({
// CHECK-NEXT:  [string]: %union.5
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict mayReachImplicitReturn
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'k' Const : 1
// CHECK-NEXT:        Decl %d.3 'w' Let : number
// CHECK-NEXT:        Decl %d.4 't' Let : %union.2
// CHECK-NEXT:        Decl %d.5 's' Let : number
// CHECK-NEXT:        Decl %d.6 's2' Let : number
// CHECK-NEXT:        Decl %d.7 'o' Let : %object.3
// CHECK-NEXT:        Decl %d.8 'makeIndexer' Var : %function.4
// CHECK-NEXT:        Decl %d.9 'arguments' Var Arguments
// CHECK-NEXT:        hoistedFunction makeIndexer
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.10 'k1' Parameter : string
// CHECK-NEXT:            Decl %d.11 'k2' Parameter : string
// CHECK-NEXT:            Decl %d.12 'm' Let : %object.6
// CHECK-NEXT:            Decl %d.13 'arguments' Var Arguments

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            StringLiteral : "use strict"
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                NumericLiteral : 1
// CHECK-NEXT:                Id 'k' [D:E:%d.2 'k']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                NumericLiteral : 1
// CHECK-NEXT:                Id 'w' [D:E:%d.3 'w']
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            AssignmentExpression : 2
// CHECK-NEXT:                Id 'w' [D:E:%d.3 'w'] : number
// CHECK-NEXT:                NumericLiteral : 2
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'T'
// CHECK-NEXT:            UnionTypeAnnotation
// CHECK-NEXT:                NumberLiteralTypeAnnotation
// CHECK-NEXT:                NumberLiteralTypeAnnotation
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                NumericLiteral : 1
// CHECK-NEXT:                Id 't' [D:E:%d.4 't']
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            AssignmentExpression : 2
// CHECK-NEXT:                Id 't' [D:E:%d.4 't'] : %union.2
// CHECK-NEXT:                NumericLiteral : 2
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                Id 't' [D:E:%d.4 't'] : %union.2
// CHECK-NEXT:                Id 's' [D:E:%d.5 's']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                NumericLiteral : 42
// CHECK-NEXT:                Id 's2' [D:E:%d.6 's2']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                ObjectExpression : %object.3
// CHECK-NEXT:                    Property
// CHECK-NEXT:                        Id 'a'
// CHECK-NEXT:                        NumericLiteral : 1
// CHECK-NEXT:                Id 'o' [D:E:%d.7 'o']
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            AssignmentExpression : 2
// CHECK-NEXT:                MemberExpression : number
// CHECK-NEXT:                    Id 'o' [D:E:%d.7 'o'] : %object.3
// CHECK-NEXT:                    Id 'a'
// CHECK-NEXT:                NumericLiteral : 2
// CHECK-NEXT:        FunctionDeclaration : %function.4
// CHECK-NEXT:            Id 'makeIndexer' [D:E:%d.8 'makeIndexer']
// CHECK-NEXT:            Id 'k1' [D:E:%d.10 'k1']
// CHECK-NEXT:            Id 'k2' [D:E:%d.11 'k2']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                VariableDeclaration
// CHECK-NEXT:                    VariableDeclarator
// CHECK-NEXT:                        ObjectExpression : %object.6
// CHECK-NEXT:                            Property
// CHECK-NEXT:                                Id 'k1' [D:E:%d.10 'k1'] : string
// CHECK-NEXT:                                NumericLiteral : 1
// CHECK-NEXT:                            Property
// CHECK-NEXT:                                Id 'k2' [D:E:%d.11 'k2'] : string
// CHECK-NEXT:                                StringLiteral : "v"
// CHECK-NEXT:                        Id 'm' [D:E:%d.12 'm']
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    AssignmentExpression : 2
// CHECK-NEXT:                        MemberExpression : %union.5
// CHECK-NEXT:                            Id 'm' [D:E:%d.12 'm'] : %object.6
// CHECK-NEXT:                            Id 'k1' [D:E:%d.10 'k1'] : string
// CHECK-NEXT:                        NumericLiteral : 2
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    Id 'm' [D:E:%d.12 'm'] : %object.6
