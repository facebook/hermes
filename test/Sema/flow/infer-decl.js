/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals --typed --dump-sema %s | %FileCheckOrRegen %s --match-full-lines

'use strict';

let x = 0;
let y = x;
let z: number | string = x;

// Auto-generated content below. Please do not modify manually.

// CHECK:union %t.1 = union string | number

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'x' Let : number
// CHECK-NEXT:        Decl %d.2 'y' Let : number
// CHECK-NEXT:        Decl %d.3 'z' Let : union %t.1

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        StringLiteral : string
// CHECK-NEXT:    VariableDeclaration
// CHECK-NEXT:        VariableDeclarator
// CHECK-NEXT:            NumericLiteral : number
// CHECK-NEXT:            Id 'x' [D:E:%d.1 'x']
// CHECK-NEXT:    VariableDeclaration
// CHECK-NEXT:        VariableDeclarator
// CHECK-NEXT:            Id 'x' [D:E:%d.1 'x'] : number
// CHECK-NEXT:            Id 'y' [D:E:%d.2 'y']
// CHECK-NEXT:    VariableDeclaration
// CHECK-NEXT:        VariableDeclarator
// CHECK-NEXT:            Id 'x' [D:E:%d.1 'x'] : number
// CHECK-NEXT:            Id 'z' [D:E:%d.3 'z']
