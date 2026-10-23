/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror --typed --dump-sema -fno-std-globals %s | %FileCheckOrRegen %s --match-full-lines

'use strict';

// A literal arm is absorbed by its base type, by 'any' and by 'mixed'.
type A = string | 'abc';
type B = 'a' | 'b';
type C = 'a' | 'b' | string;
type D = any | 'a';
type E = mixed | 'a';
type F = ?('a' | string);
// Different base kind, so no absorption.
type G = number | 'a';

function decls(a: A, b: B, c: C, d: D, e: E, f: F, g: G) {}

// Absorption and widening interact: here widening does the work, because
// there's no 'string' arm to absorb the literals.
function widen(cond: boolean, s: string, anyVal: any) {
  let w1 = cond ? 'a' : 'b';
  // Here absorption does the work and widening is a no-op.
  let w2 = cond ? 'a' : s;
  // A const keeps the union of literals.
  const w3 = cond ? 'a' : 'b';
  // 'any' absorbs the literal, so this is 'any', not 'any | string'.
  let w4 = cond ? anyVal : 'a';
  let w5 = {f: cond ? 'a' : s};
}

// Narrowing through '?T' still reaches string.
function narrow(x: ?('a' | string)): string {
  return x;
}

// Operators treat a literal as its base type.
function ops(s: string): boolean {
  return ('a' + s) < 'b';
}

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%union.2 = union("a" | "b")
// CHECK-NEXT:%union.3 = union(void | null | string)
// CHECK-NEXT:%union.4 = union(number | "a")
// CHECK-NEXT:%function.5 = function(a: string, b: %union.2, c: string, d: any, e: mixed, f: %union.3, g: %union.4): any
// CHECK-NEXT:%function.6 = function(cond: boolean, s: string, anyVal: any): any
// CHECK-NEXT:%function.7 = function(x: %union.3): string
// CHECK-NEXT:%function.8 = function(s: string): boolean
// CHECK-NEXT:%object.9 = object({
// CHECK-NEXT:  f: string
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict mayReachImplicitReturn
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'decls' Var : %function.5
// CHECK-NEXT:        Decl %d.3 'widen' Var : %function.6
// CHECK-NEXT:        Decl %d.4 'narrow' Var : %function.7
// CHECK-NEXT:        Decl %d.5 'ops' Var : %function.8
// CHECK-NEXT:        Decl %d.6 'arguments' Var Arguments
// CHECK-NEXT:        hoistedFunction decls
// CHECK-NEXT:        hoistedFunction widen
// CHECK-NEXT:        hoistedFunction narrow
// CHECK-NEXT:        hoistedFunction ops
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.7 'a' Parameter : string
// CHECK-NEXT:            Decl %d.8 'b' Parameter : %union.2
// CHECK-NEXT:            Decl %d.9 'c' Parameter : string
// CHECK-NEXT:            Decl %d.10 'd' Parameter : any
// CHECK-NEXT:            Decl %d.11 'e' Parameter : mixed
// CHECK-NEXT:            Decl %d.12 'f' Parameter : %union.3
// CHECK-NEXT:            Decl %d.13 'g' Parameter : %union.4
// CHECK-NEXT:            Decl %d.14 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.3
// CHECK-NEXT:            Decl %d.15 'cond' Parameter : boolean
// CHECK-NEXT:            Decl %d.16 's' Parameter : string
// CHECK-NEXT:            Decl %d.17 'anyVal' Parameter : any
// CHECK-NEXT:            Decl %d.18 'w1' Let : string
// CHECK-NEXT:            Decl %d.19 'w2' Let : string
// CHECK-NEXT:            Decl %d.20 'w3' Const : %union.2
// CHECK-NEXT:            Decl %d.21 'w4' Let : any
// CHECK-NEXT:            Decl %d.22 'w5' Let : %object.9
// CHECK-NEXT:            Decl %d.23 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.4
// CHECK-NEXT:            Decl %d.24 'x' Parameter : %union.3
// CHECK-NEXT:            Decl %d.25 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.5
// CHECK-NEXT:            Decl %d.26 's' Parameter : string
// CHECK-NEXT:            Decl %d.27 'arguments' Var Arguments

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            StringLiteral : "use strict"
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'A'
// CHECK-NEXT:            UnionTypeAnnotation
// CHECK-NEXT:                StringTypeAnnotation
// CHECK-NEXT:                StringLiteralTypeAnnotation
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'B'
// CHECK-NEXT:            UnionTypeAnnotation
// CHECK-NEXT:                StringLiteralTypeAnnotation
// CHECK-NEXT:                StringLiteralTypeAnnotation
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'C'
// CHECK-NEXT:            UnionTypeAnnotation
// CHECK-NEXT:                StringLiteralTypeAnnotation
// CHECK-NEXT:                StringLiteralTypeAnnotation
// CHECK-NEXT:                StringTypeAnnotation
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'D'
// CHECK-NEXT:            UnionTypeAnnotation
// CHECK-NEXT:                AnyTypeAnnotation
// CHECK-NEXT:                StringLiteralTypeAnnotation
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'E'
// CHECK-NEXT:            UnionTypeAnnotation
// CHECK-NEXT:                MixedTypeAnnotation
// CHECK-NEXT:                StringLiteralTypeAnnotation
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'F'
// CHECK-NEXT:            NullableTypeAnnotation
// CHECK-NEXT:                UnionTypeAnnotation
// CHECK-NEXT:                    StringLiteralTypeAnnotation
// CHECK-NEXT:                    StringTypeAnnotation
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'G'
// CHECK-NEXT:            UnionTypeAnnotation
// CHECK-NEXT:                NumberTypeAnnotation
// CHECK-NEXT:                StringLiteralTypeAnnotation
// CHECK-NEXT:        FunctionDeclaration : %function.5
// CHECK-NEXT:            Id 'decls' [D:E:%d.2 'decls']
// CHECK-NEXT:            Id 'a' [D:E:%d.7 'a']
// CHECK-NEXT:            Id 'b' [D:E:%d.8 'b']
// CHECK-NEXT:            Id 'c' [D:E:%d.9 'c']
// CHECK-NEXT:            Id 'd' [D:E:%d.10 'd']
// CHECK-NEXT:            Id 'e' [D:E:%d.11 'e']
// CHECK-NEXT:            Id 'f' [D:E:%d.12 'f']
// CHECK-NEXT:            Id 'g' [D:E:%d.13 'g']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:        FunctionDeclaration : %function.6
// CHECK-NEXT:            Id 'widen' [D:E:%d.3 'widen']
// CHECK-NEXT:            Id 'cond' [D:E:%d.15 'cond']
// CHECK-NEXT:            Id 's' [D:E:%d.16 's']
// CHECK-NEXT:            Id 'anyVal' [D:E:%d.17 'anyVal']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                VariableDeclaration
// CHECK-NEXT:                    VariableDeclarator
// CHECK-NEXT:                        ConditionalExpression : %union.2
// CHECK-NEXT:                            Id 'cond' [D:E:%d.15 'cond'] : boolean
// CHECK-NEXT:                            StringLiteral : "b"
// CHECK-NEXT:                            StringLiteral : "a"
// CHECK-NEXT:                        Id 'w1' [D:E:%d.18 'w1']
// CHECK-NEXT:                VariableDeclaration
// CHECK-NEXT:                    VariableDeclarator
// CHECK-NEXT:                        ConditionalExpression : string
// CHECK-NEXT:                            Id 'cond' [D:E:%d.15 'cond'] : boolean
// CHECK-NEXT:                            Id 's' [D:E:%d.16 's'] : string
// CHECK-NEXT:                            StringLiteral : "a"
// CHECK-NEXT:                        Id 'w2' [D:E:%d.19 'w2']
// CHECK-NEXT:                VariableDeclaration
// CHECK-NEXT:                    VariableDeclarator
// CHECK-NEXT:                        ConditionalExpression : %union.2
// CHECK-NEXT:                            Id 'cond' [D:E:%d.15 'cond'] : boolean
// CHECK-NEXT:                            StringLiteral : "b"
// CHECK-NEXT:                            StringLiteral : "a"
// CHECK-NEXT:                        Id 'w3' [D:E:%d.20 'w3']
// CHECK-NEXT:                VariableDeclaration
// CHECK-NEXT:                    VariableDeclarator
// CHECK-NEXT:                        ConditionalExpression : any
// CHECK-NEXT:                            Id 'cond' [D:E:%d.15 'cond'] : boolean
// CHECK-NEXT:                            StringLiteral : "a"
// CHECK-NEXT:                            Id 'anyVal' [D:E:%d.17 'anyVal'] : any
// CHECK-NEXT:                        Id 'w4' [D:E:%d.21 'w4']
// CHECK-NEXT:                VariableDeclaration
// CHECK-NEXT:                    VariableDeclarator
// CHECK-NEXT:                        ObjectExpression : %object.9
// CHECK-NEXT:                            Property
// CHECK-NEXT:                                Id 'f'
// CHECK-NEXT:                                ConditionalExpression : string
// CHECK-NEXT:                                    Id 'cond' [D:E:%d.15 'cond'] : boolean
// CHECK-NEXT:                                    Id 's' [D:E:%d.16 's'] : string
// CHECK-NEXT:                                    StringLiteral : "a"
// CHECK-NEXT:                        Id 'w5' [D:E:%d.22 'w5']
// CHECK-NEXT:        FunctionDeclaration : %function.7
// CHECK-NEXT:            Id 'narrow' [D:E:%d.4 'narrow']
// CHECK-NEXT:            Id 'x' [D:E:%d.24 'x']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    ImplicitCheckedCast : string
// CHECK-NEXT:                        Id 'x' [D:E:%d.24 'x'] : %union.3
// CHECK-NEXT:        FunctionDeclaration : %function.8
// CHECK-NEXT:            Id 'ops' [D:E:%d.5 'ops']
// CHECK-NEXT:            Id 's' [D:E:%d.26 's']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    BinaryExpression : boolean
// CHECK-NEXT:                        BinaryExpression : string
// CHECK-NEXT:                            StringLiteral : "a"
// CHECK-NEXT:                            BinOp +
// CHECK-NEXT:                            Id 's' [D:E:%d.26 's'] : string
// CHECK-NEXT:                        StringLiteral : "b"
