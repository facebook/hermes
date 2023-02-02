/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -dump-sema -fno-std-globals %s | %FileCheckOrRegen %s --match-full-lines

// Because of IRGen limitations, we deviate from the spec in loose mode
// and treat "var arguments" as a new variable instead of an alias for
// the Arguments object.
//
// Make sure we are doing it.

function f1() {
    var arguments = 0;
    return arguments;
}
function f2() {
    var arguments;
    arguments = 0;
    return arguments;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:SemContext
// CHECK-NEXT:Func loose
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'f1' GlobalProperty
// CHECK-NEXT:        Decl %d.2 'f2' GlobalProperty
// CHECK-NEXT:        hoistedFunction f1
// CHECK-NEXT:        hoistedFunction f2
// CHECK-NEXT:    Func loose
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.3 'arguments' Var
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:    Func loose
// CHECK-NEXT:        Scope %s.4
// CHECK-NEXT:            Decl %d.4 'arguments' Var
// CHECK-NEXT:            Scope %s.5

// CHECK:Program Scope %s.1
// CHECK-NEXT:    FunctionDeclaration
// CHECK-NEXT:        Id 'f1' [D:E:%d.1 'f1']
// CHECK-NEXT:        BlockStatement Scope %s.3
// CHECK-NEXT:            VariableDeclaration
// CHECK-NEXT:                VariableDeclarator
// CHECK-NEXT:                    NumericLiteral
// CHECK-NEXT:                    Id 'arguments' [D:E:%d.3 'arguments']
// CHECK-NEXT:            ReturnStatement
// CHECK-NEXT:                Id 'arguments' [D:E:%d.3 'arguments']
// CHECK-NEXT:    FunctionDeclaration
// CHECK-NEXT:        Id 'f2' [D:E:%d.2 'f2']
// CHECK-NEXT:        BlockStatement Scope %s.5
// CHECK-NEXT:            VariableDeclaration
// CHECK-NEXT:                VariableDeclarator
// CHECK-NEXT:                    Id 'arguments' [D:E:%d.4 'arguments']
// CHECK-NEXT:            ExpressionStatement
// CHECK-NEXT:                AssignmentExpression
// CHECK-NEXT:                    Id 'arguments' [D:E:%d.4 'arguments']
// CHECK-NEXT:                    NumericLiteral
// CHECK-NEXT:            ReturnStatement
// CHECK-NEXT:                Id 'arguments' [D:E:%d.4 'arguments']
