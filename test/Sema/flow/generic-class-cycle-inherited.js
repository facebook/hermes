/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals -typed -dump-sema %s | %FileCheckOrRegen %s --match-full-lines

class A {
  foo(b: B<number>): void {}
}

class B<T> extends A {}

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%class_constructor.2 = class_constructor(%class.7)
// CHECK-NEXT:%class.3 = class(B extends %class.7 {
// CHECK-NEXT:  %homeObject: %class.8
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.4 = class_constructor(%class.3)
// CHECK-NEXT:%function.5 = function(this: %class.7, b: %class.3): void
// CHECK-NEXT:%object.6 = object({
// CHECK-NEXT:})
// CHECK-NEXT:%class.7 = class(A {
// CHECK-NEXT:  %homeObject: %class.9
// CHECK-NEXT:})
// CHECK-NEXT:%class.8 = class( extends %class.9 {
// CHECK-NEXT:})
// CHECK-NEXT:%class.9 = class( {
// CHECK-NEXT:  foo [final]: %function.5
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'A' Class : %class_constructor.2
// CHECK-NEXT:            Decl %d.3 'B' Class
// CHECK-NEXT:            Decl %d.4 'arguments' Var Arguments
// CHECK-NEXT:            Decl %d.5 'B' Class : %class_constructor.4
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:            Scope %s.4
// CHECK-NEXT:            Scope %s.5
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.6
// CHECK-NEXT:                Decl %d.6 'b' Parameter : %class.3
// CHECK-NEXT:                Decl %d.7 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.7
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.8
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.9

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    ClassDeclaration Scope %s.3
// CHECK-NEXT:                        Id 'A' [D:E:%d.2 'A']
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            MethodDefinition : %function.5
// CHECK-NEXT:                                Id 'foo'
// CHECK-NEXT:                                FunctionExpression : %function.5
// CHECK-NEXT:                                    Id 'b' [D:E:%d.6 'b']
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                    ClassDeclaration Scope %s.5
// CHECK-NEXT:                        Id 'B' [D:E:%d.5 'B']
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                        Id 'A' [D:E:%d.2 'A'] : %class_constructor.2
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                    ClassDeclaration Scope %s.4
// CHECK-NEXT:                        Id 'B' [D:E:%d.3 'B']
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                        Id 'A' [D:E:%d.2 'A']
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:            ObjectExpression : %object.6
