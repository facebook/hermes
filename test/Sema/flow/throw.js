/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -fno-std-globals --typed --dump-sema %s | %FileCheckOrRegen %s --match-full-lines

// Make sure that FlowChecker visits the argument of a throw statement.

let x = 10;
throw x + 1;

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()

// CHECK:SemContext
// CHECK-NEXT:Func strict noImplicitReturn
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'x' Let : number
// CHECK-NEXT:        Decl %d.3 'arguments' Var Arguments

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                NumericLiteral : number
// CHECK-NEXT:                Id 'x' [D:E:%d.2 'x']
// CHECK-NEXT:        ThrowStatement
// CHECK-NEXT:            BinaryExpression : number
// CHECK-NEXT:                Id 'x' [D:E:%d.2 'x'] : number
// CHECK-NEXT:                BinOp +
// CHECK-NEXT:                NumericLiteral : number
