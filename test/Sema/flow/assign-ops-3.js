/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -fno-std-globals --typed --dump-sema %s | %FileCheckOrRegen %s --match-full-lines

// Test that += where the target is a string, correctly infers that the result
// of the addition is string and there is no need for a checked cast.
// The result type of the assignment is also string (which used to be a regression).

return function foo(x: any): string {
    let res = "";
    res += x;
    return res;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%function.2 = function(x: any): string

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'arguments' Var Arguments
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:                Decl %d.3 'foo' FunctionExprName : %function.2
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.4
// CHECK-NEXT:                Decl %d.4 'x' Parameter : any
// CHECK-NEXT:                Decl %d.5 'res' Let : string
// CHECK-NEXT:                Decl %d.6 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    ReturnStatement
// CHECK-NEXT:                        FunctionExpression : %function.2 Scope %s.3
// CHECK-NEXT:                            Id 'foo' [D:E:%d.3 'foo']
// CHECK-NEXT:                            Id 'x' [D:E:%d.4 'x']
// CHECK-NEXT:                            BlockStatement
// CHECK-NEXT:                                VariableDeclaration
// CHECK-NEXT:                                    VariableDeclarator
// CHECK-NEXT:                                        StringLiteral : string
// CHECK-NEXT:                                        Id 'res' [D:E:%d.5 'res']
// CHECK-NEXT:                                ExpressionStatement
// CHECK-NEXT:                                    AssignmentExpression : string
// CHECK-NEXT:                                        Id 'res' [D:E:%d.5 'res'] : string
// CHECK-NEXT:                                        Id 'x' [D:E:%d.4 'x'] : any
// CHECK-NEXT:                                ReturnStatement
// CHECK-NEXT:                                    Id 'res' [D:E:%d.5 'res'] : string
// CHECK-NEXT:            ObjectExpression
