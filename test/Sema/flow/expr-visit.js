/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals --typed --dump-sema %s | %FileCheckOrRegen %s --match-full-lines

// Make sure that FlowChecker visits expression in loop condition.

let x = 10;
while (x + 1) {}

// Auto-generated content below. Please do not modify manually.

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'x' Let : number
// CHECK-NEXT:        Scope %s.2

// CHECK:Program Scope %s.1
// CHECK-NEXT:    VariableDeclaration
// CHECK-NEXT:        VariableDeclarator
// CHECK-NEXT:            NumericLiteral : number
// CHECK-NEXT:            Id 'x' [D:E:%d.1 'x']
// CHECK-NEXT:    WhileStatement
// CHECK-NEXT:        BlockStatement Scope %s.2
// CHECK-NEXT:        BinaryExpression : number
// CHECK-NEXT:            Id 'x' [D:E:%d.1 'x'] : number
// CHECK-NEXT:            NumericLiteral : number
