/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals --typed --dump-sema %s | %FileCheckOrRegen %s --match-full-lines

function f(x: any, y: number) {
  // ImplicitCheckedCast to number.
  y = x;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function %t.1 = function (x: any, y: number): any

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'f' GlobalProperty : any
// CHECK-NEXT:        hoistedFunction f
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.2 'x' Parameter : any
// CHECK-NEXT:            Decl %d.3 'y' Parameter : number
// CHECK-NEXT:            Decl %d.4 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    FunctionDeclaration : function %t.1
// CHECK-NEXT:        Id 'f' [D:E:%d.1 'f']
// CHECK-NEXT:        Id 'x' [D:E:%d.2 'x']
// CHECK-NEXT:        Id 'y' [D:E:%d.3 'y']
// CHECK-NEXT:        BlockStatement
// CHECK-NEXT:            ExpressionStatement
// CHECK-NEXT:                AssignmentExpression : number
// CHECK-NEXT:                    Id 'y' [D:E:%d.3 'y'] : number
// CHECK-NEXT:                    ImplicitCheckedCast : number
// CHECK-NEXT:                        Id 'x' [D:E:%d.2 'x'] : any
