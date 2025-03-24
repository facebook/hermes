/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -dump-sema -fno-std-globals %s | %FileCheckOrRegen %s --match-full-lines

// These can be redeclared as 'var'.
var undefined;
var NaN;
var Infinity;

// Auto-generated content below. Please do not modify manually.

// CHECK:SemContext
// CHECK-NEXT:Func loose
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'undefined' GlobalProperty
// CHECK-NEXT:        Decl %d.2 'NaN' GlobalProperty
// CHECK-NEXT:        Decl %d.3 'Infinity' GlobalProperty

// CHECK:Program Scope %s.1
// CHECK-NEXT:    VariableDeclaration
// CHECK-NEXT:        VariableDeclarator
// CHECK-NEXT:            Id 'undefined' [D:E:%d.1 'undefined']
// CHECK-NEXT:    VariableDeclaration
// CHECK-NEXT:        VariableDeclarator
// CHECK-NEXT:            Id 'NaN' [D:E:%d.2 'NaN']
// CHECK-NEXT:    VariableDeclaration
// CHECK-NEXT:        VariableDeclarator
// CHECK-NEXT:            Id 'Infinity' [D:E:%d.3 'Infinity']
