/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -typed -dump-sema -fno-std-globals %s | %FileCheckOrRegen %s --match-full-lines

class A {
  #a1: number;
  #a2(): void {
    this.#a2();
  }
}

class B {
  #b1: number;
  #b2(): void {}
}

class C extends B {
  #b1: string;
  #b2(): number {
    return 1;
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%class.2 = class(A {
// CHECK-NEXT:  %homeObject: %class.3
// CHECK-NEXT:  #a1: number
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.4 = class_constructor(%class.2)
// CHECK-NEXT:%class.5 = class(B {
// CHECK-NEXT:  %homeObject: %class.6
// CHECK-NEXT:  #b1: number
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.7 = class_constructor(%class.5)
// CHECK-NEXT:%class.8 = class(C extends %class.5 {
// CHECK-NEXT:  %homeObject: %class.9
// CHECK-NEXT:  #b1: string
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.10 = class_constructor(%class.8)
// CHECK-NEXT:%function.11 = function(this: %class.2): void
// CHECK-NEXT:%class.3 = class( {
// CHECK-NEXT:  #a2 [final]: %function.11
// CHECK-NEXT:})
// CHECK-NEXT:%function.12 = function(this: %class.5): void
// CHECK-NEXT:%class.6 = class( {
// CHECK-NEXT:  #b2 [final]: %function.12
// CHECK-NEXT:})
// CHECK-NEXT:%function.13 = function(this: %class.8): number
// CHECK-NEXT:%class.9 = class( extends %class.6 {
// CHECK-NEXT:  #b2 [final]: %function.13
// CHECK-NEXT:})
// CHECK-NEXT:%object.14 = object({
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'A' Class : %class_constructor.4
// CHECK-NEXT:            Decl %d.3 'B' Class : %class_constructor.7
// CHECK-NEXT:            Decl %d.4 'C' Class : %class_constructor.10
// CHECK-NEXT:            Decl %d.5 'arguments' Var Arguments
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:                Decl %d.6 '#a1' PrivateField
// CHECK-NEXT:                Decl %d.7 '#a2' PrivateMethod
// CHECK-NEXT:            Scope %s.4
// CHECK-NEXT:                Decl %d.8 '#b1' PrivateField
// CHECK-NEXT:                Decl %d.9 '#b2' PrivateMethod
// CHECK-NEXT:            Scope %s.5
// CHECK-NEXT:                Decl %d.10 '#b1' PrivateField
// CHECK-NEXT:                Decl %d.11 '#b2' PrivateMethod
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.6
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.7
// CHECK-NEXT:                Decl %d.12 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.8
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.9
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.10
// CHECK-NEXT:                Decl %d.13 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.11
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.12
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.13
// CHECK-NEXT:                Decl %d.14 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.14

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    ClassDeclaration Scope %s.3
// CHECK-NEXT:                        Id 'A' [D:E:%d.2 'A']
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            ClassPrivateProperty : number
// CHECK-NEXT:                                Id 'a1' [D:E:%d.6 '#a1']
// CHECK-NEXT:                            MethodDefinition : %function.11
// CHECK-NEXT:                                PrivateName
// CHECK-NEXT:                                    Id 'a2' [D:E:%d.7 '#a2']
// CHECK-NEXT:                                FunctionExpression : %function.11
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        ExpressionStatement
// CHECK-NEXT:                                            CallExpression : void
// CHECK-NEXT:                                                MemberExpression : %function.11
// CHECK-NEXT:                                                    ThisExpression : %class.2
// CHECK-NEXT:                                                    PrivateName
// CHECK-NEXT:                                                        Id 'a2' [D:E:%d.7 '#a2']
// CHECK-NEXT:                    ClassDeclaration Scope %s.4
// CHECK-NEXT:                        Id 'B' [D:E:%d.3 'B']
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            ClassPrivateProperty : number
// CHECK-NEXT:                                Id 'b1' [D:E:%d.8 '#b1']
// CHECK-NEXT:                            MethodDefinition : %function.12
// CHECK-NEXT:                                PrivateName
// CHECK-NEXT:                                    Id 'b2' [D:E:%d.9 '#b2']
// CHECK-NEXT:                                FunctionExpression : %function.12
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                    ClassDeclaration Scope %s.5
// CHECK-NEXT:                        Id 'C' [D:E:%d.4 'C']
// CHECK-NEXT:                        Id 'B' [D:E:%d.3 'B'] : %class_constructor.7
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            ClassPrivateProperty : string
// CHECK-NEXT:                                Id 'b1' [D:E:%d.10 '#b1']
// CHECK-NEXT:                            MethodDefinition : %function.13
// CHECK-NEXT:                                PrivateName
// CHECK-NEXT:                                    Id 'b2' [D:E:%d.11 '#b2']
// CHECK-NEXT:                                FunctionExpression : %function.13
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        ReturnStatement
// CHECK-NEXT:                                            NumericLiteral : number
// CHECK-NEXT:            ObjectExpression : %object.14
