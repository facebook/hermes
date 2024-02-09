/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals -typed -dump-sema %s | %FileCheckOrRegen %s --match-full-lines

let outer: C<number> | null = null;

class A {
  foo(val: B<number>): void {}
}

class B<T> extends A {
  x: T;
  constructor() {
    super();
  }
}

class C<T> extends B<T> {
  y: T;
  constructor() {
    super();
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%class.2 = class(A {
// CHECK-NEXT:  %homeObject: %class.3
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.4 = class_constructor(%class.2)
// CHECK-NEXT:%class.5 = class(B extends %class.2 {
// CHECK-NEXT:  %constructor: %function.6
// CHECK-NEXT:  %homeObject: %class.7
// CHECK-NEXT:  x: number
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.8 = class_constructor(%class.5)
// CHECK-NEXT:%function.9 = function(this: %class.2, val: %class.5): void
// CHECK-NEXT:%class.3 = class( {
// CHECK-NEXT:  foo [final]: %function.9
// CHECK-NEXT:})
// CHECK-NEXT:%function.6 = function(this: %class.5): void
// CHECK-NEXT:%class.7 = class( extends %class.3 {
// CHECK-NEXT:})
// CHECK-NEXT:%class.10 = class(C extends %class.5 {
// CHECK-NEXT:  %constructor: %function.11
// CHECK-NEXT:  %homeObject: %class.12
// CHECK-NEXT:  y: number
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.13 = class_constructor(%class.10)
// CHECK-NEXT:%function.11 = function(this: %class.10): void
// CHECK-NEXT:%class.12 = class( extends %class.7 {
// CHECK-NEXT:})
// CHECK-NEXT:%union.14 = union(null | %class.10)

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'outer' Let : %union.14
// CHECK-NEXT:            Decl %d.3 'A' Class : %class_constructor.4
// CHECK-NEXT:            Decl %d.4 'B' Class
// CHECK-NEXT:            Decl %d.5 'C' Class
// CHECK-NEXT:            Decl %d.6 'arguments' Var Arguments
// CHECK-NEXT:            Decl %d.7 'B' Class : %class_constructor.8
// CHECK-NEXT:            Decl %d.8 'C' Class : %class_constructor.13
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:                Decl %d.9 'val' Parameter : %class.5
// CHECK-NEXT:                Decl %d.10 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.4
// CHECK-NEXT:                Decl %d.11 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.5
// CHECK-NEXT:                Decl %d.12 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.6
// CHECK-NEXT:                Decl %d.13 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.7
// CHECK-NEXT:                Decl %d.14 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            NullLiteral : null
// CHECK-NEXT:                            Id 'outer' [D:E:%d.2 'outer']
// CHECK-NEXT:                    ClassDeclaration
// CHECK-NEXT:                        Id 'A' [D:E:%d.3 'A']
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            MethodDefinition : %function.9
// CHECK-NEXT:                                Id 'foo'
// CHECK-NEXT:                                FunctionExpression : %function.9
// CHECK-NEXT:                                    Id 'val' [D:E:%d.9 'val']
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                    ClassDeclaration
// CHECK-NEXT:                        Id 'B' [D:E:%d.7 'B']
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                        Id 'A' [D:E:%d.3 'A'] : %class_constructor.4
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            ClassProperty : number
// CHECK-NEXT:                                Id 'x'
// CHECK-NEXT:                            MethodDefinition : %function.6
// CHECK-NEXT:                                Id 'constructor'
// CHECK-NEXT:                                FunctionExpression : %function.6
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        ExpressionStatement
// CHECK-NEXT:                                            CallExpression
// CHECK-NEXT:                                                Super
// CHECK-NEXT:                    ClassDeclaration
// CHECK-NEXT:                        Id 'B' [D:E:%d.4 'B']
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                        Id 'A' [D:E:%d.3 'A']
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            ClassProperty
// CHECK-NEXT:                                Id 'x'
// CHECK-NEXT:                            MethodDefinition
// CHECK-NEXT:                                Id 'constructor'
// CHECK-NEXT:                                FunctionExpression
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        ExpressionStatement
// CHECK-NEXT:                                            CallExpression
// CHECK-NEXT:                                                Super
// CHECK-NEXT:                    ClassDeclaration
// CHECK-NEXT:                        Id 'C' [D:E:%d.8 'C']
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                        Id 'B' [D:E:%d.7 'B'] : %class_constructor.8
// CHECK-NEXT:                        TypeParameterInstantiation
// CHECK-NEXT:                            GenericTypeAnnotation
// CHECK-NEXT:                                Id 'T'
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            ClassProperty : number
// CHECK-NEXT:                                Id 'y'
// CHECK-NEXT:                            MethodDefinition : %function.11
// CHECK-NEXT:                                Id 'constructor'
// CHECK-NEXT:                                FunctionExpression : %function.11
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        ExpressionStatement
// CHECK-NEXT:                                            CallExpression : void
// CHECK-NEXT:                                                Super : %function.6
// CHECK-NEXT:                    ClassDeclaration
// CHECK-NEXT:                        Id 'C' [D:E:%d.5 'C']
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                        Id 'B' [D:E:%d.4 'B']
// CHECK-NEXT:                        TypeParameterInstantiation
// CHECK-NEXT:                            GenericTypeAnnotation
// CHECK-NEXT:                                Id 'T'
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            ClassProperty
// CHECK-NEXT:                                Id 'y'
// CHECK-NEXT:                            MethodDefinition
// CHECK-NEXT:                                Id 'constructor'
// CHECK-NEXT:                                FunctionExpression
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        ExpressionStatement
// CHECK-NEXT:                                            CallExpression
// CHECK-NEXT:                                                Super
// CHECK-NEXT:            ObjectExpression
