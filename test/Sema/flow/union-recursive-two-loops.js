/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes --typed --dump-sema -fno-std-globals %s | %FileCheckOrRegen %s --match-full-lines

type A = B[] | B[];
type B = A;

type X = Y[];
type Y = X | X;

type U = A | X;

// These two types should be the same.
// The two loops have two different paths to reach them,
// but they should be uniqued properly.
let a: A;
let u: U;

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%array.2 = array(%array.2)

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'a' Let : %array.2
// CHECK-NEXT:            Decl %d.3 'u' Let : %array.2
// CHECK-NEXT:            Decl %d.4 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    TypeAlias
// CHECK-NEXT:                        Id 'A'
// CHECK-NEXT:                        UnionTypeAnnotation
// CHECK-NEXT:                            ArrayTypeAnnotation
// CHECK-NEXT:                                GenericTypeAnnotation
// CHECK-NEXT:                                    Id 'B'
// CHECK-NEXT:                            ArrayTypeAnnotation
// CHECK-NEXT:                                GenericTypeAnnotation
// CHECK-NEXT:                                    Id 'B'
// CHECK-NEXT:                    TypeAlias
// CHECK-NEXT:                        Id 'B'
// CHECK-NEXT:                        GenericTypeAnnotation
// CHECK-NEXT:                            Id 'A'
// CHECK-NEXT:                    TypeAlias
// CHECK-NEXT:                        Id 'X'
// CHECK-NEXT:                        ArrayTypeAnnotation
// CHECK-NEXT:                            GenericTypeAnnotation
// CHECK-NEXT:                                Id 'Y'
// CHECK-NEXT:                    TypeAlias
// CHECK-NEXT:                        Id 'Y'
// CHECK-NEXT:                        UnionTypeAnnotation
// CHECK-NEXT:                            GenericTypeAnnotation
// CHECK-NEXT:                                Id 'X'
// CHECK-NEXT:                            GenericTypeAnnotation
// CHECK-NEXT:                                Id 'X'
// CHECK-NEXT:                    TypeAlias
// CHECK-NEXT:                        Id 'U'
// CHECK-NEXT:                        UnionTypeAnnotation
// CHECK-NEXT:                            GenericTypeAnnotation
// CHECK-NEXT:                                Id 'A'
// CHECK-NEXT:                            GenericTypeAnnotation
// CHECK-NEXT:                                Id 'X'
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            Id 'a' [D:E:%d.2 'a']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            Id 'u' [D:E:%d.3 'u']
// CHECK-NEXT:            ObjectExpression
