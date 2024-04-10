/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -dump-sema -fno-std-globals %s | %FileCheckOrRegen %s --match-full-lines

function f(i) {
    var j = i + 1;
    // Classes within the scope of f, with fields with initializers
    // that reference variables in the scope of f.
    class A0 {
        x = i;
        constructor() {}
    }
    class A1 {
        x = j * 10;
        constructor() {}
    }
    var a0 = new A0();
    var a1 = new A1();
    return a0.x + a1.x;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:SemContext
// CHECK-NEXT:Func loose
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'f' GlobalProperty
// CHECK-NEXT:        hoistedFunction f
// CHECK-NEXT:    Func loose
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.2 'i' Parameter
// CHECK-NEXT:            Decl %d.3 'j' Var
// CHECK-NEXT:            Decl %d.4 'A0' Class
// CHECK-NEXT:            Decl %d.5 'A1' Class
// CHECK-NEXT:            Decl %d.6 'a0' Var
// CHECK-NEXT:            Decl %d.7 'a1' Var
// CHECK-NEXT:            Decl %d.8 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.4
// CHECK-NEXT:                Decl %d.9 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.5
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.6
// CHECK-NEXT:                Decl %d.10 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    FunctionDeclaration
// CHECK-NEXT:        Id 'f' [D:E:%d.1 'f']
// CHECK-NEXT:        Id 'i' [D:E:%d.2 'i']
// CHECK-NEXT:        BlockStatement
// CHECK-NEXT:            VariableDeclaration
// CHECK-NEXT:                VariableDeclarator
// CHECK-NEXT:                    BinaryExpression
// CHECK-NEXT:                        Id 'i' [D:E:%d.2 'i']
// CHECK-NEXT:                        BinOp +
// CHECK-NEXT:                        NumericLiteral
// CHECK-NEXT:                    Id 'j' [D:E:%d.3 'j']
// CHECK-NEXT:            ClassDeclaration
// CHECK-NEXT:                Id 'A0' [D:E:%d.4 'A0']
// CHECK-NEXT:                ClassBody
// CHECK-NEXT:                    ClassProperty
// CHECK-NEXT:                        Id 'x'
// CHECK-NEXT:                        Id 'i' [D:E:%d.2 'i']
// CHECK-NEXT:                    MethodDefinition
// CHECK-NEXT:                        Id 'constructor'
// CHECK-NEXT:                        FunctionExpression
// CHECK-NEXT:                            BlockStatement
// CHECK-NEXT:            ClassDeclaration
// CHECK-NEXT:                Id 'A1' [D:E:%d.5 'A1']
// CHECK-NEXT:                ClassBody
// CHECK-NEXT:                    ClassProperty
// CHECK-NEXT:                        Id 'x'
// CHECK-NEXT:                        BinaryExpression
// CHECK-NEXT:                            Id 'j' [D:E:%d.3 'j']
// CHECK-NEXT:                            NumericLiteral
// CHECK-NEXT:                    MethodDefinition
// CHECK-NEXT:                        Id 'constructor'
// CHECK-NEXT:                        FunctionExpression
// CHECK-NEXT:                            BlockStatement
// CHECK-NEXT:            VariableDeclaration
// CHECK-NEXT:                VariableDeclarator
// CHECK-NEXT:                    NewExpression
// CHECK-NEXT:                        Id 'A0' [D:E:%d.4 'A0']
// CHECK-NEXT:                    Id 'a0' [D:E:%d.6 'a0']
// CHECK-NEXT:            VariableDeclaration
// CHECK-NEXT:                VariableDeclarator
// CHECK-NEXT:                    NewExpression
// CHECK-NEXT:                        Id 'A1' [D:E:%d.5 'A1']
// CHECK-NEXT:                    Id 'a1' [D:E:%d.7 'a1']
// CHECK-NEXT:            ReturnStatement
// CHECK-NEXT:                BinaryExpression
// CHECK-NEXT:                    MemberExpression
// CHECK-NEXT:                        Id 'a0' [D:E:%d.6 'a0']
// CHECK-NEXT:                        Id 'x'
// CHECK-NEXT:                    BinOp +
// CHECK-NEXT:                    MemberExpression
// CHECK-NEXT:                        Id 'a1' [D:E:%d.7 'a1']
// CHECK-NEXT:                        Id 'x'
