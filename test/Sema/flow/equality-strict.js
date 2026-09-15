/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror --typed -fno-std-globals --dump-sema %s | %FileCheckOrRegen --match-full-lines %s

/// Strict equality is accepted when one side flows into the other without
/// a checked cast.

class Foo {}
class SubFoo extends Foo {}

function main(
    n: number,
    s: string,
    foo: Foo,
    sub: SubFoo,
    optFoo: ?Foo,
    anyVal: any,
    mixedVal: mixed,
    unionAB: number | string,
) {
  // Identical primitives.
  n === n;
  s !== s;
  n === 1;
  s === "x";

  // Nullable patterns.
  optFoo === null;
  optFoo !== (void 0);
  optFoo === foo;
  foo === optFoo;

  // Class hierarchy in either order.
  foo === sub;
  sub === foo;
  sub !== sub;

  // any flows in both directions (T -> any is no-cast).
  anyVal === n;
  n === anyVal;
  anyVal === foo;

  // mixed accepts everything (T -> mixed is no-cast).
  mixedVal === n;
  s === mixedVal;

  // Union arm match.
  unionAB === n;
  s === unionAB;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%class.2 = class(Foo {
// CHECK-NEXT:  %homeObject: %class.9
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.3 = class_constructor(%class.2)
// CHECK-NEXT:%class.4 = class(SubFoo extends %class.2 {
// CHECK-NEXT:  %homeObject: %class.10
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.5 = class_constructor(%class.4)
// CHECK-NEXT:%union.6 = union(void | null | %class.2)
// CHECK-NEXT:%union.7 = union(string | number)
// CHECK-NEXT:%function.8 = function(n: number, s: string, foo: %class.2, sub: %class.4, optFoo: %union.6, anyVal: any, mixedVal: mixed, unionAB: %union.7): any
// CHECK-NEXT:%class.9 = class( {
// CHECK-NEXT:})
// CHECK-NEXT:%class.10 = class( extends %class.9 {
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict mayReachImplicitReturn
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'Foo' Class : %class_constructor.3
// CHECK-NEXT:        Decl %d.3 'SubFoo' Class : %class_constructor.5
// CHECK-NEXT:        Decl %d.4 'main' Var : %function.8
// CHECK-NEXT:        Decl %d.5 'arguments' Var Arguments
// CHECK-NEXT:        hoistedFunction main
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:        Scope %s.3
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.4
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.5
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.6
// CHECK-NEXT:            Decl %d.6 'n' Parameter : number
// CHECK-NEXT:            Decl %d.7 's' Parameter : string
// CHECK-NEXT:            Decl %d.8 'foo' Parameter : %class.2
// CHECK-NEXT:            Decl %d.9 'sub' Parameter : %class.4
// CHECK-NEXT:            Decl %d.10 'optFoo' Parameter : %union.6
// CHECK-NEXT:            Decl %d.11 'anyVal' Parameter : any
// CHECK-NEXT:            Decl %d.12 'mixedVal' Parameter : mixed
// CHECK-NEXT:            Decl %d.13 'unionAB' Parameter : %union.7
// CHECK-NEXT:            Decl %d.14 'arguments' Var Arguments

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        ClassDeclaration Scope %s.2
// CHECK-NEXT:            Id 'Foo' [D:E:%d.2 'Foo']
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:        ClassDeclaration Scope %s.3
// CHECK-NEXT:            Id 'SubFoo' [D:E:%d.3 'SubFoo']
// CHECK-NEXT:            Id 'Foo' [D:E:%d.2 'Foo'] : %class_constructor.3
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:        FunctionDeclaration : %function.8
// CHECK-NEXT:            Id 'main' [D:E:%d.4 'main']
// CHECK-NEXT:            Id 'n' [D:E:%d.6 'n']
// CHECK-NEXT:            Id 's' [D:E:%d.7 's']
// CHECK-NEXT:            Id 'foo' [D:E:%d.8 'foo']
// CHECK-NEXT:            Id 'sub' [D:E:%d.9 'sub']
// CHECK-NEXT:            Id 'optFoo' [D:E:%d.10 'optFoo']
// CHECK-NEXT:            Id 'anyVal' [D:E:%d.11 'anyVal']
// CHECK-NEXT:            Id 'mixedVal' [D:E:%d.12 'mixedVal']
// CHECK-NEXT:            Id 'unionAB' [D:E:%d.13 'unionAB']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    BinaryExpression : boolean
// CHECK-NEXT:                        Id 'n' [D:E:%d.6 'n'] : number
// CHECK-NEXT:                        Id 'n' [D:E:%d.6 'n'] : number
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    BinaryExpression : boolean
// CHECK-NEXT:                        Id 's' [D:E:%d.7 's'] : string
// CHECK-NEXT:                        Id 's' [D:E:%d.7 's'] : string
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    BinaryExpression : boolean
// CHECK-NEXT:                        Id 'n' [D:E:%d.6 'n'] : number
// CHECK-NEXT:                        NumericLiteral : number
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    BinaryExpression : boolean
// CHECK-NEXT:                        Id 's' [D:E:%d.7 's'] : string
// CHECK-NEXT:                        StringLiteral : string
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    BinaryExpression : boolean
// CHECK-NEXT:                        Id 'optFoo' [D:E:%d.10 'optFoo'] : %union.6
// CHECK-NEXT:                        NullLiteral : null
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    BinaryExpression : boolean
// CHECK-NEXT:                        Id 'optFoo' [D:E:%d.10 'optFoo'] : %union.6
// CHECK-NEXT:                        UnaryExpression : void
// CHECK-NEXT:                            NumericLiteral : number
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    BinaryExpression : boolean
// CHECK-NEXT:                        Id 'optFoo' [D:E:%d.10 'optFoo'] : %union.6
// CHECK-NEXT:                        Id 'foo' [D:E:%d.8 'foo'] : %class.2
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    BinaryExpression : boolean
// CHECK-NEXT:                        Id 'foo' [D:E:%d.8 'foo'] : %class.2
// CHECK-NEXT:                        Id 'optFoo' [D:E:%d.10 'optFoo'] : %union.6
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    BinaryExpression : boolean
// CHECK-NEXT:                        Id 'foo' [D:E:%d.8 'foo'] : %class.2
// CHECK-NEXT:                        Id 'sub' [D:E:%d.9 'sub'] : %class.4
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    BinaryExpression : boolean
// CHECK-NEXT:                        Id 'sub' [D:E:%d.9 'sub'] : %class.4
// CHECK-NEXT:                        Id 'foo' [D:E:%d.8 'foo'] : %class.2
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    BinaryExpression : boolean
// CHECK-NEXT:                        Id 'sub' [D:E:%d.9 'sub'] : %class.4
// CHECK-NEXT:                        Id 'sub' [D:E:%d.9 'sub'] : %class.4
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    BinaryExpression : boolean
// CHECK-NEXT:                        Id 'anyVal' [D:E:%d.11 'anyVal'] : any
// CHECK-NEXT:                        Id 'n' [D:E:%d.6 'n'] : number
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    BinaryExpression : boolean
// CHECK-NEXT:                        Id 'n' [D:E:%d.6 'n'] : number
// CHECK-NEXT:                        Id 'anyVal' [D:E:%d.11 'anyVal'] : any
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    BinaryExpression : boolean
// CHECK-NEXT:                        Id 'anyVal' [D:E:%d.11 'anyVal'] : any
// CHECK-NEXT:                        Id 'foo' [D:E:%d.8 'foo'] : %class.2
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    BinaryExpression : boolean
// CHECK-NEXT:                        Id 'mixedVal' [D:E:%d.12 'mixedVal'] : mixed
// CHECK-NEXT:                        Id 'n' [D:E:%d.6 'n'] : number
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    BinaryExpression : boolean
// CHECK-NEXT:                        Id 's' [D:E:%d.7 's'] : string
// CHECK-NEXT:                        Id 'mixedVal' [D:E:%d.12 'mixedVal'] : mixed
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    BinaryExpression : boolean
// CHECK-NEXT:                        Id 'unionAB' [D:E:%d.13 'unionAB'] : %union.7
// CHECK-NEXT:                        Id 'n' [D:E:%d.6 'n'] : number
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    BinaryExpression : boolean
// CHECK-NEXT:                        Id 's' [D:E:%d.7 's'] : string
// CHECK-NEXT:                        Id 'unionAB' [D:E:%d.13 'unionAB'] : %union.7
