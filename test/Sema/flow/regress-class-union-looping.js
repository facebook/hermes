/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals --typed --dump-sema %s | %FileCheckOrRegen %s --match-full-lines

class B<T> {}
type C<T> = B<D>[];
type D = [B<D>] | C<number>;

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%class_constructor.2 = class_constructor(%class.3)
// CHECK-NEXT:%class.3 = class(B {
// CHECK-NEXT:  %homeObject: %class.4
// CHECK-NEXT:})
// CHECK-NEXT:%class.4 = class( {
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'B' Class
// CHECK-NEXT:        Decl %d.3 'arguments' Var Arguments
// CHECK-NEXT:        Decl %d.4 'B' Class : %class_constructor.2
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:        Scope %s.3
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.4
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.5

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        ClassDeclaration Scope %s.3
// CHECK-NEXT:            Id 'B' [D:E:%d.4 'B']
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:        ClassDeclaration Scope %s.2
// CHECK-NEXT:            Id 'B' [D:E:%d.2 'B']
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'C'
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:            ArrayTypeAnnotation
// CHECK-NEXT:                GenericTypeAnnotation
// CHECK-NEXT:                    Id 'B'
// CHECK-NEXT:                    TypeParameterInstantiation
// CHECK-NEXT:                        GenericTypeAnnotation
// CHECK-NEXT:                            Id 'D'
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'D'
// CHECK-NEXT:            UnionTypeAnnotation
// CHECK-NEXT:                TupleTypeAnnotation
// CHECK-NEXT:                    GenericTypeAnnotation
// CHECK-NEXT:                        Id 'B'
// CHECK-NEXT:                        TypeParameterInstantiation
// CHECK-NEXT:                            GenericTypeAnnotation
// CHECK-NEXT:                                Id 'D'
// CHECK-NEXT:                GenericTypeAnnotation
// CHECK-NEXT:                    Id 'C'
// CHECK-NEXT:                    TypeParameterInstantiation
// CHECK-NEXT:                        NumberTypeAnnotation
