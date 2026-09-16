/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals -typed -dump-sema %s | %FileCheckOrRegen --match-full-lines %s

// Alias bounds resolve at the declaration site, not the instantiation site.
// These all typecheck; resolving bounds at the instantiation site would report
// spurious errors.

class Base {}
class Derived extends Base {}

// 'Base' resolves to the top-level Base despite being shadowed at the
// instantiation site.
type Alias<T: Base> = T;
{
  class Base {}
  let x: Alias<Derived> = new Derived();
}

// Sibling reference: U's bound T resolves to the concrete argument (Base).
type Sibling<T: Base, U: T> = U;
let s: Sibling<Base, Derived> = new Derived();

// A bound naming a type only visible at the declaration site.
{
  class Local {}
  class SubLocal extends Local {}
  type Nested<T: Local> = T;
  let n: Nested<SubLocal> = new SubLocal();
}

// Recursive bounds are completed after the enclosing specialization is
// cached.
class RecursiveBase {}

type Recursive<T: Recursive<RecursiveBase>> = T;
let recursive: Recursive<Recursive<RecursiveBase>> = new RecursiveBase();

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%class_constructor.2 = class_constructor(%class.11)
// CHECK-NEXT:%class.3 = class(Derived extends %class.11 {
// CHECK-NEXT:  %homeObject: %class.12
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.4 = class_constructor(%class.3)
// CHECK-NEXT:%class.5 = class(RecursiveBase {
// CHECK-NEXT:  %homeObject: %class.13
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.6 = class_constructor(%class.5)
// CHECK-NEXT:%class_constructor.7 = class_constructor(%class.14)
// CHECK-NEXT:%class_constructor.8 = class_constructor(%class.15)
// CHECK-NEXT:%class.9 = class(SubLocal extends %class.15 {
// CHECK-NEXT:  %homeObject: %class.16
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.10 = class_constructor(%class.9)
// CHECK-NEXT:%class.11 = class(Base {
// CHECK-NEXT:  %homeObject: %class.17
// CHECK-NEXT:})
// CHECK-NEXT:%class.12 = class( extends %class.17 {
// CHECK-NEXT:})
// CHECK-NEXT:%class.13 = class( {
// CHECK-NEXT:})
// CHECK-NEXT:%class.14 = class(Base {
// CHECK-NEXT:  %homeObject: %class.18
// CHECK-NEXT:})
// CHECK-NEXT:%class.15 = class(Local {
// CHECK-NEXT:  %homeObject: %class.19
// CHECK-NEXT:})
// CHECK-NEXT:%class.16 = class( extends %class.19 {
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
// CHECK-NEXT:        Decl %d.3 'Derived' Class : %class_constructor.4
// CHECK-NEXT:        Decl %d.4 's' Let : %class.3
// CHECK-NEXT:        Decl %d.5 'RecursiveBase' Class : %class_constructor.6
// CHECK-NEXT:        Decl %d.6 'recursive' Let : %class.5
// CHECK-NEXT:        Decl %d.7 'arguments' Var Arguments
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:        Scope %s.3
// CHECK-NEXT:        Scope %s.4
// CHECK-NEXT:            Decl %d.8 'Base' Class : %class_constructor.7
// CHECK-NEXT:            Decl %d.9 'x' Let : %class.3
// CHECK-NEXT:            Scope %s.5
// CHECK-NEXT:        Scope %s.6
// CHECK-NEXT:            Decl %d.10 'Local' Class : %class_constructor.8
// CHECK-NEXT:            Decl %d.11 'SubLocal' Class : %class_constructor.10
// CHECK-NEXT:            Decl %d.12 'n' Let : %class.9
// CHECK-NEXT:            Scope %s.7
// CHECK-NEXT:            Scope %s.8
// CHECK-NEXT:        Scope %s.9
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.10
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.11
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.12
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.13
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.14
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.15

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
// CHECK-NEXT:        BlockStatement Scope %s.4
// CHECK-NEXT:            ClassDeclaration Scope %s.5
// CHECK-NEXT:                Id 'Base' [D:E:%d.8 'Base']
// CHECK-NEXT:                ClassBody
// CHECK-NEXT:            VariableDeclaration
// CHECK-NEXT:                VariableDeclarator
// CHECK-NEXT:                    NewExpression : %class.3
// CHECK-NEXT:                        Id 'Derived' [D:E:%d.3 'Derived'] : %class_constructor.4
// CHECK-NEXT:                    Id 'x' [D:E:%d.9 'x']
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'Sibling'
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:            GenericTypeAnnotation
// CHECK-NEXT:                Id 'U'
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                NewExpression : %class.3
// CHECK-NEXT:                    Id 'Derived' [D:E:%d.3 'Derived'] : %class_constructor.4
// CHECK-NEXT:                Id 's' [D:E:%d.4 's']
// CHECK-NEXT:        BlockStatement Scope %s.6
// CHECK-NEXT:            ClassDeclaration Scope %s.7
// CHECK-NEXT:                Id 'Local' [D:E:%d.10 'Local']
// CHECK-NEXT:                ClassBody
// CHECK-NEXT:            ClassDeclaration Scope %s.8
// CHECK-NEXT:                Id 'SubLocal' [D:E:%d.11 'SubLocal']
// CHECK-NEXT:                Id 'Local' [D:E:%d.10 'Local'] : %class_constructor.8
// CHECK-NEXT:                ClassBody
// CHECK-NEXT:            TypeAlias
// CHECK-NEXT:                Id 'Nested'
// CHECK-NEXT:                TypeParameterDeclaration
// CHECK-NEXT:                    TypeParameter
// CHECK-NEXT:                GenericTypeAnnotation
// CHECK-NEXT:                    Id 'T'
// CHECK-NEXT:            VariableDeclaration
// CHECK-NEXT:                VariableDeclarator
// CHECK-NEXT:                    NewExpression : %class.9
// CHECK-NEXT:                        Id 'SubLocal' [D:E:%d.11 'SubLocal'] : %class_constructor.10
// CHECK-NEXT:                    Id 'n' [D:E:%d.12 'n']
// CHECK-NEXT:        ClassDeclaration Scope %s.9
// CHECK-NEXT:            Id 'RecursiveBase' [D:E:%d.5 'RecursiveBase']
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'Recursive'
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:            GenericTypeAnnotation
// CHECK-NEXT:                Id 'T'
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                NewExpression : %class.5
// CHECK-NEXT:                    Id 'RecursiveBase' [D:E:%d.5 'RecursiveBase'] : %class_constructor.6
// CHECK-NEXT:                Id 'recursive' [D:E:%d.6 'recursive']
