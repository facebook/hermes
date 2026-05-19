/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes --typed --dump-sema %s 2>&1 | %FileCheckOrRegen --match-full-lines %s

let anyVal: any = null;
// Spreading 'any' degrades the result to 'any' with a warning, instead of
// being a hard error.
let result = {...anyVal, x: 1};

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}exact-object-spread-any.js:13:15: warning: ft: unsupported property for typed object, assuming 'any'
// CHECK-NEXT:let result = {...anyVal, x: 1};
// CHECK-NEXT:              ^~~~~~~~~
// CHECK-NEXT:%untyped_function.1 = untyped_function()

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'anyVal' Let : any
// CHECK-NEXT:        Decl %d.3 'result' Let : any
// CHECK-NEXT:        Decl %d.4 'arguments' Var Arguments

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                NullLiteral : null
// CHECK-NEXT:                Id 'anyVal' [D:E:%d.2 'anyVal']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                ObjectExpression : any
// CHECK-NEXT:                    SpreadElement
// CHECK-NEXT:                        Id 'anyVal' [D:E:%d.2 'anyVal'] : any
// CHECK-NEXT:                    Property
// CHECK-NEXT:                        Id 'x'
// CHECK-NEXT:                        NumericLiteral : number
// CHECK-NEXT:                Id 'result' [D:E:%d.3 'result']
