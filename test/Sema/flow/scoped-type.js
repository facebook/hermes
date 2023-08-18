/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals --typed --dump-sema %s | %FileCheckOrRegen %s --match-full-lines

'use strict';

function foo() {
  type A = number;

  {
    type A = string;
    let x: A = 'a';
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:untyped function %t.1 = untyped function ()

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'foo' GlobalProperty : any
// CHECK-NEXT:        hoistedFunction foo
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.2 'arguments' Var Arguments
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:                Decl %d.3 'x' Let : string

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        StringLiteral : string
// CHECK-NEXT:    FunctionDeclaration
// CHECK-NEXT:        Id 'foo' [D:E:%d.1 'foo']
// CHECK-NEXT:        BlockStatement
// CHECK-NEXT:            TypeAlias
// CHECK-NEXT:                Id 'A'
// CHECK-NEXT:                NumberTypeAnnotation
// CHECK-NEXT:            BlockStatement Scope %s.3
// CHECK-NEXT:                TypeAlias
// CHECK-NEXT:                    Id 'A'
// CHECK-NEXT:                    StringTypeAnnotation
// CHECK-NEXT:                VariableDeclaration
// CHECK-NEXT:                    VariableDeclarator
// CHECK-NEXT:                        StringLiteral : string
// CHECK-NEXT:                        Id 'x' [D:E:%d.3 'x']
