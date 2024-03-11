/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -dump-sema -fno-std-globals %s | %FileCheckOrRegen %s --match-full-lines

class Foo {
}

class Bar extends Foo {
    constructor() {
        var f = () => { super(); }
        f();
    }
}

// Ensure works at multiple levels of nesting.
class Bar2 extends Foo {
    constructor() {
        var f0 = () => {
            var f1 = () => {
                super();
            };
            f1();
        };
        f0();
    }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:SemContext
// CHECK-NEXT:Func loose
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'Foo' Class
// CHECK-NEXT:        Decl %d.2 'Bar' Class
// CHECK-NEXT:        Decl %d.3 'Bar2' Class
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.3
// CHECK-NEXT:            Decl %d.4 'f' Var
// CHECK-NEXT:            Decl %d.5 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.4
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.5
// CHECK-NEXT:            Decl %d.6 'f0' Var
// CHECK-NEXT:            Decl %d.7 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.6
// CHECK-NEXT:                Decl %d.8 'f1' Var
// CHECK-NEXT:            Func strict
// CHECK-NEXT:                Scope %s.7

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ClassDeclaration
// CHECK-NEXT:        Id 'Foo' [D:E:%d.1 'Foo']
// CHECK-NEXT:        ClassBody
// CHECK-NEXT:    ClassDeclaration
// CHECK-NEXT:        Id 'Bar' [D:E:%d.2 'Bar']
// CHECK-NEXT:        Id 'Foo' [D:E:%d.1 'Foo']
// CHECK-NEXT:        ClassBody
// CHECK-NEXT:            MethodDefinition
// CHECK-NEXT:                Id 'constructor'
// CHECK-NEXT:                FunctionExpression
// CHECK-NEXT:                    BlockStatement
// CHECK-NEXT:                        VariableDeclaration
// CHECK-NEXT:                            VariableDeclarator
// CHECK-NEXT:                                ArrowFunctionExpression
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        ExpressionStatement
// CHECK-NEXT:                                            CallExpression
// CHECK-NEXT:                                                Super
// CHECK-NEXT:                                Id 'f' [D:E:%d.4 'f']
// CHECK-NEXT:                        ExpressionStatement
// CHECK-NEXT:                            CallExpression
// CHECK-NEXT:                                Id 'f' [D:E:%d.4 'f']
// CHECK-NEXT:    ClassDeclaration
// CHECK-NEXT:        Id 'Bar2' [D:E:%d.3 'Bar2']
// CHECK-NEXT:        Id 'Foo' [D:E:%d.1 'Foo']
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
// CHECK-NEXT:                                                Id 'f1' [D:E:%d.8 'f1']
// CHECK-NEXT:                                        ExpressionStatement
// CHECK-NEXT:                                            CallExpression
// CHECK-NEXT:                                                Id 'f1' [D:E:%d.8 'f1']
// CHECK-NEXT:                                Id 'f0' [D:E:%d.6 'f0']
// CHECK-NEXT:                        ExpressionStatement
// CHECK-NEXT:                            CallExpression
// CHECK-NEXT:                                Id 'f0' [D:E:%d.6 'f0']
