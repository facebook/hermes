/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -dump-sema -fno-std-globals %s | %FileCheckOrRegen %s --match-full-lines

// These can be declared in block scope
{
let undefined;
let NaN;
let Infinity;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:SemContext
// CHECK-NEXT:Func loose
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'undefined' Let
// CHECK-NEXT:            Decl %d.2 'NaN' Let
// CHECK-NEXT:            Decl %d.3 'Infinity' Let

// CHECK:Program Scope %s.1
// CHECK-NEXT:    BlockStatement Scope %s.2
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                Id 'undefined' [D:E:%d.1 'undefined']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                Id 'NaN' [D:E:%d.2 'NaN']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                Id 'Infinity' [D:E:%d.3 'Infinity']
