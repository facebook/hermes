/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals -typed -dump-sema %s | %FileCheckOrRegen %s --match-full-lines

type A = number|string;
type X = string | B<number> | B<string> | B<number> | B<number>[] | B<A> | B<number|string>;

class B<T> {
  bval: T;
  x: X;
}

type Y = X;

class C<T> {
  cval: T;
}

type Z = C<number> | C<number> | C<C<number>> | C<C<string>>;

let y: Y;
let z: Z;

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%union.2 = union(string | number)
// CHECK-NEXT:%union.3 = union(string | %array.14 | %class.15 | %class.16 | %class.17)
// CHECK-NEXT:%union.4 = union(%class.5 | %class.18 | %class.19)
// CHECK-NEXT:%class.5 = class(C {
// CHECK-NEXT:  %homeObject: %class.20
// CHECK-NEXT:  cval: number
// CHECK-NEXT:})
// CHECK-NEXT:%class.6 = class(C {
// CHECK-NEXT:  %homeObject: %class.21
// CHECK-NEXT:  cval: string
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.7 = class_constructor(%class.15)
// CHECK-NEXT:%class_constructor.8 = class_constructor(%class.16)
// CHECK-NEXT:%class_constructor.9 = class_constructor(%class.17)
// CHECK-NEXT:%class_constructor.10 = class_constructor(%class.5)
// CHECK-NEXT:%class_constructor.11 = class_constructor(%class.18)
// CHECK-NEXT:%class_constructor.12 = class_constructor(%class.6)
// CHECK-NEXT:%class_constructor.13 = class_constructor(%class.19)
// CHECK-NEXT:%array.14 = array(%class.15)
// CHECK-NEXT:%class.15 = class(B {
// CHECK-NEXT:  %homeObject: %class.22
// CHECK-NEXT:  bval: number
// CHECK-NEXT:  x: %union.3
// CHECK-NEXT:})
// CHECK-NEXT:%class.16 = class(B {
// CHECK-NEXT:  %homeObject: %class.23
// CHECK-NEXT:  bval: string
// CHECK-NEXT:  x: %union.3
// CHECK-NEXT:})
// CHECK-NEXT:%class.17 = class(B {
// CHECK-NEXT:  %homeObject: %class.24
// CHECK-NEXT:  bval: %union.2
// CHECK-NEXT:  x: %union.3
// CHECK-NEXT:})
// CHECK-NEXT:%class.18 = class(C {
// CHECK-NEXT:  %homeObject: %class.25
// CHECK-NEXT:  cval: %class.5
// CHECK-NEXT:})
// CHECK-NEXT:%class.19 = class(C {
// CHECK-NEXT:  %homeObject: %class.26
// CHECK-NEXT:  cval: %class.6
// CHECK-NEXT:})
// CHECK-NEXT:%class.20 = class( {
// CHECK-NEXT:})
// CHECK-NEXT:%class.21 = class( {
// CHECK-NEXT:})
// CHECK-NEXT:%class.22 = class( {
// CHECK-NEXT:})
// CHECK-NEXT:%class.23 = class( {
// CHECK-NEXT:})
// CHECK-NEXT:%class.24 = class( {
// CHECK-NEXT:})
// CHECK-NEXT:%class.25 = class( {
// CHECK-NEXT:})
// CHECK-NEXT:%class.26 = class( {
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'B' Class
// CHECK-NEXT:        Decl %d.3 'C' Class
// CHECK-NEXT:        Decl %d.4 'y' Let : %union.3
// CHECK-NEXT:        Decl %d.5 'z' Let : %union.4
// CHECK-NEXT:        Decl %d.6 'arguments' Var Arguments
// CHECK-NEXT:        Decl %d.7 'B' Class : %class_constructor.7
// CHECK-NEXT:        Decl %d.8 'B' Class : %class_constructor.8
// CHECK-NEXT:        Decl %d.9 'B' Class : %class_constructor.9
// CHECK-NEXT:        Decl %d.10 'C' Class : %class_constructor.10
// CHECK-NEXT:        Decl %d.11 'C' Class : %class_constructor.11
// CHECK-NEXT:        Decl %d.12 'C' Class : %class_constructor.12
// CHECK-NEXT:        Decl %d.13 'C' Class : %class_constructor.13
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:        Scope %s.3
// CHECK-NEXT:        Scope %s.4
// CHECK-NEXT:        Scope %s.5
// CHECK-NEXT:        Scope %s.6
// CHECK-NEXT:        Scope %s.7
// CHECK-NEXT:        Scope %s.8
// CHECK-NEXT:        Scope %s.9
// CHECK-NEXT:        Scope %s.10
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.11
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.12
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.13
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.14
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.15
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.16
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.17
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.18
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.19

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'A'
// CHECK-NEXT:            UnionTypeAnnotation
// CHECK-NEXT:                NumberTypeAnnotation
// CHECK-NEXT:                StringTypeAnnotation
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'X'
// CHECK-NEXT:            UnionTypeAnnotation
// CHECK-NEXT:                StringTypeAnnotation
// CHECK-NEXT:                GenericTypeAnnotation
// CHECK-NEXT:                    Id 'B'
// CHECK-NEXT:                    TypeParameterInstantiation
// CHECK-NEXT:                        NumberTypeAnnotation
// CHECK-NEXT:                GenericTypeAnnotation
// CHECK-NEXT:                    Id 'B'
// CHECK-NEXT:                    TypeParameterInstantiation
// CHECK-NEXT:                        StringTypeAnnotation
// CHECK-NEXT:                GenericTypeAnnotation
// CHECK-NEXT:                    Id 'B'
// CHECK-NEXT:                    TypeParameterInstantiation
// CHECK-NEXT:                        NumberTypeAnnotation
// CHECK-NEXT:                ArrayTypeAnnotation
// CHECK-NEXT:                    GenericTypeAnnotation
// CHECK-NEXT:                        Id 'B'
// CHECK-NEXT:                        TypeParameterInstantiation
// CHECK-NEXT:                            NumberTypeAnnotation
// CHECK-NEXT:                GenericTypeAnnotation
// CHECK-NEXT:                    Id 'B'
// CHECK-NEXT:                    TypeParameterInstantiation
// CHECK-NEXT:                        GenericTypeAnnotation
// CHECK-NEXT:                            Id 'A'
// CHECK-NEXT:                GenericTypeAnnotation
// CHECK-NEXT:                    Id 'B'
// CHECK-NEXT:                    TypeParameterInstantiation
// CHECK-NEXT:                        UnionTypeAnnotation
// CHECK-NEXT:                            NumberTypeAnnotation
// CHECK-NEXT:                            StringTypeAnnotation
// CHECK-NEXT:        ClassDeclaration Scope %s.4
// CHECK-NEXT:            Id 'B' [D:E:%d.7 'B']
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:                ClassProperty : number
// CHECK-NEXT:                    Id 'bval'
// CHECK-NEXT:                ClassProperty : %union.3
// CHECK-NEXT:                    Id 'x'
// CHECK-NEXT:        ClassDeclaration Scope %s.5
// CHECK-NEXT:            Id 'B' [D:E:%d.8 'B']
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:                ClassProperty : string
// CHECK-NEXT:                    Id 'bval'
// CHECK-NEXT:                ClassProperty : %union.3
// CHECK-NEXT:                    Id 'x'
// CHECK-NEXT:        ClassDeclaration Scope %s.6
// CHECK-NEXT:            Id 'B' [D:E:%d.9 'B']
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:                ClassProperty : %union.2
// CHECK-NEXT:                    Id 'bval'
// CHECK-NEXT:                ClassProperty : %union.3
// CHECK-NEXT:                    Id 'x'
// CHECK-NEXT:        ClassDeclaration Scope %s.2
// CHECK-NEXT:            Id 'B' [D:E:%d.2 'B']
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:                ClassProperty
// CHECK-NEXT:                    Id 'bval'
// CHECK-NEXT:                ClassProperty
// CHECK-NEXT:                    Id 'x'
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'Y'
// CHECK-NEXT:            GenericTypeAnnotation
// CHECK-NEXT:                Id 'X'
// CHECK-NEXT:        ClassDeclaration Scope %s.7
// CHECK-NEXT:            Id 'C' [D:E:%d.10 'C']
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:                ClassProperty : number
// CHECK-NEXT:                    Id 'cval'
// CHECK-NEXT:        ClassDeclaration Scope %s.8
// CHECK-NEXT:            Id 'C' [D:E:%d.11 'C']
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:                ClassProperty : %class.5
// CHECK-NEXT:                    Id 'cval'
// CHECK-NEXT:        ClassDeclaration Scope %s.9
// CHECK-NEXT:            Id 'C' [D:E:%d.12 'C']
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:                ClassProperty : string
// CHECK-NEXT:                    Id 'cval'
// CHECK-NEXT:        ClassDeclaration Scope %s.10
// CHECK-NEXT:            Id 'C' [D:E:%d.13 'C']
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:                ClassProperty : %class.6
// CHECK-NEXT:                    Id 'cval'
// CHECK-NEXT:        ClassDeclaration Scope %s.3
// CHECK-NEXT:            Id 'C' [D:E:%d.3 'C']
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:                ClassProperty
// CHECK-NEXT:                    Id 'cval'
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'Z'
// CHECK-NEXT:            UnionTypeAnnotation
// CHECK-NEXT:                GenericTypeAnnotation
// CHECK-NEXT:                    Id 'C'
// CHECK-NEXT:                    TypeParameterInstantiation
// CHECK-NEXT:                        NumberTypeAnnotation
// CHECK-NEXT:                GenericTypeAnnotation
// CHECK-NEXT:                    Id 'C'
// CHECK-NEXT:                    TypeParameterInstantiation
// CHECK-NEXT:                        NumberTypeAnnotation
// CHECK-NEXT:                GenericTypeAnnotation
// CHECK-NEXT:                    Id 'C'
// CHECK-NEXT:                    TypeParameterInstantiation
// CHECK-NEXT:                        GenericTypeAnnotation
// CHECK-NEXT:                            Id 'C'
// CHECK-NEXT:                            TypeParameterInstantiation
// CHECK-NEXT:                                NumberTypeAnnotation
// CHECK-NEXT:                GenericTypeAnnotation
// CHECK-NEXT:                    Id 'C'
// CHECK-NEXT:                    TypeParameterInstantiation
// CHECK-NEXT:                        GenericTypeAnnotation
// CHECK-NEXT:                            Id 'C'
// CHECK-NEXT:                            TypeParameterInstantiation
// CHECK-NEXT:                                StringTypeAnnotation
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                Id 'y' [D:E:%d.4 'y']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                Id 'z' [D:E:%d.5 'z']
