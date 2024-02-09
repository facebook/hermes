/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals -typed -dump-sema %s | %FileCheckOrRegen %s --match-full-lines

class Foo {}
class Bar {}
type C<T> = (T | Foo | Bar) | null;
type C_crazy = C<number | Foo | Bar> | null;

// Observe that this type collapses the duplicate Foo, Bar, null together.
var c2: C_crazy;

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%class.2 = class(Foo {
// CHECK-NEXT:  %homeObject: %class.3
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.4 = class_constructor(%class.2)
// CHECK-NEXT:%class.5 = class(Bar {
// CHECK-NEXT:  %homeObject: %class.6
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.7 = class_constructor(%class.5)
// CHECK-NEXT:%union.8 = union(null | number | %class.2 | %class.5)
// CHECK-NEXT:%union.9 = union(number | %class.2 | %class.5)
// CHECK-NEXT:%class.3 = class( {
// CHECK-NEXT:})
// CHECK-NEXT:%class.6 = class( {
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'Foo' Class : %class_constructor.4
// CHECK-NEXT:            Decl %d.3 'Bar' Class : %class_constructor.7
// CHECK-NEXT:            Decl %d.4 'c2' Var : %union.8
// CHECK-NEXT:            Decl %d.5 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    ClassDeclaration
// CHECK-NEXT:                        Id 'Foo' [D:E:%d.2 'Foo']
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                    ClassDeclaration
// CHECK-NEXT:                        Id 'Bar' [D:E:%d.3 'Bar']
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                    TypeAlias
// CHECK-NEXT:                        Id 'C'
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                        UnionTypeAnnotation
// CHECK-NEXT:                            UnionTypeAnnotation
// CHECK-NEXT:                                GenericTypeAnnotation
// CHECK-NEXT:                                    Id 'T'
// CHECK-NEXT:                                GenericTypeAnnotation
// CHECK-NEXT:                                    Id 'Foo'
// CHECK-NEXT:                                GenericTypeAnnotation
// CHECK-NEXT:                                    Id 'Bar'
// CHECK-NEXT:                            NullLiteralTypeAnnotation
// CHECK-NEXT:                    TypeAlias
// CHECK-NEXT:                        Id 'C_crazy'
// CHECK-NEXT:                        UnionTypeAnnotation
// CHECK-NEXT:                            GenericTypeAnnotation
// CHECK-NEXT:                                Id 'C'
// CHECK-NEXT:                                TypeParameterInstantiation
// CHECK-NEXT:                                    UnionTypeAnnotation
// CHECK-NEXT:                                        NumberTypeAnnotation
// CHECK-NEXT:                                        GenericTypeAnnotation
// CHECK-NEXT:                                            Id 'Foo'
// CHECK-NEXT:                                        GenericTypeAnnotation
// CHECK-NEXT:                                            Id 'Bar'
// CHECK-NEXT:                            NullLiteralTypeAnnotation
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            Id 'c2' [D:E:%d.4 'c2']
// CHECK-NEXT:            ObjectExpression
