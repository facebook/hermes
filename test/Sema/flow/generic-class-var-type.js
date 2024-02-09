/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals -typed -dump-sema %s | %FileCheckOrRegen %s --match-full-lines

// All specializations should realize that counter is a number.
let counter: number = 0;

class A {
  f: F<boolean>;
}

class F<T> {
  x: T;
  inc(): void {
    ++counter; // make sure this is number.
  }
}

let fString: F<string>;

new F<number>();

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%class.2 = class(A {
// CHECK-NEXT:  %homeObject: %class.3
// CHECK-NEXT:  f: %class.4
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.5 = class_constructor(%class.2)
// CHECK-NEXT:%class.4 = class(F {
// CHECK-NEXT:  %homeObject: %class.6
// CHECK-NEXT:  x: boolean
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.7 = class_constructor(%class.4)
// CHECK-NEXT:%class.3 = class( {
// CHECK-NEXT:})
// CHECK-NEXT:%function.8 = function(this: %class.4): void
// CHECK-NEXT:%class.6 = class( {
// CHECK-NEXT:  inc [final]: %function.8
// CHECK-NEXT:})
// CHECK-NEXT:%class.9 = class(F {
// CHECK-NEXT:  %homeObject: %class.10
// CHECK-NEXT:  x: string
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.11 = class_constructor(%class.9)
// CHECK-NEXT:%function.12 = function(this: %class.9): void
// CHECK-NEXT:%class.10 = class( {
// CHECK-NEXT:  inc [final]: %function.12
// CHECK-NEXT:})
// CHECK-NEXT:%class.13 = class(F {
// CHECK-NEXT:  %homeObject: %class.14
// CHECK-NEXT:  x: number
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.15 = class_constructor(%class.13)
// CHECK-NEXT:%function.16 = function(this: %class.13): void
// CHECK-NEXT:%class.14 = class( {
// CHECK-NEXT:  inc [final]: %function.16
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'counter' Let : number
// CHECK-NEXT:            Decl %d.3 'A' Class : %class_constructor.5
// CHECK-NEXT:            Decl %d.4 'F' Class
// CHECK-NEXT:            Decl %d.5 'fString' Let : %class.9
// CHECK-NEXT:            Decl %d.6 'arguments' Var Arguments
// CHECK-NEXT:            Decl %d.7 'F' Class : %class_constructor.7
// CHECK-NEXT:            Decl %d.8 'F' Class : %class_constructor.11
// CHECK-NEXT:            Decl %d.9 'F' Class : %class_constructor.15
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.3
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

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            NumericLiteral : number
// CHECK-NEXT:                            Id 'counter' [D:E:%d.2 'counter']
// CHECK-NEXT:                    ClassDeclaration
// CHECK-NEXT:                        Id 'A' [D:E:%d.3 'A']
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            ClassProperty : %class.4
// CHECK-NEXT:                                Id 'f'
// CHECK-NEXT:                    ClassDeclaration
// CHECK-NEXT:                        Id 'F' [D:E:%d.7 'F']
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            ClassProperty : boolean
// CHECK-NEXT:                                Id 'x'
// CHECK-NEXT:                            MethodDefinition : %function.8
// CHECK-NEXT:                                Id 'inc'
// CHECK-NEXT:                                FunctionExpression : %function.8
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        ExpressionStatement
// CHECK-NEXT:                                            UpdateExpression : number
// CHECK-NEXT:                                                Id 'counter' [D:E:%d.2 'counter'] : number
// CHECK-NEXT:                    ClassDeclaration
// CHECK-NEXT:                        Id 'F' [D:E:%d.8 'F']
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            ClassProperty : string
// CHECK-NEXT:                                Id 'x'
// CHECK-NEXT:                            MethodDefinition : %function.12
// CHECK-NEXT:                                Id 'inc'
// CHECK-NEXT:                                FunctionExpression : %function.12
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        ExpressionStatement
// CHECK-NEXT:                                            UpdateExpression : number
// CHECK-NEXT:                                                Id 'counter' [D:E:%d.2 'counter'] : number
// CHECK-NEXT:                    ClassDeclaration
// CHECK-NEXT:                        Id 'F' [D:E:%d.9 'F']
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            ClassProperty : number
// CHECK-NEXT:                                Id 'x'
// CHECK-NEXT:                            MethodDefinition : %function.16
// CHECK-NEXT:                                Id 'inc'
// CHECK-NEXT:                                FunctionExpression : %function.16
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        ExpressionStatement
// CHECK-NEXT:                                            UpdateExpression : number
// CHECK-NEXT:                                                Id 'counter' [D:E:%d.2 'counter'] : number
// CHECK-NEXT:                    ClassDeclaration
// CHECK-NEXT:                        Id 'F' [D:E:%d.4 'F']
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            ClassProperty
// CHECK-NEXT:                                Id 'x'
// CHECK-NEXT:                            MethodDefinition
// CHECK-NEXT:                                Id 'inc'
// CHECK-NEXT:                                FunctionExpression
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        ExpressionStatement
// CHECK-NEXT:                                            UpdateExpression
// CHECK-NEXT:                                                Id 'counter' [D:E:%d.2 'counter']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            Id 'fString' [D:E:%d.5 'fString']
// CHECK-NEXT:                    ExpressionStatement
// CHECK-NEXT:                        NewExpression : %class.13
// CHECK-NEXT:                            Id 'F' [D:E:%d.9 'F'] : %class_constructor.15
// CHECK-NEXT:                            TypeParameterInstantiation
// CHECK-NEXT:                                NumberTypeAnnotation
// CHECK-NEXT:            ObjectExpression
