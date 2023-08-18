/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals --typed --dump-sema %s | %FileCheckOrRegen %s --match-full-lines

+1;
-1;
-1n;
!1;
~1;
~1n;
typeof 1;
void 1;
let x: number = 1;
++x;
--x;
let y: bigint = 1n;
++y;
--y;

// Auto-generated content below. Please do not modify manually.

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'x' Let : number
// CHECK-NEXT:        Decl %d.2 'y' Let : bigint

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        UnaryExpression : number
// CHECK-NEXT:            NumericLiteral : number
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        UnaryExpression : number
// CHECK-NEXT:            NumericLiteral : number
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        UnaryExpression : bigint
// CHECK-NEXT:            BigIntLiteral : bigint
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        UnaryExpression : boolean
// CHECK-NEXT:            NumericLiteral : number
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        UnaryExpression : number
// CHECK-NEXT:            NumericLiteral : number
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        UnaryExpression : bigint
// CHECK-NEXT:            BigIntLiteral : bigint
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        UnaryExpression : string
// CHECK-NEXT:            NumericLiteral : number
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        UnaryExpression : void
// CHECK-NEXT:            NumericLiteral : number
// CHECK-NEXT:    VariableDeclaration
// CHECK-NEXT:        VariableDeclarator
// CHECK-NEXT:            NumericLiteral : number
// CHECK-NEXT:            Id 'x' [D:E:%d.1 'x']
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        UpdateExpression : number
// CHECK-NEXT:            Id 'x' [D:E:%d.1 'x'] : number
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        UpdateExpression : number
// CHECK-NEXT:            Id 'x' [D:E:%d.1 'x'] : number
// CHECK-NEXT:    VariableDeclaration
// CHECK-NEXT:        VariableDeclarator
// CHECK-NEXT:            BigIntLiteral : bigint
// CHECK-NEXT:            Id 'y' [D:E:%d.2 'y']
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        UpdateExpression : bigint
// CHECK-NEXT:            Id 'y' [D:E:%d.2 'y'] : bigint
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        UpdateExpression : bigint
// CHECK-NEXT:            Id 'y' [D:E:%d.2 'y'] : bigint
