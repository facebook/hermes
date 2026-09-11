/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -typed -dump-sema %s | %FileCheckOrRegen %s --match-full-lines

// Function declarations are hoisted for type resolution, so a variable
// initializer may reference a function declared later in the same scope without
// the reference falling back to 'any'. -Werror ensures a regression (the
// "used prior to declaration" warning) fails the test.

// Referenced from inside a closure stored in a const.
const wrap = (): boolean => isValid(1);

// Referenced directly as a value.
const alias = isValid;

function isValid(n: number): boolean {
  return n > 0;
}

print(wrap(), alias(2));

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%function.2 = function(n: number): boolean
// CHECK-NEXT:%function.3 = function(): boolean

// CHECK:SemContext
// CHECK-NEXT:Func strict mayReachImplicitReturn
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'wrap' Const : %function.3
// CHECK-NEXT:        Decl %d.3 'alias' Const : %function.2
// CHECK-NEXT:        Decl %d.4 'isValid' Var : %function.2
// CHECK-NEXT:        Decl %d.5 'arguments' Var Arguments
// CHECK-NEXT:        hoistedFunction isValid
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.3
// CHECK-NEXT:            Decl %d.6 'n' Parameter : number
// CHECK-NEXT:            Decl %d.7 'arguments' Var Arguments

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                ArrowFunctionExpression : %function.3
// CHECK-NEXT:                    BlockStatement
// CHECK-NEXT:                        ReturnStatement
// CHECK-NEXT:                            CallExpression : boolean
// CHECK-NEXT:                                Id 'isValid' [D:E:%d.4 'isValid'] : %function.2
// CHECK-NEXT:                                NumericLiteral : 1
// CHECK-NEXT:                Id 'wrap' [D:E:%d.2 'wrap']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                Id 'isValid' [D:E:%d.4 'isValid'] : %function.2
// CHECK-NEXT:                Id 'alias' [D:E:%d.3 'alias']
// CHECK-NEXT:        FunctionDeclaration : %function.2
// CHECK-NEXT:            Id 'isValid' [D:E:%d.4 'isValid']
// CHECK-NEXT:            Id 'n' [D:E:%d.6 'n']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    BinaryExpression : boolean
// CHECK-NEXT:                        Id 'n' [D:E:%d.6 'n'] : number
// CHECK-NEXT:                        NumericLiteral : 0
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            CallExpression
// CHECK-NEXT:                Id 'print' [D:E:%d.8 'print'] : any
// CHECK-NEXT:                CallExpression : boolean
// CHECK-NEXT:                    Id 'wrap' [D:E:%d.2 'wrap'] : %function.3
// CHECK-NEXT:                CallExpression : boolean
// CHECK-NEXT:                    Id 'alias' [D:E:%d.3 'alias'] : %function.2
// CHECK-NEXT:                    NumericLiteral : 2
