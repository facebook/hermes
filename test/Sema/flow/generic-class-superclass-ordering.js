/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals -typed -dump-sema %s | %FileCheckOrRegen %s --match-full-lines

let outer: B<any>;

// Notice that A<any> is constructed from multiple locations.
// Make sure that it is initialized before B<any> is parsed.
class A<T> {
  constructor() {
  }
  foo(a: A<any>): void {}
}

class B<T> extends A<T> {
  constructor() {
    super();
  }
}

class D {
  constructor(val: A<string>) {
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%class_constructor.2 = class_constructor(%class.15)
// CHECK-NEXT:%class.3 = class(A {
// CHECK-NEXT:  %constructor: %function.6
// CHECK-NEXT:  %homeObject: %class.16
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.4 = class_constructor(%class.3)
// CHECK-NEXT:%function.5 = function(this: %class.15, val: %class.3): void
// CHECK-NEXT:%function.6 = function(this: %class.3): void
// CHECK-NEXT:%class.7 = class(A {
// CHECK-NEXT:  %constructor: %function.10
// CHECK-NEXT:  %homeObject: %class.17
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.8 = class_constructor(%class.7)
// CHECK-NEXT:%function.9 = function(this: %class.3, a: %class.7): void
// CHECK-NEXT:%function.10 = function(this: %class.7): void
// CHECK-NEXT:%function.11 = function(this: %class.7, a: %class.7): void
// CHECK-NEXT:%class.12 = class(B extends %class.7 {
// CHECK-NEXT:  %constructor: %function.14
// CHECK-NEXT:  %homeObject: %class.18
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.13 = class_constructor(%class.12)
// CHECK-NEXT:%function.14 = function(this: %class.12): void
// CHECK-NEXT:%class.15 = class(D {
// CHECK-NEXT:  %constructor: %function.5
// CHECK-NEXT:  %homeObject: %class.19
// CHECK-NEXT:})
// CHECK-NEXT:%class.16 = class( {
// CHECK-NEXT:  foo [final]: %function.9
// CHECK-NEXT:})
// CHECK-NEXT:%class.17 = class( {
// CHECK-NEXT:  foo [final]: %function.11
// CHECK-NEXT:})
// CHECK-NEXT:%class.18 = class( extends %class.17 {
// CHECK-NEXT:})
// CHECK-NEXT:%class.19 = class( {
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'outer' Let : %class.12
// CHECK-NEXT:        Decl %d.3 'A' Class
// CHECK-NEXT:        Decl %d.4 'B' Class
// CHECK-NEXT:        Decl %d.5 'D' Class : %class_constructor.2
// CHECK-NEXT:        Decl %d.6 'arguments' Var Arguments
// CHECK-NEXT:        Decl %d.7 'A' Class : %class_constructor.4
// CHECK-NEXT:        Decl %d.8 'A' Class : %class_constructor.8
// CHECK-NEXT:        Decl %d.9 'B' Class : %class_constructor.13
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:        Scope %s.3
// CHECK-NEXT:        Scope %s.4
// CHECK-NEXT:        Scope %s.5
// CHECK-NEXT:        Scope %s.6
// CHECK-NEXT:        Scope %s.7
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.8
// CHECK-NEXT:            Decl %d.10 'arguments' Var Arguments
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.9
// CHECK-NEXT:            Decl %d.11 'a' Parameter
// CHECK-NEXT:            Decl %d.12 'arguments' Var Arguments
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.10
// CHECK-NEXT:            Decl %d.13 'arguments' Var Arguments
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.11
// CHECK-NEXT:            Decl %d.14 'val' Parameter : %class.3
// CHECK-NEXT:            Decl %d.15 'arguments' Var Arguments
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.12
// CHECK-NEXT:            Decl %d.16 'arguments' Var Arguments
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.13
// CHECK-NEXT:            Decl %d.17 'a' Parameter : %class.7
// CHECK-NEXT:            Decl %d.18 'arguments' Var Arguments
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.14
// CHECK-NEXT:            Decl %d.19 'arguments' Var Arguments
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.15
// CHECK-NEXT:            Decl %d.20 'a' Parameter : %class.7
// CHECK-NEXT:            Decl %d.21 'arguments' Var Arguments
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.16
// CHECK-NEXT:            Decl %d.22 'arguments' Var Arguments

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                Id 'outer' [D:E:%d.2 'outer']
// CHECK-NEXT:        ClassDeclaration Scope %s.5
// CHECK-NEXT:            Id 'A' [D:E:%d.7 'A']
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:                MethodDefinition : %function.6
// CHECK-NEXT:                    Id 'constructor'
// CHECK-NEXT:                    FunctionExpression : %function.6
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                MethodDefinition : %function.9
// CHECK-NEXT:                    Id 'foo'
// CHECK-NEXT:                    FunctionExpression : %function.9
// CHECK-NEXT:                        Id 'a' [D:E:%d.17 'a']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:        ClassDeclaration Scope %s.6
// CHECK-NEXT:            Id 'A' [D:E:%d.8 'A']
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:                MethodDefinition : %function.10
// CHECK-NEXT:                    Id 'constructor'
// CHECK-NEXT:                    FunctionExpression : %function.10
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                MethodDefinition : %function.11
// CHECK-NEXT:                    Id 'foo'
// CHECK-NEXT:                    FunctionExpression : %function.11
// CHECK-NEXT:                        Id 'a' [D:E:%d.20 'a']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:        ClassDeclaration Scope %s.2
// CHECK-NEXT:            Id 'A' [D:E:%d.3 'A']
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:                MethodDefinition
// CHECK-NEXT:                    Id 'constructor'
// CHECK-NEXT:                    FunctionExpression
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                MethodDefinition
// CHECK-NEXT:                    Id 'foo'
// CHECK-NEXT:                    FunctionExpression
// CHECK-NEXT:                        Id 'a' [D:E:%d.11 'a']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:        ClassDeclaration Scope %s.7
// CHECK-NEXT:            Id 'B' [D:E:%d.9 'B']
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:            Id 'A' [D:E:%d.8 'A'] : %class_constructor.8
// CHECK-NEXT:            TypeParameterInstantiation
// CHECK-NEXT:                GenericTypeAnnotation
// CHECK-NEXT:                    Id 'T'
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:                MethodDefinition : %function.14
// CHECK-NEXT:                    Id 'constructor'
// CHECK-NEXT:                    FunctionExpression : %function.14
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            ExpressionStatement
// CHECK-NEXT:                                CallExpression : void
// CHECK-NEXT:                                    Super : %function.10
// CHECK-NEXT:        ClassDeclaration Scope %s.3
// CHECK-NEXT:            Id 'B' [D:E:%d.4 'B']
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:            Id 'A' [D:E:%d.3 'A']
// CHECK-NEXT:            TypeParameterInstantiation
// CHECK-NEXT:                GenericTypeAnnotation
// CHECK-NEXT:                    Id 'T'
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:                MethodDefinition
// CHECK-NEXT:                    Id 'constructor'
// CHECK-NEXT:                    FunctionExpression
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            ExpressionStatement
// CHECK-NEXT:                                CallExpression
// CHECK-NEXT:                                    Super
// CHECK-NEXT:        ClassDeclaration Scope %s.4
// CHECK-NEXT:            Id 'D' [D:E:%d.5 'D']
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:                MethodDefinition : %function.5
// CHECK-NEXT:                    Id 'constructor'
// CHECK-NEXT:                    FunctionExpression : %function.5
// CHECK-NEXT:                        Id 'val' [D:E:%d.14 'val']
// CHECK-NEXT:                        BlockStatement
