/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -fno-std-globals --typed --dump-sema %s | %FileCheckOrRegen %s --match-full-lines

class A {
  constructor() {}

  f(): number {
    return 0;
  }
}

class B extends A {
  constructor() {
    super();
  }

  f(): number {
    return super.f();
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%class.2 = class(A {
// CHECK-NEXT:  %constructor: %function.3
// CHECK-NEXT:  %homeObject: %class.4
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.5 = class_constructor(%class.2)
// CHECK-NEXT:%class.6 = class(B extends %class.2 {
// CHECK-NEXT:  %constructor: %function.7
// CHECK-NEXT:  %homeObject: %class.8
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.9 = class_constructor(%class.6)
// CHECK-NEXT:%function.3 = function(this: %class.2): void
// CHECK-NEXT:%function.10 = function(this: %class.2): number
// CHECK-NEXT:%class.4 = class( {
// CHECK-NEXT:  f: %function.10
// CHECK-NEXT:})
// CHECK-NEXT:%function.7 = function(this: %class.6): void
// CHECK-NEXT:%function.11 = function(this: %class.6): number
// CHECK-NEXT:%class.8 = class( extends %class.4 {
// CHECK-NEXT:  f: %function.11
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'A' Class : %class_constructor.5
// CHECK-NEXT:            Decl %d.3 'B' Class : %class_constructor.9
// CHECK-NEXT:            Decl %d.4 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:                Decl %d.5 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.4
// CHECK-NEXT:                Decl %d.6 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.5
// CHECK-NEXT:                Decl %d.7 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.6
// CHECK-NEXT:                Decl %d.8 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    ClassDeclaration
// CHECK-NEXT:                        Id 'A' [D:E:%d.2 'A']
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            MethodDefinition : %function.3
// CHECK-NEXT:                                Id 'constructor'
// CHECK-NEXT:                                FunctionExpression : %function.3
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                            MethodDefinition : %function.10
// CHECK-NEXT:                                Id 'f'
// CHECK-NEXT:                                FunctionExpression : %function.10
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        ReturnStatement
// CHECK-NEXT:                                            NumericLiteral : number
// CHECK-NEXT:                    ClassDeclaration
// CHECK-NEXT:                        Id 'B' [D:E:%d.3 'B']
// CHECK-NEXT:                        Id 'A' [D:E:%d.2 'A'] : %class_constructor.5
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            MethodDefinition : %function.7
// CHECK-NEXT:                                Id 'constructor'
// CHECK-NEXT:                                FunctionExpression : %function.7
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        ExpressionStatement
// CHECK-NEXT:                                            CallExpression : void
// CHECK-NEXT:                                                Super : %function.3
// CHECK-NEXT:                            MethodDefinition : %function.11
// CHECK-NEXT:                                Id 'f'
// CHECK-NEXT:                                FunctionExpression : %function.11
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        ReturnStatement
// CHECK-NEXT:                                            CallExpression : number
// CHECK-NEXT:                                                MemberExpression : %function.10
// CHECK-NEXT:                                                    Super : %class.2
// CHECK-NEXT:                                                    Id 'f'
// CHECK-NEXT:            ObjectExpression
