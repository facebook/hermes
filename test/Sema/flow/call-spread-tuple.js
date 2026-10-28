/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -typed -dump-sema %s | %FileCheckOrRegen %s --match-full-lines

'use strict';

function f(a: number, b: string): void {}

function testCall(t: [number, string], empty: []): void {
  f(...t);
  // Spread mixed with ordinary arguments, in any position.
  f(...([1]: [number]), 'x');
  f(1, ...([ 'x' ]: [string]));
  // An empty tuple contributes no arguments.
  f(...t, ...empty);
}

// Tuple elements are absorbed by a rest parameter.
function rest(a: number, ...r: string[]): void {}
function testRest(t: [number, string, string]): void {
  rest(...t);
}

class C {
  x: number;
  constructor(a: number, b: string) {
    this.x = a;
  }
  m(a: number, b: string): void {}
}

class D extends C {
  constructor(p: [number, string]) {
    super(...p);
  }
}

function testMethodAndNew(t: [number, string]): void {
  new C(...t);
  new C(...t).m(...t);
}

// Arguments after a spread keep their own parameter as the contextual type.
class K {
  static SH_call(a: number, b: number, cb: (x: number) => number): void {}
}
function testSHCall(t: [number, number]): void {
  K(...t, x => x + 1);
}

function withThis(this: any, a: number, b: string): void {}
function testThisForms(t: [number, string]): void {
  withThis.call(undefined, ...t);
  $SHBuiltin.call(withThis, undefined, ...t);
}

// Explicit type arguments skip inference, which does not expand spreads.
function generic<T>(x: T): T {
  return x;
}
function testGeneric(t: [string]): string {
  return generic<string>(...t);
}

// Auto-generated content below. Please do not modify manually.

// CHECK:%class.1 = class(Array<string>)
// CHECK-NEXT:%untyped_function.2 = untyped_function()
// CHECK-NEXT:%class.3 = class(C {
// CHECK-NEXT:  %constructor: %function.7
// CHECK-NEXT:  %homeObject: %class.26
// CHECK-NEXT:  x: number
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.4 = class_constructor(%class.3)
// CHECK-NEXT:%class_constructor.5 = class_constructor(%class.27)
// CHECK-NEXT:%class_constructor.6 = class_constructor(%class.28)
// CHECK-NEXT:%function.7 = function(this: %class.3, a: number, b: string): void
// CHECK-NEXT:%tuple.8 = tuple(number, string)
// CHECK-NEXT:%function.9 = function(this: %class.27, p: %tuple.8): void
// CHECK-NEXT:%function.10 = function(x: number): number
// CHECK-NEXT:%function.11 = function(this: %class_constructor.6, a: number, b: number, cb: %function.10): void
// CHECK-NEXT:%function.12 = function(a: number, b: string): void
// CHECK-NEXT:%tuple.13 = tuple()
// CHECK-NEXT:%function.14 = function(t: %tuple.8, empty: %tuple.13): void
// CHECK-NEXT:%function.15 = function(a: number, ...r: %class.1): void
// CHECK-NEXT:%tuple.16 = tuple(number, string, string)
// CHECK-NEXT:%function.17 = function(t: %tuple.16): void
// CHECK-NEXT:%function.18 = function(t: %tuple.8): void
// CHECK-NEXT:%tuple.19 = tuple(number, number)
// CHECK-NEXT:%function.20 = function(t: %tuple.19): void
// CHECK-NEXT:%function.21 = function(this: any, a: number, b: string): void
// CHECK-NEXT:%tuple.22 = tuple(string)
// CHECK-NEXT:%function.23 = function(t: %tuple.22): string
// CHECK-NEXT:%tuple.24 = tuple(number)
// CHECK-NEXT:%function.25 = function(x: string): string
// CHECK-NEXT:%class.26 = class( {
// CHECK-NEXT:  m [final]: %function.7
// CHECK-NEXT:})
// CHECK-NEXT:%class.27 = class(D extends %class.3 {
// CHECK-NEXT:  %constructor: %function.9
// CHECK-NEXT:  %homeObject: %class.29
// CHECK-NEXT:})
// CHECK-NEXT:%class.28 = class(K {
// CHECK-NEXT:  %homeObject: %class.30
// CHECK-NEXT:  %staticObject: %class.31
// CHECK-NEXT:})
// CHECK-NEXT:%class.29 = class( extends %class.26 {
// CHECK-NEXT:})
// CHECK-NEXT:%class.30 = class( {
// CHECK-NEXT:})
// CHECK-NEXT:%class.31 = class( {
// CHECK-NEXT:  SH_call [final]: %function.11
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict mayReachImplicitReturn
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'f' Var : %function.12
// CHECK-NEXT:        Decl %d.3 'testCall' Var : %function.14
// CHECK-NEXT:        Decl %d.4 'rest' Var : %function.15
// CHECK-NEXT:        Decl %d.5 'testRest' Var : %function.17
// CHECK-NEXT:        Decl %d.6 'C' Class : %class_constructor.4
// CHECK-NEXT:        Decl %d.7 'D' Class : %class_constructor.5
// CHECK-NEXT:        Decl %d.8 'testMethodAndNew' Var : %function.18
// CHECK-NEXT:        Decl %d.9 'K' Class : %class_constructor.6
// CHECK-NEXT:        Decl %d.10 'testSHCall' Var : %function.20
// CHECK-NEXT:        Decl %d.11 'withThis' Var : %function.21
// CHECK-NEXT:        Decl %d.12 'testThisForms' Var : %function.18
// CHECK-NEXT:        Decl %d.13 'generic' Var
// CHECK-NEXT:        Decl %d.14 'testGeneric' Var : %function.23
// CHECK-NEXT:        Decl %d.15 'arguments' Var Arguments
// CHECK-NEXT:        Decl %d.16 'generic' Var : %function.25
// CHECK-NEXT:        hoistedFunction f
// CHECK-NEXT:        hoistedFunction testCall
// CHECK-NEXT:        hoistedFunction rest
// CHECK-NEXT:        hoistedFunction testRest
// CHECK-NEXT:        hoistedFunction testMethodAndNew
// CHECK-NEXT:        hoistedFunction testSHCall
// CHECK-NEXT:        hoistedFunction withThis
// CHECK-NEXT:        hoistedFunction testThisForms
// CHECK-NEXT:        hoistedFunction generic
// CHECK-NEXT:        hoistedFunction testGeneric
// CHECK-NEXT:        hoistedFunction generic
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:        Scope %s.3
// CHECK-NEXT:        Scope %s.4
// CHECK-NEXT:            Decl %d.17 'SH_call' Const
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.5
// CHECK-NEXT:            Decl %d.18 'a' Parameter : number
// CHECK-NEXT:            Decl %d.19 'b' Parameter : string
// CHECK-NEXT:            Decl %d.20 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.6
// CHECK-NEXT:            Decl %d.21 't' Parameter : %tuple.8
// CHECK-NEXT:            Decl %d.22 'empty' Parameter : %tuple.13
// CHECK-NEXT:            Decl %d.23 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.7
// CHECK-NEXT:            Decl %d.24 'a' Parameter : number
// CHECK-NEXT:            Decl %d.25 'r' Parameter : %class.1
// CHECK-NEXT:            Decl %d.26 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.8
// CHECK-NEXT:            Decl %d.27 't' Parameter : %tuple.16
// CHECK-NEXT:            Decl %d.28 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.9
// CHECK-NEXT:            Decl %d.29 'a' Parameter : number
// CHECK-NEXT:            Decl %d.30 'b' Parameter : string
// CHECK-NEXT:            Decl %d.31 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.10
// CHECK-NEXT:            Decl %d.32 'a' Parameter : number
// CHECK-NEXT:            Decl %d.33 'b' Parameter : string
// CHECK-NEXT:            Decl %d.34 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.11
// CHECK-NEXT:            Decl %d.35 'p' Parameter : %tuple.8
// CHECK-NEXT:            Decl %d.36 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.12
// CHECK-NEXT:            Decl %d.37 't' Parameter : %tuple.8
// CHECK-NEXT:            Decl %d.38 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.13
// CHECK-NEXT:            Decl %d.39 'a' Parameter : number
// CHECK-NEXT:            Decl %d.40 'b' Parameter : number
// CHECK-NEXT:            Decl %d.41 'cb' Parameter : %function.10
// CHECK-NEXT:            Decl %d.42 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.14
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.15
// CHECK-NEXT:            Decl %d.43 't' Parameter : %tuple.19
// CHECK-NEXT:            Decl %d.44 'arguments' Var Arguments
// CHECK-NEXT:        Func strict noImplicitReturn
// CHECK-NEXT:            Scope %s.16
// CHECK-NEXT:                Decl %d.45 'x' Parameter : number
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.17
// CHECK-NEXT:            Decl %d.46 'this' Parameter : any
// CHECK-NEXT:            Decl %d.47 'a' Parameter : number
// CHECK-NEXT:            Decl %d.48 'b' Parameter : string
// CHECK-NEXT:            Decl %d.49 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.18
// CHECK-NEXT:            Decl %d.50 't' Parameter : %tuple.8
// CHECK-NEXT:            Decl %d.51 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.19
// CHECK-NEXT:            Decl %d.52 'x' Parameter
// CHECK-NEXT:            Decl %d.53 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.20
// CHECK-NEXT:            Decl %d.54 't' Parameter : %tuple.22
// CHECK-NEXT:            Decl %d.55 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.21
// CHECK-NEXT:            Decl %d.56 'x' Parameter : string
// CHECK-NEXT:            Decl %d.57 'arguments' Var Arguments

// CHECK:FunctionExpression : %untyped_function.2
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            StringLiteral : "use strict"
// CHECK-NEXT:        FunctionDeclaration : %function.12
// CHECK-NEXT:            Id 'f' [D:E:%d.2 'f']
// CHECK-NEXT:            Id 'a' [D:E:%d.18 'a']
// CHECK-NEXT:            Id 'b' [D:E:%d.19 'b']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:        FunctionDeclaration : %function.14
// CHECK-NEXT:            Id 'testCall' [D:E:%d.3 'testCall']
// CHECK-NEXT:            Id 't' [D:E:%d.21 't']
// CHECK-NEXT:            Id 'empty' [D:E:%d.22 'empty']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    CallExpression : void
// CHECK-NEXT:                        Id 'f' [D:E:%d.2 'f'] : %function.12
// CHECK-NEXT:                        SpreadElement
// CHECK-NEXT:                            Id 't' [D:E:%d.21 't'] : %tuple.8
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    CallExpression : void
// CHECK-NEXT:                        Id 'f' [D:E:%d.2 'f'] : %function.12
// CHECK-NEXT:                        SpreadElement
// CHECK-NEXT:                            TypeCastExpression : %tuple.24
// CHECK-NEXT:                                ArrayExpression : %tuple.24
// CHECK-NEXT:                                    NumericLiteral : 1
// CHECK-NEXT:                        StringLiteral : "x"
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    CallExpression : void
// CHECK-NEXT:                        Id 'f' [D:E:%d.2 'f'] : %function.12
// CHECK-NEXT:                        NumericLiteral : 1
// CHECK-NEXT:                        SpreadElement
// CHECK-NEXT:                            TypeCastExpression : %tuple.22
// CHECK-NEXT:                                ArrayExpression : %tuple.22
// CHECK-NEXT:                                    StringLiteral : "x"
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    CallExpression : void
// CHECK-NEXT:                        Id 'f' [D:E:%d.2 'f'] : %function.12
// CHECK-NEXT:                        SpreadElement
// CHECK-NEXT:                            Id 't' [D:E:%d.21 't'] : %tuple.8
// CHECK-NEXT:                        SpreadElement
// CHECK-NEXT:                            Id 'empty' [D:E:%d.22 'empty'] : %tuple.13
// CHECK-NEXT:        FunctionDeclaration : %function.15
// CHECK-NEXT:            Id 'rest' [D:E:%d.4 'rest']
// CHECK-NEXT:            Id 'a' [D:E:%d.24 'a']
// CHECK-NEXT:            RestElement
// CHECK-NEXT:                Id 'r' [D:E:%d.25 'r']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:        FunctionDeclaration : %function.17
// CHECK-NEXT:            Id 'testRest' [D:E:%d.5 'testRest']
// CHECK-NEXT:            Id 't' [D:E:%d.27 't']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    CallExpression : void
// CHECK-NEXT:                        Id 'rest' [D:E:%d.4 'rest'] : %function.15
// CHECK-NEXT:                        SpreadElement
// CHECK-NEXT:                            Id 't' [D:E:%d.27 't'] : %tuple.16
// CHECK-NEXT:        ClassDeclaration Scope %s.2
// CHECK-NEXT:            Id 'C' [D:E:%d.6 'C']
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:                ClassProperty : number
// CHECK-NEXT:                    Id 'x'
// CHECK-NEXT:                MethodDefinition : %function.7
// CHECK-NEXT:                    Id 'constructor'
// CHECK-NEXT:                    FunctionExpression : %function.7
// CHECK-NEXT:                        Id 'a' [D:E:%d.29 'a']
// CHECK-NEXT:                        Id 'b' [D:E:%d.30 'b']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            ExpressionStatement
// CHECK-NEXT:                                AssignmentExpression : number
// CHECK-NEXT:                                    MemberExpression : number
// CHECK-NEXT:                                        ThisExpression : %class.3
// CHECK-NEXT:                                        Id 'x'
// CHECK-NEXT:                                    Id 'a' [D:E:%d.29 'a'] : number
// CHECK-NEXT:                MethodDefinition : %function.7
// CHECK-NEXT:                    Id 'm'
// CHECK-NEXT:                    FunctionExpression : %function.7
// CHECK-NEXT:                        Id 'a' [D:E:%d.32 'a']
// CHECK-NEXT:                        Id 'b' [D:E:%d.33 'b']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:        ClassDeclaration Scope %s.3
// CHECK-NEXT:            Id 'D' [D:E:%d.7 'D']
// CHECK-NEXT:            Id 'C' [D:E:%d.6 'C'] : %class_constructor.4
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:                MethodDefinition : %function.9
// CHECK-NEXT:                    Id 'constructor'
// CHECK-NEXT:                    FunctionExpression : %function.9
// CHECK-NEXT:                        Id 'p' [D:E:%d.35 'p']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            ExpressionStatement
// CHECK-NEXT:                                CallExpression : void
// CHECK-NEXT:                                    Super : %function.7
// CHECK-NEXT:                                    SpreadElement
// CHECK-NEXT:                                        Id 'p' [D:E:%d.35 'p'] : %tuple.8
// CHECK-NEXT:        FunctionDeclaration : %function.18
// CHECK-NEXT:            Id 'testMethodAndNew' [D:E:%d.8 'testMethodAndNew']
// CHECK-NEXT:            Id 't' [D:E:%d.37 't']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    NewExpression : %class.3
// CHECK-NEXT:                        Id 'C' [D:E:%d.6 'C'] : %class_constructor.4
// CHECK-NEXT:                        SpreadElement
// CHECK-NEXT:                            Id 't' [D:E:%d.37 't'] : %tuple.8
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    CallExpression : void
// CHECK-NEXT:                        MemberExpression : %function.7
// CHECK-NEXT:                            NewExpression : %class.3
// CHECK-NEXT:                                Id 'C' [D:E:%d.6 'C'] : %class_constructor.4
// CHECK-NEXT:                                SpreadElement
// CHECK-NEXT:                                    Id 't' [D:E:%d.37 't'] : %tuple.8
// CHECK-NEXT:                            Id 'm'
// CHECK-NEXT:                        SpreadElement
// CHECK-NEXT:                            Id 't' [D:E:%d.37 't'] : %tuple.8
// CHECK-NEXT:        ClassDeclaration Scope %s.4
// CHECK-NEXT:            Id 'K' [D:E:%d.9 'K']
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:                MethodDefinition : %function.11
// CHECK-NEXT:                    Id 'SH_call' [D:E:%d.17 'SH_call']
// CHECK-NEXT:                    FunctionExpression : %function.11
// CHECK-NEXT:                        Id 'a' [D:E:%d.39 'a']
// CHECK-NEXT:                        Id 'b' [D:E:%d.40 'b']
// CHECK-NEXT:                        Id 'cb' [D:E:%d.41 'cb']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:        FunctionDeclaration : %function.20
// CHECK-NEXT:            Id 'testSHCall' [D:E:%d.10 'testSHCall']
// CHECK-NEXT:            Id 't' [D:E:%d.43 't']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    CallExpression : void
// CHECK-NEXT:                        Id 'K' [D:E:%d.9 'K'] : %class_constructor.6
// CHECK-NEXT:                        SpreadElement
// CHECK-NEXT:                            Id 't' [D:E:%d.43 't'] : %tuple.19
// CHECK-NEXT:                        ArrowFunctionExpression : %function.10
// CHECK-NEXT:                            Id 'x' [D:E:%d.45 'x']
// CHECK-NEXT:                            BlockStatement
// CHECK-NEXT:                                ReturnStatement
// CHECK-NEXT:                                    BinaryExpression : number
// CHECK-NEXT:                                        Id 'x' [D:E:%d.45 'x'] : number
// CHECK-NEXT:                                        BinOp +
// CHECK-NEXT:                                        NumericLiteral : 1
// CHECK-NEXT:        FunctionDeclaration : %function.21
// CHECK-NEXT:            Id 'withThis' [D:E:%d.11 'withThis']
// CHECK-NEXT:            Id 'this' [D:E:%d.46 'this']
// CHECK-NEXT:            Id 'a' [D:E:%d.47 'a']
// CHECK-NEXT:            Id 'b' [D:E:%d.48 'b']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:        FunctionDeclaration : %function.18
// CHECK-NEXT:            Id 'testThisForms' [D:E:%d.12 'testThisForms']
// CHECK-NEXT:            Id 't' [D:E:%d.50 't']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    CallExpression : void
// CHECK-NEXT:                        MemberExpression
// CHECK-NEXT:                            Id 'withThis' [D:E:%d.11 'withThis'] : %function.21
// CHECK-NEXT:                            Id 'call'
// CHECK-NEXT:                        Id 'undefined' [D:E:%d.58 'undefined'] : void
// CHECK-NEXT:                        SpreadElement
// CHECK-NEXT:                            Id 't' [D:E:%d.50 't'] : %tuple.8
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    CallExpression : void
// CHECK-NEXT:                        MemberExpression
// CHECK-NEXT:                            SHBuiltin
// CHECK-NEXT:                            Id 'call'
// CHECK-NEXT:                        Id 'withThis' [D:E:%d.11 'withThis'] : %function.21
// CHECK-NEXT:                        Id 'undefined' [D:E:%d.58 'undefined'] : void
// CHECK-NEXT:                        SpreadElement
// CHECK-NEXT:                            Id 't' [D:E:%d.50 't'] : %tuple.8
// CHECK-NEXT:        FunctionDeclaration : %function.25
// CHECK-NEXT:            Id 'generic' [D:E:%d.16 'generic']
// CHECK-NEXT:            Id 'x' [D:E:%d.56 'x']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    Id 'x' [D:E:%d.56 'x'] : string
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:        FunctionDeclaration
// CHECK-NEXT:            Id 'generic' [D:E:%d.13 'generic']
// CHECK-NEXT:            Id 'x' [D:E:%d.52 'x']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    Id 'x' [D:E:%d.52 'x']
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:        FunctionDeclaration : %function.23
// CHECK-NEXT:            Id 'testGeneric' [D:E:%d.14 'testGeneric']
// CHECK-NEXT:            Id 't' [D:E:%d.54 't']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    CallExpression : string
// CHECK-NEXT:                        Id 'generic' [D:E:%d.16 'generic'] : %function.25
// CHECK-NEXT:                        TypeParameterInstantiation
// CHECK-NEXT:                            StringTypeAnnotation
// CHECK-NEXT:                        SpreadElement
// CHECK-NEXT:                            Id 't' [D:E:%d.54 't'] : %tuple.22
