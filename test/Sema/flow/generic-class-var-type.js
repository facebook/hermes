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
// CHECK-NEXT:%class_constructor.2 = class_constructor(%class.13)
// CHECK-NEXT:%class.3 = class(F {
// CHECK-NEXT:  %homeObject: %class.14
// CHECK-NEXT:  x: boolean
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.4 = class_constructor(%class.3)
// CHECK-NEXT:%function.5 = function(this: %class.3): void
// CHECK-NEXT:%class.6 = class(F {
// CHECK-NEXT:  %homeObject: %class.15
// CHECK-NEXT:  x: string
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.7 = class_constructor(%class.6)
// CHECK-NEXT:%function.8 = function(this: %class.6): void
// CHECK-NEXT:%class.9 = class(F {
// CHECK-NEXT:  %homeObject: %class.16
// CHECK-NEXT:  x: number
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.10 = class_constructor(%class.9)
// CHECK-NEXT:%function.11 = function(this: %class.9): void
// CHECK-NEXT:%object.12 = object({
// CHECK-NEXT:})
// CHECK-NEXT:%class.13 = class(A {
// CHECK-NEXT:  %homeObject: %class.17
// CHECK-NEXT:  f: %class.3
// CHECK-NEXT:})
// CHECK-NEXT:%class.14 = class( {
// CHECK-NEXT:  inc [final]: %function.5
// CHECK-NEXT:})
// CHECK-NEXT:%class.15 = class( {
// CHECK-NEXT:  inc [final]: %function.8
// CHECK-NEXT:})
// CHECK-NEXT:%class.16 = class( {
// CHECK-NEXT:  inc [final]: %function.11
// CHECK-NEXT:})
// CHECK-NEXT:%class.17 = class( {
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'counter' Let : number
// CHECK-NEXT:            Decl %d.3 'A' Class : %class_constructor.2
// CHECK-NEXT:            Decl %d.4 'F' Class
// CHECK-NEXT:            Decl %d.5 'fString' Let : %class.6
// CHECK-NEXT:            Decl %d.6 'arguments' Var Arguments
// CHECK-NEXT:            Decl %d.7 'F' Class : %class_constructor.4
// CHECK-NEXT:            Decl %d.8 'F' Class : %class_constructor.7
// CHECK-NEXT:            Decl %d.9 'F' Class : %class_constructor.10
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:            Scope %s.4
// CHECK-NEXT:            Scope %s.5
// CHECK-NEXT:            Scope %s.6
// CHECK-NEXT:            Scope %s.7
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.8
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.9
// CHECK-NEXT:                Decl %d.10 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.10
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.11
// CHECK-NEXT:                Decl %d.11 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.12
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.13
// CHECK-NEXT:                Decl %d.12 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.14
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.15
// CHECK-NEXT:                Decl %d.13 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.16

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
// CHECK-NEXT:                    ClassDeclaration Scope %s.3
// CHECK-NEXT:                        Id 'A' [D:E:%d.3 'A']
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            ClassProperty : %class.3
// CHECK-NEXT:                                Id 'f'
// CHECK-NEXT:                    ClassDeclaration Scope %s.5
// CHECK-NEXT:                        Id 'F' [D:E:%d.7 'F']
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            ClassProperty : boolean
// CHECK-NEXT:                                Id 'x'
// CHECK-NEXT:                            MethodDefinition : %function.5
// CHECK-NEXT:                                Id 'inc'
// CHECK-NEXT:                                FunctionExpression : %function.5
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        ExpressionStatement
// CHECK-NEXT:                                            UpdateExpression : number
// CHECK-NEXT:                                                Id 'counter' [D:E:%d.2 'counter'] : number
// CHECK-NEXT:                    ClassDeclaration Scope %s.6
// CHECK-NEXT:                        Id 'F' [D:E:%d.8 'F']
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            ClassProperty : string
// CHECK-NEXT:                                Id 'x'
// CHECK-NEXT:                            MethodDefinition : %function.8
// CHECK-NEXT:                                Id 'inc'
// CHECK-NEXT:                                FunctionExpression : %function.8
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        ExpressionStatement
// CHECK-NEXT:                                            UpdateExpression : number
// CHECK-NEXT:                                                Id 'counter' [D:E:%d.2 'counter'] : number
// CHECK-NEXT:                    ClassDeclaration Scope %s.7
// CHECK-NEXT:                        Id 'F' [D:E:%d.9 'F']
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            ClassProperty : number
// CHECK-NEXT:                                Id 'x'
// CHECK-NEXT:                            MethodDefinition : %function.11
// CHECK-NEXT:                                Id 'inc'
// CHECK-NEXT:                                FunctionExpression : %function.11
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        ExpressionStatement
// CHECK-NEXT:                                            UpdateExpression : number
// CHECK-NEXT:                                                Id 'counter' [D:E:%d.2 'counter'] : number
// CHECK-NEXT:                    ClassDeclaration Scope %s.4
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
// CHECK-NEXT:                        NewExpression : %class.9
// CHECK-NEXT:                            Id 'F' [D:E:%d.9 'F'] : %class_constructor.10
// CHECK-NEXT:                            TypeParameterInstantiation
// CHECK-NEXT:                                NumberTypeAnnotation
// CHECK-NEXT:            ObjectExpression : %object.12
