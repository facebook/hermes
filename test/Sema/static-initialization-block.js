/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -dump-sema -fno-std-globals %s | %FileCheckOrRegen %s --match-full-lines

class A {
  static {
    var x;
  }
  static {
    var x;
  };
  static {
    if (true) {
      var y;
    }
    if (true) {
      sink(y);
    }
    let z;
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:SemContext
// CHECK-NEXT:Func loose
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'A' Class
// CHECK-NEXT:        Decl %d.2 'sink' UndeclaredGlobalProperty
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.3 'A' ClassExprName
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.3
// CHECK-NEXT:    StaticBlock strict
// CHECK-NEXT:        Scope %s.4
// CHECK-NEXT:            Decl %d.4 'x' Var
// CHECK-NEXT:    StaticBlock strict
// CHECK-NEXT:        Scope %s.5
// CHECK-NEXT:            Decl %d.5 'x' Var
// CHECK-NEXT:    StaticBlock strict
// CHECK-NEXT:        Scope %s.6
// CHECK-NEXT:            Decl %d.6 'y' Var
// CHECK-NEXT:            Decl %d.7 'z' Let
// CHECK-NEXT:            Scope %s.7
// CHECK-NEXT:            Scope %s.8
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.9

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ClassDeclaration Scope %s.2
// CHECK-NEXT:        Id 'A' [D:%d.1 E:%d.3 'A']
// CHECK-NEXT:        ClassBody
// CHECK-NEXT:            StaticBlock Scope %s.4
// CHECK-NEXT:                VariableDeclaration
// CHECK-NEXT:                    VariableDeclarator
// CHECK-NEXT:                        Id 'x' [D:E:%d.4 'x']
// CHECK-NEXT:            StaticBlock Scope %s.5
// CHECK-NEXT:                VariableDeclaration
// CHECK-NEXT:                    VariableDeclarator
// CHECK-NEXT:                        Id 'x' [D:E:%d.5 'x']
// CHECK-NEXT:            StaticBlock Scope %s.6
// CHECK-NEXT:                IfStatement
// CHECK-NEXT:                    BooleanLiteral
// CHECK-NEXT:                    BlockStatement Scope %s.7
// CHECK-NEXT:                        VariableDeclaration
// CHECK-NEXT:                            VariableDeclarator
// CHECK-NEXT:                                Id 'y' [D:E:%d.6 'y']
// CHECK-NEXT:                IfStatement
// CHECK-NEXT:                    BooleanLiteral
// CHECK-NEXT:                    BlockStatement Scope %s.8
// CHECK-NEXT:                        ExpressionStatement
// CHECK-NEXT:                            CallExpression
// CHECK-NEXT:                                Id 'sink' [D:E:%d.2 'sink']
// CHECK-NEXT:                                Id 'y' [D:E:%d.6 'y']
// CHECK-NEXT:                VariableDeclaration
// CHECK-NEXT:                    VariableDeclarator
// CHECK-NEXT:                        Id 'z' [D:E:%d.7 'z']
