/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror --typed --dump-sema -fno-std-globals %s | %FileCheckOrRegen %s --match-full-lines

// Make sure [B] gets deduplicated here even though it's recursive.
type A = [B] | [B] | number;
type B = [A] | number;
// Ensure sorting works.
type C = number | [A] | string;

let a: A;
let b: B;
let c: C;

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%union.2 = union(number | %tuple.4)
// CHECK-NEXT:%union.3 = union(string | number | %tuple.4)
// CHECK-NEXT:%tuple.4 = tuple(%union.2)

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'a' Let : %union.2
// CHECK-NEXT:        Decl %d.3 'b' Let : %union.2
// CHECK-NEXT:        Decl %d.4 'c' Let : %union.3
// CHECK-NEXT:        Decl %d.5 'arguments' Var Arguments

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'A'
// CHECK-NEXT:            UnionTypeAnnotation
// CHECK-NEXT:                TupleTypeAnnotation
// CHECK-NEXT:                    GenericTypeAnnotation
// CHECK-NEXT:                        Id 'B'
// CHECK-NEXT:                TupleTypeAnnotation
// CHECK-NEXT:                    GenericTypeAnnotation
// CHECK-NEXT:                        Id 'B'
// CHECK-NEXT:                NumberTypeAnnotation
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'B'
// CHECK-NEXT:            UnionTypeAnnotation
// CHECK-NEXT:                TupleTypeAnnotation
// CHECK-NEXT:                    GenericTypeAnnotation
// CHECK-NEXT:                        Id 'A'
// CHECK-NEXT:                NumberTypeAnnotation
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'C'
// CHECK-NEXT:            UnionTypeAnnotation
// CHECK-NEXT:                NumberTypeAnnotation
// CHECK-NEXT:                TupleTypeAnnotation
// CHECK-NEXT:                    GenericTypeAnnotation
// CHECK-NEXT:                        Id 'A'
// CHECK-NEXT:                StringTypeAnnotation
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                Id 'a' [D:E:%d.2 'a']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                Id 'b' [D:E:%d.3 'b']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                Id 'c' [D:E:%d.4 'c']
