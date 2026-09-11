/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals -typed -dump-sema %s | %FileCheckOrRegen %s --match-full-lines

class Base {}
class Derived extends Base {}

type Alias<T extends Base> = T;

class Box<T extends Alias<Derived>> {
  value: T;
}

type ConcreteBox = Box<Derived>;
let box: ConcreteBox;

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%class_constructor.2 = class_constructor(%class.7)
// CHECK-NEXT:%class.3 = class(Derived extends %class.7 {
// CHECK-NEXT:  %homeObject: %class.8
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.4 = class_constructor(%class.3)
// CHECK-NEXT:%class.5 = class(Box {
// CHECK-NEXT:  %homeObject: %class.9
// CHECK-NEXT:  value: %class.3
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.6 = class_constructor(%class.5)
// CHECK-NEXT:%class.7 = class(Base {
// CHECK-NEXT:  %homeObject: %class.10
// CHECK-NEXT:})
// CHECK-NEXT:%class.8 = class( extends %class.10 {
// CHECK-NEXT:})
// CHECK-NEXT:%class.9 = class( {
// CHECK-NEXT:})
// CHECK-NEXT:%class.10 = class( {
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict mayReachImplicitReturn
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'Base' Class : %class_constructor.2
// CHECK-NEXT:        Decl %d.3 'Derived' Class : %class_constructor.4
// CHECK-NEXT:        Decl %d.4 'Box' Class
// CHECK-NEXT:        Decl %d.5 'box' Let : %class.5
// CHECK-NEXT:        Decl %d.6 'arguments' Var Arguments
// CHECK-NEXT:        Decl %d.7 'Box' Class : %class_constructor.6
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:        Scope %s.3
// CHECK-NEXT:        Scope %s.4
// CHECK-NEXT:        Scope %s.5
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.6
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.7
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.8
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.9

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        ClassDeclaration Scope %s.2
// CHECK-NEXT:            Id 'Base' [D:E:%d.2 'Base']
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:        ClassDeclaration Scope %s.3
// CHECK-NEXT:            Id 'Derived' [D:E:%d.3 'Derived']
// CHECK-NEXT:            Id 'Base' [D:E:%d.2 'Base'] : %class_constructor.2
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'Alias'
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:            GenericTypeAnnotation
// CHECK-NEXT:                Id 'T'
// CHECK-NEXT:        ClassDeclaration Scope %s.5
// CHECK-NEXT:            Id 'Box' [D:E:%d.7 'Box']
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:                ClassProperty : %class.3
// CHECK-NEXT:                    Id 'value'
// CHECK-NEXT:        ClassDeclaration Scope %s.4
// CHECK-NEXT:            Id 'Box' [D:E:%d.4 'Box']
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:                ClassProperty
// CHECK-NEXT:                    Id 'value'
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'ConcreteBox'
// CHECK-NEXT:            GenericTypeAnnotation
// CHECK-NEXT:                Id 'Box'
// CHECK-NEXT:                TypeParameterInstantiation
// CHECK-NEXT:                    GenericTypeAnnotation
// CHECK-NEXT:                        Id 'Derived'
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                Id 'box' [D:E:%d.5 'box']
