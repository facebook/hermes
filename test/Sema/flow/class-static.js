/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -typed -dump-sema -fno-std-globals %s | %FileCheckOrRegen --match-full-lines %s

class A {
  x: number = 0;
  static y: number = 1;
  static foo(): number {
    return 1;
  }
  bar(): number {
    return 2;
  }
}

class B extends A {
  static z: string = 'hello';
  static foo(): number {
    return 3;
  }
}

// Inherits static object from A even with no new statics.
class C extends A {}

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%class_constructor.2 = class_constructor(%class.8)
// CHECK-NEXT:%class_constructor.3 = class_constructor(%class.9)
// CHECK-NEXT:%class_constructor.4 = class_constructor(%class.10)
// CHECK-NEXT:%function.5 = function(this: %class_constructor.2): number
// CHECK-NEXT:%function.6 = function(this: %class.8): number
// CHECK-NEXT:%function.7 = function(this: %class_constructor.3): number
// CHECK-NEXT:%class.8 = class(A {
// CHECK-NEXT:  %homeObject: %class.11
// CHECK-NEXT:  %staticObject: %class.12
// CHECK-NEXT:  x: number
// CHECK-NEXT:})
// CHECK-NEXT:%class.9 = class(B extends %class.8 {
// CHECK-NEXT:  %homeObject: %class.13
// CHECK-NEXT:  %staticObject: %class.14
// CHECK-NEXT:})
// CHECK-NEXT:%class.10 = class(C extends %class.8 {
// CHECK-NEXT:  %homeObject: %class.15
// CHECK-NEXT:  %staticObject: %class.16
// CHECK-NEXT:})
// CHECK-NEXT:%class.11 = class( {
// CHECK-NEXT:  bar [final]: %function.6
// CHECK-NEXT:})
// CHECK-NEXT:%class.12 = class( {
// CHECK-NEXT:  y: number
// CHECK-NEXT:  foo [overridden]: %function.5
// CHECK-NEXT:})
// CHECK-NEXT:%class.13 = class( extends %class.11 {
// CHECK-NEXT:})
// CHECK-NEXT:%class.14 = class( extends %class.12 {
// CHECK-NEXT:  z: string
// CHECK-NEXT:  foo [final]: %function.7
// CHECK-NEXT:})
// CHECK-NEXT:%class.15 = class( extends %class.11 {
// CHECK-NEXT:})
// CHECK-NEXT:%class.16 = class( extends %class.12 {
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'A' Class : %class_constructor.2
// CHECK-NEXT:        Decl %d.3 'B' Class : %class_constructor.3
// CHECK-NEXT:        Decl %d.4 'C' Class : %class_constructor.4
// CHECK-NEXT:        Decl %d.5 'arguments' Var Arguments
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.6 'y' Const
// CHECK-NEXT:            Decl %d.7 'foo' Const
// CHECK-NEXT:        Scope %s.3
// CHECK-NEXT:            Decl %d.8 'z' Const
// CHECK-NEXT:            Decl %d.9 'foo' Const
// CHECK-NEXT:        Scope %s.4
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.5
// CHECK-NEXT:            Decl %d.10 'arguments' Var Arguments
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.6
// CHECK-NEXT:            Decl %d.11 'arguments' Var Arguments
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.7
// CHECK-NEXT:            Decl %d.12 'arguments' Var Arguments
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.8
// CHECK-NEXT:            Decl %d.13 'arguments' Var Arguments
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.9
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.10
// CHECK-NEXT:            Decl %d.14 'arguments' Var Arguments
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.11
// CHECK-NEXT:            Decl %d.15 'arguments' Var Arguments
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.12
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.13

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        ClassDeclaration Scope %s.2
// CHECK-NEXT:            Id 'A' [D:E:%d.2 'A']
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:                ClassProperty : number
// CHECK-NEXT:                    Id 'x'
// CHECK-NEXT:                    NumericLiteral : number
// CHECK-NEXT:                ClassProperty : number
// CHECK-NEXT:                    Id 'y' [D:E:%d.6 'y']
// CHECK-NEXT:                    NumericLiteral : number
// CHECK-NEXT:                MethodDefinition : %function.5
// CHECK-NEXT:                    Id 'foo' [D:E:%d.7 'foo']
// CHECK-NEXT:                    FunctionExpression : %function.5
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            ReturnStatement
// CHECK-NEXT:                                NumericLiteral : number
// CHECK-NEXT:                MethodDefinition : %function.6
// CHECK-NEXT:                    Id 'bar'
// CHECK-NEXT:                    FunctionExpression : %function.6
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            ReturnStatement
// CHECK-NEXT:                                NumericLiteral : number
// CHECK-NEXT:        ClassDeclaration Scope %s.3
// CHECK-NEXT:            Id 'B' [D:E:%d.3 'B']
// CHECK-NEXT:            Id 'A' [D:E:%d.2 'A'] : %class_constructor.2
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:                ClassProperty : string
// CHECK-NEXT:                    Id 'z' [D:E:%d.8 'z']
// CHECK-NEXT:                    StringLiteral : string
// CHECK-NEXT:                MethodDefinition : %function.7
// CHECK-NEXT:                    Id 'foo' [D:E:%d.9 'foo']
// CHECK-NEXT:                    FunctionExpression : %function.7
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            ReturnStatement
// CHECK-NEXT:                                NumericLiteral : number
// CHECK-NEXT:        ClassDeclaration Scope %s.4
// CHECK-NEXT:            Id 'C' [D:E:%d.4 'C']
// CHECK-NEXT:            Id 'A' [D:E:%d.2 'A'] : %class_constructor.2
// CHECK-NEXT:            ClassBody
