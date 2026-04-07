/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals --typed --dump-sema %s | %FileCheckOrRegen %s --match-full-lines

type B<T> = [B<string> | number] | [string | number];
type C = B<number>;
var c: C;

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%union.2 = union(%tuple.3 | %tuple.4)
// CHECK-NEXT:%tuple.3 = tuple(%union.5)
// CHECK-NEXT:%tuple.4 = tuple(%union.6)
// CHECK-NEXT:%union.5 = union(string | number)
// CHECK-NEXT:%union.6 = union(number | %tuple.3 | %tuple.4)

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'c' Var : %union.2
// CHECK-NEXT:        Decl %d.3 'arguments' Var Arguments

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'B'
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:            UnionTypeAnnotation
// CHECK-NEXT:                TupleTypeAnnotation
// CHECK-NEXT:                    UnionTypeAnnotation
// CHECK-NEXT:                        GenericTypeAnnotation
// CHECK-NEXT:                            Id 'B'
// CHECK-NEXT:                            TypeParameterInstantiation
// CHECK-NEXT:                                StringTypeAnnotation
// CHECK-NEXT:                        NumberTypeAnnotation
// CHECK-NEXT:                TupleTypeAnnotation
// CHECK-NEXT:                    UnionTypeAnnotation
// CHECK-NEXT:                        StringTypeAnnotation
// CHECK-NEXT:                        NumberTypeAnnotation
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'C'
// CHECK-NEXT:            GenericTypeAnnotation
// CHECK-NEXT:                Id 'B'
// CHECK-NEXT:                TypeParameterInstantiation
// CHECK-NEXT:                    NumberTypeAnnotation
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                Id 'c' [D:E:%d.2 'c']
