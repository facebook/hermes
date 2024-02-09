/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals -typed -dump-sema %s | %FileCheckOrRegen %s --match-full-lines

type Array<T> = T[];
var arr: Array<number>;

type A<T> = B<T>[] | T | Cls<T>;
type B<T> = A<T>[] | T;

class Cls<T> {
  x: T;
  y: A<T>;
}

type C = A<number>;
var c: C;
var d: A<string>;

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%union.2 = union(number | %array.3 | %class.4)
// CHECK-NEXT:%union.5 = union(number | %array.6)
// CHECK-NEXT:%array.3 = array(%union.5)
// CHECK-NEXT:%class.4 = class(Cls {
// CHECK-NEXT:  %homeObject: %class.7
// CHECK-NEXT:  x: number
// CHECK-NEXT:  y: %union.2
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.8 = class_constructor(%class.4)
// CHECK-NEXT:%array.6 = array(%union.2)
// CHECK-NEXT:%class.7 = class( {
// CHECK-NEXT:})
// CHECK-NEXT:%array.9 = array(number)
// CHECK-NEXT:%union.10 = union(string | %array.11 | %class.12)
// CHECK-NEXT:%union.13 = union(string | %array.14)
// CHECK-NEXT:%array.11 = array(%union.13)
// CHECK-NEXT:%class.12 = class(Cls {
// CHECK-NEXT:  %homeObject: %class.15
// CHECK-NEXT:  x: string
// CHECK-NEXT:  y: %union.10
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.16 = class_constructor(%class.12)
// CHECK-NEXT:%class.15 = class( {
// CHECK-NEXT:})
// CHECK-NEXT:%array.14 = array(%union.10)

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'arr' Var : %array.9
// CHECK-NEXT:            Decl %d.3 'Cls' Class
// CHECK-NEXT:            Decl %d.4 'c' Var : %union.2
// CHECK-NEXT:            Decl %d.5 'd' Var : %union.10
// CHECK-NEXT:            Decl %d.6 'arguments' Var Arguments
// CHECK-NEXT:            Decl %d.7 'Cls' Class : %class_constructor.8
// CHECK-NEXT:            Decl %d.8 'Cls' Class : %class_constructor.16

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    TypeAlias
// CHECK-NEXT:                        Id 'Array'
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                        ArrayTypeAnnotation
// CHECK-NEXT:                            GenericTypeAnnotation
// CHECK-NEXT:                                Id 'T'
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            Id 'arr' [D:E:%d.2 'arr']
// CHECK-NEXT:                    TypeAlias
// CHECK-NEXT:                        Id 'A'
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                        UnionTypeAnnotation
// CHECK-NEXT:                            ArrayTypeAnnotation
// CHECK-NEXT:                                GenericTypeAnnotation
// CHECK-NEXT:                                    Id 'B'
// CHECK-NEXT:                                    TypeParameterInstantiation
// CHECK-NEXT:                                        GenericTypeAnnotation
// CHECK-NEXT:                                            Id 'T'
// CHECK-NEXT:                            GenericTypeAnnotation
// CHECK-NEXT:                                Id 'T'
// CHECK-NEXT:                            GenericTypeAnnotation
// CHECK-NEXT:                                Id 'Cls'
// CHECK-NEXT:                                TypeParameterInstantiation
// CHECK-NEXT:                                    GenericTypeAnnotation
// CHECK-NEXT:                                        Id 'T'
// CHECK-NEXT:                    TypeAlias
// CHECK-NEXT:                        Id 'B'
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                        UnionTypeAnnotation
// CHECK-NEXT:                            ArrayTypeAnnotation
// CHECK-NEXT:                                GenericTypeAnnotation
// CHECK-NEXT:                                    Id 'A'
// CHECK-NEXT:                                    TypeParameterInstantiation
// CHECK-NEXT:                                        GenericTypeAnnotation
// CHECK-NEXT:                                            Id 'T'
// CHECK-NEXT:                            GenericTypeAnnotation
// CHECK-NEXT:                                Id 'T'
// CHECK-NEXT:                    ClassDeclaration
// CHECK-NEXT:                        Id 'Cls' [D:E:%d.7 'Cls']
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            ClassProperty : number
// CHECK-NEXT:                                Id 'x'
// CHECK-NEXT:                            ClassProperty : %union.2
// CHECK-NEXT:                                Id 'y'
// CHECK-NEXT:                    ClassDeclaration
// CHECK-NEXT:                        Id 'Cls' [D:E:%d.8 'Cls']
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            ClassProperty : string
// CHECK-NEXT:                                Id 'x'
// CHECK-NEXT:                            ClassProperty : %union.10
// CHECK-NEXT:                                Id 'y'
// CHECK-NEXT:                    ClassDeclaration
// CHECK-NEXT:                        Id 'Cls' [D:E:%d.3 'Cls']
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            ClassProperty
// CHECK-NEXT:                                Id 'x'
// CHECK-NEXT:                            ClassProperty
// CHECK-NEXT:                                Id 'y'
// CHECK-NEXT:                    TypeAlias
// CHECK-NEXT:                        Id 'C'
// CHECK-NEXT:                        GenericTypeAnnotation
// CHECK-NEXT:                            Id 'A'
// CHECK-NEXT:                            TypeParameterInstantiation
// CHECK-NEXT:                                NumberTypeAnnotation
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            Id 'c' [D:E:%d.4 'c']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            Id 'd' [D:E:%d.5 'd']
// CHECK-NEXT:            ObjectExpression
