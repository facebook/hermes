/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -dump-sema -fno-std-globals %s | %FileCheckOrRegen %s --match-full-lines

// Ensure that children of class node are not resolved
class Cls {
    a;
    // TODO: private properties not supported yet.
    // #b;
    method() {}
}

// Auto-generated content below. Please do not modify manually.

// CHECK:SemContext
// CHECK-NEXT:Func loose
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'Cls' Class
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.2 'arguments' Var Arguments
// CHECK-NEXT:            Scope %s.3

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ClassDeclaration
// CHECK-NEXT:        Id 'Cls' [D:E:%d.1 'Cls']
// CHECK-NEXT:        ClassBody
// CHECK-NEXT:            ClassProperty
// CHECK-NEXT:                Id 'a'
// CHECK-NEXT:            MethodDefinition
// CHECK-NEXT:                Id 'method'
// CHECK-NEXT:                FunctionExpression
// CHECK-NEXT:                    BlockStatement Scope %s.3
