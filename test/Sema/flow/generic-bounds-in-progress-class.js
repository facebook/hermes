/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -typed -dump-sema %s | %FileCheckOrRegen --match-full-lines %s

// Bounds checked against a class whose specialization is still in progress.

class Base {}
class A<T: Base> {}
class B<T> extends Base {
  x: A<B<T>> | null = null;
}
let b: B<number>;
function foo(): void {
  new B<string>();
}

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%class_constructor.2 = class_constructor(%class.12)
// CHECK-NEXT:%class.3 = class(B extends %class.12 {
// CHECK-NEXT:  %homeObject: %class.13
// CHECK-NEXT:  x: %union.6
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.4 = class_constructor(%class.3)
// CHECK-NEXT:%class_constructor.5 = class_constructor(%class.14)
// CHECK-NEXT:%union.6 = union(null | %class.14)
// CHECK-NEXT:%function.7 = function(): void
// CHECK-NEXT:%class.8 = class(B extends %class.12 {
// CHECK-NEXT:  %homeObject: %class.15
// CHECK-NEXT:  x: %union.11
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.9 = class_constructor(%class.8)
// CHECK-NEXT:%class_constructor.10 = class_constructor(%class.16)
// CHECK-NEXT:%union.11 = union(null | %class.16)
// CHECK-NEXT:%class.12 = class(Base {
// CHECK-NEXT:  %homeObject: %class.17
// CHECK-NEXT:})
// CHECK-NEXT:%class.13 = class( extends %class.17 {
// CHECK-NEXT:})
// CHECK-NEXT:%class.14 = class(A {
// CHECK-NEXT:  %homeObject: %class.18
// CHECK-NEXT:})
// CHECK-NEXT:%class.15 = class( extends %class.17 {
// CHECK-NEXT:})
// CHECK-NEXT:%class.16 = class(A {
// CHECK-NEXT:  %homeObject: %class.19
// CHECK-NEXT:})
// CHECK-NEXT:%class.17 = class( {
// CHECK-NEXT:})
// CHECK-NEXT:%class.18 = class( {
// CHECK-NEXT:})
// CHECK-NEXT:%class.19 = class( {
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict mayReachImplicitReturn
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'Base' Class : %class_constructor.2
// CHECK-NEXT:        Decl %d.3 'A' Class
// CHECK-NEXT:        Decl %d.4 'B' Class
// CHECK-NEXT:        Decl %d.5 'b' Let : %class.3
// CHECK-NEXT:        Decl %d.6 'foo' Var : %function.7
// CHECK-NEXT:        Decl %d.7 'arguments' Var Arguments
// CHECK-NEXT:        Decl %d.8 'B' Class : %class_constructor.4
// CHECK-NEXT:        Decl %d.9 'A' Class : %class_constructor.5
// CHECK-NEXT:        Decl %d.10 'B' Class : %class_constructor.9
// CHECK-NEXT:        Decl %d.11 'A' Class : %class_constructor.10
// CHECK-NEXT:        hoistedFunction foo
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:        Scope %s.3
// CHECK-NEXT:        Scope %s.4
// CHECK-NEXT:        Scope %s.5
// CHECK-NEXT:        Scope %s.6
// CHECK-NEXT:        Scope %s.7
// CHECK-NEXT:        Scope %s.8
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.9
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.10
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.11
// CHECK-NEXT:            Decl %d.12 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.12
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.13
// CHECK-NEXT:            Decl %d.13 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.14
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.15
// CHECK-NEXT:            Decl %d.14 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.16
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.17
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.18
// CHECK-NEXT:            Decl %d.15 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.19

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        ClassDeclaration Scope %s.2
// CHECK-NEXT:            Id 'Base' [D:E:%d.2 'Base']
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:        ClassDeclaration Scope %s.6
// CHECK-NEXT:            Id 'A' [D:E:%d.9 'A']
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:        ClassDeclaration Scope %s.8
// CHECK-NEXT:            Id 'A' [D:E:%d.11 'A']
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:        ClassDeclaration Scope %s.3
// CHECK-NEXT:            Id 'A' [D:E:%d.3 'A']
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:        ClassDeclaration Scope %s.5
// CHECK-NEXT:            Id 'B' [D:E:%d.8 'B']
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:            Id 'Base' [D:E:%d.2 'Base'] : %class_constructor.2
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:                ClassProperty : %union.6
// CHECK-NEXT:                    Id 'x'
// CHECK-NEXT:                    NullLiteral : null
// CHECK-NEXT:        ClassDeclaration Scope %s.7
// CHECK-NEXT:            Id 'B' [D:E:%d.10 'B']
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:            Id 'Base' [D:E:%d.2 'Base'] : %class_constructor.2
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:                ClassProperty : %union.11
// CHECK-NEXT:                    Id 'x'
// CHECK-NEXT:                    NullLiteral : null
// CHECK-NEXT:        ClassDeclaration Scope %s.4
// CHECK-NEXT:            Id 'B' [D:E:%d.4 'B']
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:            Id 'Base' [D:E:%d.2 'Base']
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:                ClassProperty
// CHECK-NEXT:                    Id 'x'
// CHECK-NEXT:                    NullLiteral
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                Id 'b' [D:E:%d.5 'b']
// CHECK-NEXT:        FunctionDeclaration : %function.7
// CHECK-NEXT:            Id 'foo' [D:E:%d.6 'foo']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    NewExpression : %class.8
// CHECK-NEXT:                        Id 'B' [D:E:%d.10 'B'] : %class_constructor.9
// CHECK-NEXT:                        TypeParameterInstantiation
// CHECK-NEXT:                            StringTypeAnnotation
