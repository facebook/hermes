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
// CHECK-NEXT:%class.2 = class(D {
// CHECK-NEXT:  %constructor: %function.3
// CHECK-NEXT:  %homeObject: %class.4
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.5 = class_constructor(%class.2)
// CHECK-NEXT:%class.6 = class(A {
// CHECK-NEXT:  %constructor: %function.7
// CHECK-NEXT:  %homeObject: %class.8
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.9 = class_constructor(%class.6)
// CHECK-NEXT:%function.3 = function(this: %class.2, val: %class.6): void
// CHECK-NEXT:%class.4 = class( {
// CHECK-NEXT:})
// CHECK-NEXT:%function.7 = function(this: %class.6): void
// CHECK-NEXT:%class.10 = class(A {
// CHECK-NEXT:  %constructor: %function.11
// CHECK-NEXT:  %homeObject: %class.12
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.13 = class_constructor(%class.10)
// CHECK-NEXT:%function.14 = function(this: %class.6, a: %class.10): void
// CHECK-NEXT:%class.8 = class( {
// CHECK-NEXT:  foo [final]: %function.14
// CHECK-NEXT:})
// CHECK-NEXT:%function.11 = function(this: %class.10): void
// CHECK-NEXT:%function.15 = function(this: %class.10, a: %class.10): void
// CHECK-NEXT:%class.12 = class( {
// CHECK-NEXT:  foo [final]: %function.15
// CHECK-NEXT:})
// CHECK-NEXT:%class.16 = class(B extends %class.10 {
// CHECK-NEXT:  %constructor: %function.17
// CHECK-NEXT:  %homeObject: %class.18
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.19 = class_constructor(%class.16)
// CHECK-NEXT:%function.17 = function(this: %class.16): void
// CHECK-NEXT:%class.18 = class( extends %class.12 {
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'outer' Let : %class.16
// CHECK-NEXT:            Decl %d.3 'A' Class
// CHECK-NEXT:            Decl %d.4 'B' Class
// CHECK-NEXT:            Decl %d.5 'D' Class : %class_constructor.5
// CHECK-NEXT:            Decl %d.6 'arguments' Var Arguments
// CHECK-NEXT:            Decl %d.7 'A' Class : %class_constructor.9
// CHECK-NEXT:            Decl %d.8 'A' Class : %class_constructor.13
// CHECK-NEXT:            Decl %d.9 'B' Class : %class_constructor.19
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:                Decl %d.10 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.4
// CHECK-NEXT:                Decl %d.11 'a' Parameter
// CHECK-NEXT:                Decl %d.12 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.5
// CHECK-NEXT:                Decl %d.13 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.6
// CHECK-NEXT:                Decl %d.14 'val' Parameter : %class.6
// CHECK-NEXT:                Decl %d.15 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.7
// CHECK-NEXT:                Decl %d.16 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.8
// CHECK-NEXT:                Decl %d.17 'a' Parameter : %class.10
// CHECK-NEXT:                Decl %d.18 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.9
// CHECK-NEXT:                Decl %d.19 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.10
// CHECK-NEXT:                Decl %d.20 'a' Parameter : %class.10
// CHECK-NEXT:                Decl %d.21 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.11
// CHECK-NEXT:                Decl %d.22 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            Id 'outer' [D:E:%d.2 'outer']
// CHECK-NEXT:                    ClassDeclaration
// CHECK-NEXT:                        Id 'A' [D:E:%d.7 'A']
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            MethodDefinition : %function.7
// CHECK-NEXT:                                Id 'constructor'
// CHECK-NEXT:                                FunctionExpression : %function.7
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                            MethodDefinition : %function.14
// CHECK-NEXT:                                Id 'foo'
// CHECK-NEXT:                                FunctionExpression : %function.14
// CHECK-NEXT:                                    Id 'a' [D:E:%d.17 'a']
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                    ClassDeclaration
// CHECK-NEXT:                        Id 'A' [D:E:%d.8 'A']
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            MethodDefinition : %function.11
// CHECK-NEXT:                                Id 'constructor'
// CHECK-NEXT:                                FunctionExpression : %function.11
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                            MethodDefinition : %function.15
// CHECK-NEXT:                                Id 'foo'
// CHECK-NEXT:                                FunctionExpression : %function.15
// CHECK-NEXT:                                    Id 'a' [D:E:%d.20 'a']
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                    ClassDeclaration
// CHECK-NEXT:                        Id 'A' [D:E:%d.3 'A']
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            MethodDefinition
// CHECK-NEXT:                                Id 'constructor'
// CHECK-NEXT:                                FunctionExpression
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                            MethodDefinition
// CHECK-NEXT:                                Id 'foo'
// CHECK-NEXT:                                FunctionExpression
// CHECK-NEXT:                                    Id 'a' [D:E:%d.11 'a']
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                    ClassDeclaration
// CHECK-NEXT:                        Id 'B' [D:E:%d.9 'B']
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                        Id 'A' [D:E:%d.8 'A'] : %class_constructor.13
// CHECK-NEXT:                        TypeParameterInstantiation
// CHECK-NEXT:                            GenericTypeAnnotation
// CHECK-NEXT:                                Id 'T'
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            MethodDefinition : %function.17
// CHECK-NEXT:                                Id 'constructor'
// CHECK-NEXT:                                FunctionExpression : %function.17
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        ExpressionStatement
// CHECK-NEXT:                                            CallExpression : void
// CHECK-NEXT:                                                Super : %function.11
// CHECK-NEXT:                    ClassDeclaration
// CHECK-NEXT:                        Id 'B' [D:E:%d.4 'B']
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                        Id 'A' [D:E:%d.3 'A']
// CHECK-NEXT:                        TypeParameterInstantiation
// CHECK-NEXT:                            GenericTypeAnnotation
// CHECK-NEXT:                                Id 'T'
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            MethodDefinition
// CHECK-NEXT:                                Id 'constructor'
// CHECK-NEXT:                                FunctionExpression
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        ExpressionStatement
// CHECK-NEXT:                                            CallExpression
// CHECK-NEXT:                                                Super
// CHECK-NEXT:                    ClassDeclaration
// CHECK-NEXT:                        Id 'D' [D:E:%d.5 'D']
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            MethodDefinition : %function.3
// CHECK-NEXT:                                Id 'constructor'
// CHECK-NEXT:                                FunctionExpression : %function.3
// CHECK-NEXT:                                    Id 'val' [D:E:%d.14 'val']
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:            ObjectExpression
