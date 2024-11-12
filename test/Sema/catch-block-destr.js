/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -dump-sema -fno-std-globals %s | %FileCheckOrRegen %s --match-full-lines

try {} catch ([a, b]) { let x; }

// Auto-generated content below. Please do not modify manually.

// CHECK:SemContext
// CHECK-NEXT:Func loose
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:        Scope %s.3
// CHECK-NEXT:            Decl %d.1 'a' Catch
// CHECK-NEXT:            Decl %d.2 'b' Catch
// CHECK-NEXT:            Scope %s.4
// CHECK-NEXT:                Decl %d.3 'x' Let

// CHECK:Program Scope %s.1
// CHECK-NEXT:    TryStatement
// CHECK-NEXT:        BlockStatement Scope %s.2
// CHECK-NEXT:        CatchClause Scope %s.3
// CHECK-NEXT:            ArrayPattern
// CHECK-NEXT:                Id 'a' [D:E:%d.1 'a']
// CHECK-NEXT:                Id 'b' [D:E:%d.2 'b']
// CHECK-NEXT:            BlockStatement Scope %s.4
// CHECK-NEXT:                VariableDeclaration
// CHECK-NEXT:                    VariableDeclarator
// CHECK-NEXT:                        Id 'x' [D:E:%d.3 'x']
