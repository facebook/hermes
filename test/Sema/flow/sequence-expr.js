/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -fno-std-globals -dump-sema %s 2>&1 | %FileCheckOrRegen --match-full-lines %s

// Sequence: the value is the last expression.
var seq: string = ("ignored", 42, "kept");

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()

// CHECK:SemContext
// CHECK-NEXT:Func strict mayReachImplicitReturn
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'seq' Var : string
// CHECK-NEXT:        Decl %d.3 'arguments' Var Arguments

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                SequenceExpression : string
// CHECK-NEXT:                    StringLiteral : string
// CHECK-NEXT:                    NumericLiteral : number
// CHECK-NEXT:                    StringLiteral : string
// CHECK-NEXT:                Id 'seq' [D:E:%d.2 'seq']
