/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals --typed --dump-sema %s | %FileCheckOrRegen %s --match-full-lines

'use strict';

(function(): void {

function foo(): void {
  z = 1;
}

// Doesn't infer the type because y,z is declared after x.
// Test just makes sure it doesn't crash.
let x = y + z;
let y = 0;
let z;

})();

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%function.2 = function(): void

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:                Decl %d.3 'foo' ScopedFunction : %function.2
// CHECK-NEXT:                Decl %d.4 'x' Let : any
// CHECK-NEXT:                Decl %d.5 'y' Let : number
// CHECK-NEXT:                Decl %d.6 'z' Let : any
// CHECK-NEXT:                Decl %d.7 'arguments' Var Arguments
// CHECK-NEXT:                hoistedFunction foo
// CHECK-NEXT:            Func strict
// CHECK-NEXT:                Scope %s.4
// CHECK-NEXT:                    Decl %d.8 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    ExpressionStatement
// CHECK-NEXT:                        StringLiteral : string
// CHECK-NEXT:                    ExpressionStatement
// CHECK-NEXT:                        CallExpression : void
// CHECK-NEXT:                            FunctionExpression : %function.2
// CHECK-NEXT:                                BlockStatement
// CHECK-NEXT:                                    FunctionDeclaration : %function.2
// CHECK-NEXT:                                        Id 'foo' [D:E:%d.3 'foo']
// CHECK-NEXT:                                        BlockStatement
// CHECK-NEXT:                                            ExpressionStatement
// CHECK-NEXT:                                                AssignmentExpression : number
// CHECK-NEXT:                                                    Id 'z' [D:E:%d.6 'z'] : any
// CHECK-NEXT:                                                    NumericLiteral : number
// CHECK-NEXT:                                    VariableDeclaration
// CHECK-NEXT:                                        VariableDeclarator
// CHECK-NEXT:                                            BinaryExpression : any
// CHECK-NEXT:                                                Id 'y' [D:E:%d.5 'y'] : number
// CHECK-NEXT:                                                Id 'z' [D:E:%d.6 'z'] : any
// CHECK-NEXT:                                            Id 'x' [D:E:%d.4 'x']
// CHECK-NEXT:                                    VariableDeclaration
// CHECK-NEXT:                                        VariableDeclarator
// CHECK-NEXT:                                            NumericLiteral : number
// CHECK-NEXT:                                            Id 'y' [D:E:%d.5 'y']
// CHECK-NEXT:                                    VariableDeclaration
// CHECK-NEXT:                                        VariableDeclarator
// CHECK-NEXT:                                            Id 'z' [D:E:%d.6 'z']
// CHECK-NEXT:            ObjectExpression
