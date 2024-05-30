/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes %s -dump-sema -fno-std-globals | %FileCheckOrRegen --match-full-lines %s

class A {}

class B extends A {
  constructor() {
    let arrow1 = () => {
      let arrow2 = () => {
        super();
      };
    };
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:SemContext
// CHECK-NEXT:Func loose
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'A' Class
// CHECK-NEXT:        Decl %d.2 'B' Class
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.3
// CHECK-NEXT:            Decl %d.3 'arrow1' Let
// CHECK-NEXT:            Decl %d.4 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.4
// CHECK-NEXT:                Decl %d.5 'arrow2' Let
// CHECK-NEXT:            Func strict
// CHECK-NEXT:                Scope %s.5

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ClassDeclaration
// CHECK-NEXT:        Id 'A' [D:E:%d.1 'A']
// CHECK-NEXT:        ClassBody
// CHECK-NEXT:    ClassDeclaration
// CHECK-NEXT:        Id 'B' [D:E:%d.2 'B']
// CHECK-NEXT:        Id 'A' [D:E:%d.1 'A']
// CHECK-NEXT:        ClassBody
// CHECK-NEXT:            MethodDefinition
// CHECK-NEXT:                Id 'constructor'
// CHECK-NEXT:                FunctionExpression
// CHECK-NEXT:                    BlockStatement
// CHECK-NEXT:                        VariableDeclaration
// CHECK-NEXT:                            VariableDeclarator
// CHECK-NEXT:                                ArrowFunctionExpression
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        VariableDeclaration
// CHECK-NEXT:                                            VariableDeclarator
// CHECK-NEXT:                                                ArrowFunctionExpression
// CHECK-NEXT:                                                    BlockStatement
// CHECK-NEXT:                                                        ExpressionStatement
// CHECK-NEXT:                                                            CallExpression
// CHECK-NEXT:                                                                Super
// CHECK-NEXT:                                                Id 'arrow2' [D:E:%d.5 'arrow2']
// CHECK-NEXT:                                Id 'arrow1' [D:E:%d.3 'arrow1']
