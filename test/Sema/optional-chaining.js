/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -dump-sema -fno-std-globals %s | %FileCheckOrRegen %s --match-full-lines

function foo(x) {
  // Avoid visiting identifiers on the RHS of optional chains.
  return x?.y.z;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:SemContext
// CHECK-NEXT:Func loose
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'foo' GlobalProperty
// CHECK-NEXT:        hoistedFunction foo
// CHECK-NEXT:    Func loose
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.2 'x' Parameter
// CHECK-NEXT:            Decl %d.3 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    FunctionDeclaration
// CHECK-NEXT:        Id 'foo' [D:E:%d.1 'foo']
// CHECK-NEXT:        Id 'x' [D:E:%d.2 'x']
// CHECK-NEXT:        BlockStatement
// CHECK-NEXT:            ReturnStatement
// CHECK-NEXT:                OptionalMemberExpression
// CHECK-NEXT:                    OptionalMemberExpression
// CHECK-NEXT:                        Id 'x' [D:E:%d.2 'x']
// CHECK-NEXT:                        Id 'y'
// CHECK-NEXT:                    Id 'z'
