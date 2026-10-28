/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes --typed --dump-sema -fno-std-globals %s | %FileCheckOrRegen --match-full-lines %s

type A<T> = [B<T>];
type B<T> = A<B<T>>;
type C = A<number>;

// T never reaches a leaf, so this is the finite type C = [C].
let c: C;

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%tuple.2 = tuple(%tuple.2)

// CHECK:SemContext
// CHECK-NEXT:Func strict mayReachImplicitReturn
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'c' Let : %tuple.2
// CHECK-NEXT:        Decl %d.3 'arguments' Var Arguments

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'A'
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:            TupleTypeAnnotation
// CHECK-NEXT:                GenericTypeAnnotation
// CHECK-NEXT:                    Id 'B'
// CHECK-NEXT:                    TypeParameterInstantiation
// CHECK-NEXT:                        GenericTypeAnnotation
// CHECK-NEXT:                            Id 'T'
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'B'
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:            GenericTypeAnnotation
// CHECK-NEXT:                Id 'A'
// CHECK-NEXT:                TypeParameterInstantiation
// CHECK-NEXT:                    GenericTypeAnnotation
// CHECK-NEXT:                        Id 'B'
// CHECK-NEXT:                        TypeParameterInstantiation
// CHECK-NEXT:                            GenericTypeAnnotation
// CHECK-NEXT:                                Id 'T'
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'C'
// CHECK-NEXT:            GenericTypeAnnotation
// CHECK-NEXT:                Id 'A'
// CHECK-NEXT:                TypeParameterInstantiation
// CHECK-NEXT:                    NumberTypeAnnotation
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                Id 'c' [D:E:%d.2 'c']
