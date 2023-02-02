/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -dump-sema -fno-std-globals %s | %FileCheckOrRegen %s --match-full-lines

// Test that "arguments" in an arrow function captures the surrounding
// "let arguments" declaration.

let arguments = 10;
() => { print(arguments); }

function outer() {
    let arguments = 20;
    return () => arguments;
}

() => {
    let arguments = 30;
    return arguments;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:SemContext
// CHECK-NEXT:Func loose
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'arguments' Let
// CHECK-NEXT:        Decl %d.2 'outer' GlobalProperty
// CHECK-NEXT:        Decl %d.3 'print' UndeclaredGlobalProperty
// CHECK-NEXT:        hoistedFunction outer
// CHECK-NEXT:    Func loose
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:    Func loose
// CHECK-NEXT:        Scope %s.4
// CHECK-NEXT:            Decl %d.4 'arguments' Let
// CHECK-NEXT:            Scope %s.5
// CHECK-NEXT:        Func loose
// CHECK-NEXT:            Scope %s.6
// CHECK-NEXT:                Scope %s.7
// CHECK-NEXT:    Func loose
// CHECK-NEXT:        Scope %s.8
// CHECK-NEXT:            Decl %d.5 'arguments' Let
// CHECK-NEXT:            Scope %s.9

// CHECK:Program Scope %s.1
// CHECK-NEXT:    VariableDeclaration
// CHECK-NEXT:        VariableDeclarator
// CHECK-NEXT:            NumericLiteral
// CHECK-NEXT:            Id 'arguments' [D:E:%d.1 'arguments']
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        ArrowFunctionExpression
// CHECK-NEXT:            BlockStatement Scope %s.3
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    CallExpression
// CHECK-NEXT:                        Id 'print' [D:E:%d.3 'print']
// CHECK-NEXT:                        Id 'arguments' [D:E:%d.1 'arguments']
// CHECK-NEXT:    FunctionDeclaration
// CHECK-NEXT:        Id 'outer' [D:E:%d.2 'outer']
// CHECK-NEXT:        BlockStatement Scope %s.5
// CHECK-NEXT:            VariableDeclaration
// CHECK-NEXT:                VariableDeclarator
// CHECK-NEXT:                    NumericLiteral
// CHECK-NEXT:                    Id 'arguments' [D:E:%d.4 'arguments']
// CHECK-NEXT:            ReturnStatement
// CHECK-NEXT:                ArrowFunctionExpression
// CHECK-NEXT:                    BlockStatement Scope %s.7
// CHECK-NEXT:                        ReturnStatement
// CHECK-NEXT:                            Id 'arguments' [D:E:%d.4 'arguments']
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        ArrowFunctionExpression
// CHECK-NEXT:            BlockStatement Scope %s.9
// CHECK-NEXT:                VariableDeclaration
// CHECK-NEXT:                    VariableDeclarator
// CHECK-NEXT:                        NumericLiteral
// CHECK-NEXT:                        Id 'arguments' [D:E:%d.5 'arguments']
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    Id 'arguments' [D:E:%d.5 'arguments']
