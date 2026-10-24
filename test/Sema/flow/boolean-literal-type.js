/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -typed -dump-sema %s | %FileCheckOrRegen --match-full-lines %s

// Boolean literal types; `true | false` canonicalizes to boolean.

'use strict';

type TF = true | false;
function f(b: boolean): true | false {
  return b;
}
function g(b: TF): boolean {
  return b;
}
function wrap<T>(x: T): T[] {
  return [x];
}
const n: number = 1;
const ok = n > 0.5 ? true : false;
let arr: boolean[] = wrap(ok);

// Annotations keep the literal; let widens, const doesn't.
const t: true = true;
let l = false;
const c = false;
l = true;

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%class.2 = class(Array<boolean>)
// CHECK-NEXT:%function.3 = function(b: boolean): boolean
// CHECK-NEXT:%function.4 = function(x: boolean): %class.2

// CHECK:SemContext
// CHECK-NEXT:Func strict mayReachImplicitReturn
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'f' Var : %function.3
// CHECK-NEXT:        Decl %d.3 'g' Var : %function.3
// CHECK-NEXT:        Decl %d.4 'wrap' Var
// CHECK-NEXT:        Decl %d.5 'n' Const : number
// CHECK-NEXT:        Decl %d.6 'ok' Const : boolean
// CHECK-NEXT:        Decl %d.7 'arr' Let : %class.2
// CHECK-NEXT:        Decl %d.8 't' Const : true
// CHECK-NEXT:        Decl %d.9 'l' Let : boolean
// CHECK-NEXT:        Decl %d.10 'c' Const : false
// CHECK-NEXT:        Decl %d.11 'arguments' Var Arguments
// CHECK-NEXT:        Decl %d.12 'wrap' Var : %function.4
// CHECK-NEXT:        hoistedFunction f
// CHECK-NEXT:        hoistedFunction g
// CHECK-NEXT:        hoistedFunction wrap
// CHECK-NEXT:        hoistedFunction wrap
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.13 'b' Parameter : boolean
// CHECK-NEXT:            Decl %d.14 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.3
// CHECK-NEXT:            Decl %d.15 'b' Parameter : boolean
// CHECK-NEXT:            Decl %d.16 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.4
// CHECK-NEXT:            Decl %d.17 'x' Parameter
// CHECK-NEXT:            Decl %d.18 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.5
// CHECK-NEXT:            Decl %d.19 'x' Parameter : boolean
// CHECK-NEXT:            Decl %d.20 'arguments' Var Arguments

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            StringLiteral : "use strict"
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'TF'
// CHECK-NEXT:            UnionTypeAnnotation
// CHECK-NEXT:                BooleanLiteralTypeAnnotation
// CHECK-NEXT:                BooleanLiteralTypeAnnotation
// CHECK-NEXT:        FunctionDeclaration : %function.3
// CHECK-NEXT:            Id 'f' [D:E:%d.2 'f']
// CHECK-NEXT:            Id 'b' [D:E:%d.13 'b']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    Id 'b' [D:E:%d.13 'b'] : boolean
// CHECK-NEXT:        FunctionDeclaration : %function.3
// CHECK-NEXT:            Id 'g' [D:E:%d.3 'g']
// CHECK-NEXT:            Id 'b' [D:E:%d.15 'b']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    Id 'b' [D:E:%d.15 'b'] : boolean
// CHECK-NEXT:        FunctionDeclaration : %function.4
// CHECK-NEXT:            Id 'wrap' [D:E:%d.12 'wrap']
// CHECK-NEXT:            Id 'x' [D:E:%d.19 'x']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    ArrayExpression : %class.2
// CHECK-NEXT:                        Id 'x' [D:E:%d.19 'x'] : boolean
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:        FunctionDeclaration
// CHECK-NEXT:            Id 'wrap' [D:E:%d.4 'wrap']
// CHECK-NEXT:            Id 'x' [D:E:%d.17 'x']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    ArrayExpression
// CHECK-NEXT:                        Id 'x' [D:E:%d.17 'x']
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                NumericLiteral : 1
// CHECK-NEXT:                Id 'n' [D:E:%d.5 'n']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                ConditionalExpression : boolean
// CHECK-NEXT:                    BinaryExpression : boolean
// CHECK-NEXT:                        Id 'n' [D:E:%d.5 'n'] : number
// CHECK-NEXT:                        NumericLiteral : 0.5
// CHECK-NEXT:                    BooleanLiteral : false
// CHECK-NEXT:                    BooleanLiteral : true
// CHECK-NEXT:                Id 'ok' [D:E:%d.6 'ok']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                CallExpression : %class.2
// CHECK-NEXT:                    Id 'wrap' [D:E:%d.12 'wrap'] : %function.4
// CHECK-NEXT:                    Id 'ok' [D:E:%d.6 'ok'] : boolean
// CHECK-NEXT:                Id 'arr' [D:E:%d.7 'arr']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                BooleanLiteral : true
// CHECK-NEXT:                Id 't' [D:E:%d.8 't']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                BooleanLiteral : false
// CHECK-NEXT:                Id 'l' [D:E:%d.9 'l']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                BooleanLiteral : false
// CHECK-NEXT:                Id 'c' [D:E:%d.10 'c']
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            AssignmentExpression : true
// CHECK-NEXT:                Id 'l' [D:E:%d.9 'l'] : boolean
// CHECK-NEXT:                BooleanLiteral : true
