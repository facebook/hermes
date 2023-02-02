/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -dump-sema -fno-std-globals %s | %FileCheckOrRegen %s --match-full-lines

// Ensure that we can assign to eval in loose mode.
let eval;
function good() {
    eval = 5;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:SemContext
// CHECK-NEXT:Func loose
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'eval' Let
// CHECK-NEXT:        Decl %d.2 'good' GlobalProperty
// CHECK-NEXT:        hoistedFunction good
// CHECK-NEXT:    Func loose
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.3 'arguments' Var Arguments
// CHECK-NEXT:            Scope %s.3

// CHECK:Program Scope %s.1
// CHECK-NEXT:    VariableDeclaration
// CHECK-NEXT:        VariableDeclarator
// CHECK-NEXT:            Id 'eval' [D:E:%d.1 'eval']
// CHECK-NEXT:    FunctionDeclaration
// CHECK-NEXT:        Id 'good' [D:E:%d.2 'good']
// CHECK-NEXT:        BlockStatement Scope %s.3
// CHECK-NEXT:            ExpressionStatement
// CHECK-NEXT:                AssignmentExpression
// CHECK-NEXT:                    Id 'eval' [D:E:%d.1 'eval']
// CHECK-NEXT:                    NumericLiteral
