/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -typed -dump-sema -fno-std-globals %s | %FileCheckOrRegen %s --match-full-lines

class A {
  #x: number;
  #y(): void {}
  foo() {
    class B {
      bar(a: A) {
        a.#x;
        a.#y();
      }
    }
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%class.2 = class(A {
// CHECK-NEXT:  %homeObject: %class.3
// CHECK-NEXT:  #x: number
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.4 = class_constructor(%class.2)
// CHECK-NEXT:%function.5 = function(this: %class.2): void
// CHECK-NEXT:%class.3 = class( {
// CHECK-NEXT:  #y [final]: %function.5
// CHECK-NEXT:  foo [final]: %untyped_function.1
// CHECK-NEXT:})
// CHECK-NEXT:%class.6 = class(B {
// CHECK-NEXT:  %homeObject: %class.7
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.8 = class_constructor(%class.6)
// CHECK-NEXT:%function.9 = function(this: %class.6, a: %class.2): any
// CHECK-NEXT:%class.7 = class( {
// CHECK-NEXT:  bar [final]: %function.9
// CHECK-NEXT:})
// CHECK-NEXT:%object.10 = object({
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'A' Class : %class_constructor.4
// CHECK-NEXT:            Decl %d.3 'arguments' Var Arguments
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:                Decl %d.4 '#x' PrivateField
// CHECK-NEXT:                Decl %d.5 '#y' PrivateMethod
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.4
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.5
// CHECK-NEXT:                Decl %d.6 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.6
// CHECK-NEXT:                Decl %d.7 'B' Class : %class_constructor.8
// CHECK-NEXT:                Decl %d.8 'arguments' Var Arguments
// CHECK-NEXT:                Scope %s.7
// CHECK-NEXT:            Func strict
// CHECK-NEXT:                Scope %s.8
// CHECK-NEXT:                    Decl %d.9 'a' Parameter : %class.2
// CHECK-NEXT:                    Decl %d.10 'arguments' Var Arguments
// CHECK-NEXT:            Func strict
// CHECK-NEXT:                Scope %s.9
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.10

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
// CHECK-NEXT:                                Id 'x' [D:E:%d.4 '#x']
// CHECK-NEXT:                            MethodDefinition : %function.5
// CHECK-NEXT:                                PrivateName
// CHECK-NEXT:                                    Id 'y' [D:E:%d.5 '#y']
// CHECK-NEXT:                                FunctionExpression : %function.5
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                            MethodDefinition : %untyped_function.1
// CHECK-NEXT:                                Id 'foo'
// CHECK-NEXT:                                FunctionExpression : %untyped_function.1
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        ClassDeclaration Scope %s.7
// CHECK-NEXT:                                            Id 'B' [D:E:%d.7 'B']
// CHECK-NEXT:                                            ClassBody
// CHECK-NEXT:                                                MethodDefinition : %function.9
// CHECK-NEXT:                                                    Id 'bar'
// CHECK-NEXT:                                                    FunctionExpression : %function.9
// CHECK-NEXT:                                                        Id 'a' [D:E:%d.9 'a']
// CHECK-NEXT:                                                        BlockStatement
// CHECK-NEXT:                                                            ExpressionStatement
// CHECK-NEXT:                                                                MemberExpression : number
// CHECK-NEXT:                                                                    Id 'a' [D:E:%d.9 'a'] : %class.2
// CHECK-NEXT:                                                                    PrivateName
// CHECK-NEXT:                                                                        Id 'x' [D:E:%d.4 '#x']
// CHECK-NEXT:                                                            ExpressionStatement
// CHECK-NEXT:                                                                CallExpression : void
// CHECK-NEXT:                                                                    MemberExpression : %function.5
// CHECK-NEXT:                                                                        Id 'a' [D:E:%d.9 'a'] : %class.2
// CHECK-NEXT:                                                                        PrivateName
// CHECK-NEXT:                                                                            Id 'y' [D:E:%d.5 '#y']
// CHECK-NEXT:            ObjectExpression : %object.10
