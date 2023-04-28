/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -dump-sema -fno-std-globals %s | %FileCheckOrRegen %s --match-full-lines

function func() {
    {
        // Inner should be promoted.
        function inner() {}
        // foo is let-scoped.
        let foo = 1;
    }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:SemContext
// CHECK-NEXT:Func loose
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'func' GlobalProperty
// CHECK-NEXT:        hoistedFunction func
// CHECK-NEXT:    Func loose
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.2 'inner' ScopedFunction
// CHECK-NEXT:            Decl %d.3 'arguments' Var Arguments
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:                Scope %s.4
// CHECK-NEXT:                    Decl %d.4 'foo' Let
// CHECK-NEXT:                    hoistedFunction inner
// CHECK-NEXT:        Func loose
// CHECK-NEXT:            Scope %s.5
// CHECK-NEXT:                Decl %d.5 'arguments' Var Arguments
// CHECK-NEXT:                Scope %s.6

// CHECK:Program Scope %s.1
// CHECK-NEXT:    FunctionDeclaration
// CHECK-NEXT:        Id 'func' [D:E:%d.1 'func']
// CHECK-NEXT:        BlockStatement Scope %s.3
// CHECK-NEXT:            BlockStatement Scope %s.4
// CHECK-NEXT:                FunctionDeclaration
// CHECK-NEXT:                    Id 'inner' [D:E:%d.2 'inner']
// CHECK-NEXT:                    BlockStatement Scope %s.6
// CHECK-NEXT:                VariableDeclaration
// CHECK-NEXT:                    VariableDeclarator
// CHECK-NEXT:                        NumericLiteral
// CHECK-NEXT:                        Id 'foo' [D:E:%d.4 'foo']
