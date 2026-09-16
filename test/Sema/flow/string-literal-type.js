/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror --typed --dump-sema -fno-std-globals %s | %FileCheckOrRegen %s --match-full-lines

'use strict';

// A const keeps the literal type, a let/var widens to string.
const k = 'foo';
let w = 'foo';
w = 'bar';

// Explicit string literal type annotations.
type T = 'a' | 'b';
let t: T = 'a';
t = 'b';

// A union of string literals flows into string.
let s: string = t;

// A fresh string literal flows into string.
let s2: string = 'literal';

// An un-annotated object literal widens its fresh literal field types, because
// object fields are mutable (matches let/var widening, so the reassignment
// below is allowed).
let o = {a: 'x'};
o.a = 'y';

// An un-annotated indexer object widens its fresh literal value types too.
function makeIndexer(k1: string, k2: string) {
  let m = {[k1]: 1, [k2]: 'v'};
  m[k1] = 'w';
  return m;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%union.2 = union("a" | "b")
// CHECK-NEXT:%function.3 = function(k1: string, k2: string): any
// CHECK-NEXT:%object.4 = object({
// CHECK-NEXT:  a: string
// CHECK-NEXT:})
// CHECK-NEXT:%union.5 = union(string | number)
// CHECK-NEXT:%object.6 = object({
// CHECK-NEXT:  [string]: %union.5
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict mayReachImplicitReturn
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'k' Const : "foo"
// CHECK-NEXT:        Decl %d.3 'w' Let : string
// CHECK-NEXT:        Decl %d.4 't' Let : %union.2
// CHECK-NEXT:        Decl %d.5 's' Let : string
// CHECK-NEXT:        Decl %d.6 's2' Let : string
// CHECK-NEXT:        Decl %d.7 'o' Let : %object.4
// CHECK-NEXT:        Decl %d.8 'makeIndexer' Var : %function.3
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
// CHECK-NEXT:                StringLiteral : "foo"
// CHECK-NEXT:                Id 'k' [D:E:%d.2 'k']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                StringLiteral : "foo"
// CHECK-NEXT:                Id 'w' [D:E:%d.3 'w']
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            AssignmentExpression : "bar"
// CHECK-NEXT:                Id 'w' [D:E:%d.3 'w'] : string
// CHECK-NEXT:                StringLiteral : "bar"
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'T'
// CHECK-NEXT:            UnionTypeAnnotation
// CHECK-NEXT:                StringLiteralTypeAnnotation
// CHECK-NEXT:                StringLiteralTypeAnnotation
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                StringLiteral : "a"
// CHECK-NEXT:                Id 't' [D:E:%d.4 't']
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            AssignmentExpression : "b"
// CHECK-NEXT:                Id 't' [D:E:%d.4 't'] : %union.2
// CHECK-NEXT:                StringLiteral : "b"
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                Id 't' [D:E:%d.4 't'] : %union.2
// CHECK-NEXT:                Id 's' [D:E:%d.5 's']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                StringLiteral : "literal"
// CHECK-NEXT:                Id 's2' [D:E:%d.6 's2']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                ObjectExpression : %object.4
// CHECK-NEXT:                    Property
// CHECK-NEXT:                        Id 'a'
// CHECK-NEXT:                        StringLiteral : "x"
// CHECK-NEXT:                Id 'o' [D:E:%d.7 'o']
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            AssignmentExpression : "y"
// CHECK-NEXT:                MemberExpression : string
// CHECK-NEXT:                    Id 'o' [D:E:%d.7 'o'] : %object.4
// CHECK-NEXT:                    Id 'a'
// CHECK-NEXT:                StringLiteral : "y"
// CHECK-NEXT:        FunctionDeclaration : %function.3
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
// CHECK-NEXT:                    AssignmentExpression : "w"
// CHECK-NEXT:                        MemberExpression : %union.5
// CHECK-NEXT:                            Id 'm' [D:E:%d.12 'm'] : %object.6
// CHECK-NEXT:                            Id 'k1' [D:E:%d.10 'k1'] : string
// CHECK-NEXT:                        StringLiteral : "w"
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    Id 'm' [D:E:%d.12 'm'] : %object.6
