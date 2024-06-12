/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -dump-sema -fno-std-globals %s | %FileCheckOrRegen %s --match-full-lines

function main() {
  'use strict';
  // These redeclarations won't error because they are in function scope.
  function f1() {}
  function f1() {}

  var f2;
  {
    // This is fine because the names are in different scopes.
    function f2() {}
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:SemContext
// CHECK-NEXT:Func loose
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'main' GlobalProperty
// CHECK-NEXT:        hoistedFunction main
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.2 'f1' Var
// CHECK-NEXT:            Decl %d.3 'f2' Var
// CHECK-NEXT:            Decl %d.4 'arguments' Var Arguments
// CHECK-NEXT:            hoistedFunction f1
// CHECK-NEXT:            hoistedFunction f1
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:                Decl %d.5 'f2' ScopedFunction
// CHECK-NEXT:                hoistedFunction f2
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.4
// CHECK-NEXT:                Decl %d.6 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.5
// CHECK-NEXT:                Decl %d.7 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.6
// CHECK-NEXT:                Decl %d.8 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    FunctionDeclaration
// CHECK-NEXT:        Id 'main' [D:E:%d.1 'main']
// CHECK-NEXT:        BlockStatement
// CHECK-NEXT:            ExpressionStatement
// CHECK-NEXT:                StringLiteral
// CHECK-NEXT:            FunctionDeclaration
// CHECK-NEXT:                Id 'f1' [D:E:%d.2 'f1']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:            FunctionDeclaration
// CHECK-NEXT:                Id 'f1' [D:E:%d.2 'f1']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:            VariableDeclaration
// CHECK-NEXT:                VariableDeclarator
// CHECK-NEXT:                    Id 'f2' [D:E:%d.3 'f2']
// CHECK-NEXT:            BlockStatement Scope %s.3
// CHECK-NEXT:                FunctionDeclaration
// CHECK-NEXT:                    Id 'f2' [D:E:%d.5 'f2']
// CHECK-NEXT:                    BlockStatement
