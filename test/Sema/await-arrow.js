/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -dump-sema -fno-std-globals %s | %FileCheckOrRegen %s --match-full-lines

let await = 0;

async (x = () => { return await; }) => {}

// Auto-generated content below. Please do not modify manually.

// CHECK:SemContext
// CHECK-NEXT:Func loose
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'await' Let
// CHECK-NEXT:    Func loose
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.2 'x' Parameter
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:        Func loose
// CHECK-NEXT:            Scope %s.4

// CHECK:Program Scope %s.1
// CHECK-NEXT:    VariableDeclaration
// CHECK-NEXT:        VariableDeclarator
// CHECK-NEXT:            NumericLiteral
// CHECK-NEXT:            Id 'await' [D:E:%d.1 'await']
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        ArrowFunctionExpression
// CHECK-NEXT:            AssignmentPattern
// CHECK-NEXT:                Id 'x' [D:E:%d.2 'x']
// CHECK-NEXT:                ArrowFunctionExpression
// CHECK-NEXT:                    BlockStatement
// CHECK-NEXT:                        ReturnStatement
// CHECK-NEXT:                            Id 'await' [D:E:%d.1 'await']
// CHECK-NEXT:            BlockStatement
