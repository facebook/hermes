/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror --typed --dump-sema -fno-std-globals %s | %FileCheckOrRegen %s --match-full-lines

class A {
  x = new.target; // undefined
  constructor() {
    new.target; // A
    () => new.target; // A
  }
  foo() {
    new.target; // undefined
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%class.2 = class(A {
// CHECK-NEXT:  %constructor: %function.3
// CHECK-NEXT:  %homeObject: %class.4
// CHECK-NEXT:  x: any
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.5 = class_constructor(%class.2)
// CHECK-NEXT:%function.3 = function(this: %class.2): void
// CHECK-NEXT:%class.4 = class( {
// CHECK-NEXT:  foo [final]: %untyped_function.1
// CHECK-NEXT:})
// CHECK-NEXT:%object.6 = object({
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'A' Class : %class_constructor.5
// CHECK-NEXT:            Decl %d.3 'arguments' Var Arguments
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.4
// CHECK-NEXT:                Decl %d.4 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.5
// CHECK-NEXT:                Decl %d.5 'arguments' Var Arguments
// CHECK-NEXT:            Func strict
// CHECK-NEXT:                Scope %s.6
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.7
// CHECK-NEXT:                Decl %d.6 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    ClassDeclaration Scope %s.3
// CHECK-NEXT:                        Id 'A' [D:E:%d.2 'A']
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            ClassProperty : any
// CHECK-NEXT:                                Id 'x'
// CHECK-NEXT:                                MetaProperty : void
// CHECK-NEXT:                                    Id 'new'
// CHECK-NEXT:                                    Id 'target'
// CHECK-NEXT:                            MethodDefinition : %function.3
// CHECK-NEXT:                                Id 'constructor'
// CHECK-NEXT:                                FunctionExpression : %function.3
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        ExpressionStatement
// CHECK-NEXT:                                            MetaProperty : %function.3
// CHECK-NEXT:                                                Id 'new'
// CHECK-NEXT:                                                Id 'target'
// CHECK-NEXT:                                        ExpressionStatement
// CHECK-NEXT:                                            ArrowFunctionExpression : %untyped_function.1
// CHECK-NEXT:                                                BlockStatement
// CHECK-NEXT:                                                    ReturnStatement
// CHECK-NEXT:                                                        MetaProperty : %function.3
// CHECK-NEXT:                                                            Id 'new'
// CHECK-NEXT:                                                            Id 'target'
// CHECK-NEXT:                            MethodDefinition : %untyped_function.1
// CHECK-NEXT:                                Id 'foo'
// CHECK-NEXT:                                FunctionExpression : %untyped_function.1
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        ExpressionStatement
// CHECK-NEXT:                                            MetaProperty : void
// CHECK-NEXT:                                                Id 'new'
// CHECK-NEXT:                                                Id 'target'
// CHECK-NEXT:            ObjectExpression : %object.6
