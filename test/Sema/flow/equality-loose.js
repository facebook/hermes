/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror --typed -fno-std-globals --dump-sema %s | %FileCheckOrRegen %s --match-full-lines

/// `==` and `!=` are allowed only in the documented cases.

class Foo {}

function main(
    optFoo: ?Foo,
    fooOrNull: Foo | null,
    numOrVoid: number | void,
    anyVal: any,
    mixedVal: mixed,
    n: number,
) {
  // `null` literal: other side must accept null.
  optFoo == null;
  optFoo != null;
  fooOrNull == null;
  mixedVal == null;

  // `undefined` literal: other side must accept void.
  optFoo == (void 0);
  optFoo != (void 0);
  numOrVoid == (void 0);
  mixedVal == (void 0);

  // `null` / `undefined` flow into each other.
  numOrVoid == null;
  null == (void 0);

  // `any` escape: any pair allowed.
  anyVal == n;
  n == anyVal;
  anyVal != optFoo;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%class_constructor.2 = class_constructor(%class.7)
// CHECK-NEXT:%union.3 = union(void | null | %class.7)
// CHECK-NEXT:%union.4 = union(null | %class.7)
// CHECK-NEXT:%union.5 = union(void | number)
// CHECK-NEXT:%function.6 = function(optFoo: %union.3, fooOrNull: %union.4, numOrVoid: %union.5, anyVal: any, mixedVal: mixed, n: number): any
// CHECK-NEXT:%class.7 = class(Foo {
// CHECK-NEXT:  %homeObject: %class.8
// CHECK-NEXT:})
// CHECK-NEXT:%class.8 = class( {
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict mayReachImplicitReturn
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'Foo' Class : %class_constructor.2
// CHECK-NEXT:        Decl %d.3 'main' Var : %function.6
// CHECK-NEXT:        Decl %d.4 'arguments' Var Arguments
// CHECK-NEXT:        hoistedFunction main
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.3
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.4
// CHECK-NEXT:            Decl %d.5 'optFoo' Parameter : %union.3
// CHECK-NEXT:            Decl %d.6 'fooOrNull' Parameter : %union.4
// CHECK-NEXT:            Decl %d.7 'numOrVoid' Parameter : %union.5
// CHECK-NEXT:            Decl %d.8 'anyVal' Parameter : any
// CHECK-NEXT:            Decl %d.9 'mixedVal' Parameter : mixed
// CHECK-NEXT:            Decl %d.10 'n' Parameter : number
// CHECK-NEXT:            Decl %d.11 'arguments' Var Arguments

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        ClassDeclaration Scope %s.2
// CHECK-NEXT:            Id 'Foo' [D:E:%d.2 'Foo']
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:        FunctionDeclaration : %function.6
// CHECK-NEXT:            Id 'main' [D:E:%d.3 'main']
// CHECK-NEXT:            Id 'optFoo' [D:E:%d.5 'optFoo']
// CHECK-NEXT:            Id 'fooOrNull' [D:E:%d.6 'fooOrNull']
// CHECK-NEXT:            Id 'numOrVoid' [D:E:%d.7 'numOrVoid']
// CHECK-NEXT:            Id 'anyVal' [D:E:%d.8 'anyVal']
// CHECK-NEXT:            Id 'mixedVal' [D:E:%d.9 'mixedVal']
// CHECK-NEXT:            Id 'n' [D:E:%d.10 'n']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    BinaryExpression : boolean
// CHECK-NEXT:                        Id 'optFoo' [D:E:%d.5 'optFoo'] : %union.3
// CHECK-NEXT:                        NullLiteral : null
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    BinaryExpression : boolean
// CHECK-NEXT:                        Id 'optFoo' [D:E:%d.5 'optFoo'] : %union.3
// CHECK-NEXT:                        NullLiteral : null
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    BinaryExpression : boolean
// CHECK-NEXT:                        Id 'fooOrNull' [D:E:%d.6 'fooOrNull'] : %union.4
// CHECK-NEXT:                        NullLiteral : null
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    BinaryExpression : boolean
// CHECK-NEXT:                        Id 'mixedVal' [D:E:%d.9 'mixedVal'] : mixed
// CHECK-NEXT:                        NullLiteral : null
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    BinaryExpression : boolean
// CHECK-NEXT:                        Id 'optFoo' [D:E:%d.5 'optFoo'] : %union.3
// CHECK-NEXT:                        UnaryExpression : void
// CHECK-NEXT:                            NumericLiteral : number
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    BinaryExpression : boolean
// CHECK-NEXT:                        Id 'optFoo' [D:E:%d.5 'optFoo'] : %union.3
// CHECK-NEXT:                        UnaryExpression : void
// CHECK-NEXT:                            NumericLiteral : number
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    BinaryExpression : boolean
// CHECK-NEXT:                        Id 'numOrVoid' [D:E:%d.7 'numOrVoid'] : %union.5
// CHECK-NEXT:                        UnaryExpression : void
// CHECK-NEXT:                            NumericLiteral : number
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    BinaryExpression : boolean
// CHECK-NEXT:                        Id 'mixedVal' [D:E:%d.9 'mixedVal'] : mixed
// CHECK-NEXT:                        UnaryExpression : void
// CHECK-NEXT:                            NumericLiteral : number
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    BinaryExpression : boolean
// CHECK-NEXT:                        Id 'numOrVoid' [D:E:%d.7 'numOrVoid'] : %union.5
// CHECK-NEXT:                        NullLiteral : null
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    BinaryExpression : boolean
// CHECK-NEXT:                        NullLiteral : null
// CHECK-NEXT:                        UnaryExpression : void
// CHECK-NEXT:                            NumericLiteral : number
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    BinaryExpression : boolean
// CHECK-NEXT:                        Id 'anyVal' [D:E:%d.8 'anyVal'] : any
// CHECK-NEXT:                        Id 'n' [D:E:%d.10 'n'] : number
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    BinaryExpression : boolean
// CHECK-NEXT:                        Id 'n' [D:E:%d.10 'n'] : number
// CHECK-NEXT:                        Id 'anyVal' [D:E:%d.8 'anyVal'] : any
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    BinaryExpression : boolean
// CHECK-NEXT:                        Id 'anyVal' [D:E:%d.8 'anyVal'] : any
// CHECK-NEXT:                        Id 'optFoo' [D:E:%d.5 'optFoo'] : %union.3
