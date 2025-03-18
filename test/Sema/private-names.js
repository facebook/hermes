/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -dump-sema -fno-std-globals %s | %FileCheckOrRegen %s --match-full-lines

class A {
    #f1;
    #m1() {}
    get #x() {}
    set #x(v) {}
    get #onlyGetter() {}
    set #onlySetter(v) {}
}

// Auto-generated content below. Please do not modify manually.

// CHECK:SemContext
// CHECK-NEXT:Func loose
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'A' Class
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.2 '#f1' PrivateField
// CHECK-NEXT:            Decl %d.3 '#m1' PrivateMethod
// CHECK-NEXT:            Decl %d.4 '#x' PrivateGetterSetter
// CHECK-NEXT:            Decl %d.5 '#onlyGetter' PrivateGetter
// CHECK-NEXT:            Decl %d.6 '#onlySetter' PrivateSetter
// CHECK-NEXT:            Decl %d.7 'A' ClassExprName
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.3
// CHECK-NEXT:            Decl %d.8 'arguments' Var Arguments
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.4
// CHECK-NEXT:            Decl %d.9 'arguments' Var Arguments
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.5
// CHECK-NEXT:            Decl %d.10 'v' Parameter
// CHECK-NEXT:            Decl %d.11 'arguments' Var Arguments
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.6
// CHECK-NEXT:            Decl %d.12 'arguments' Var Arguments
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.7
// CHECK-NEXT:            Decl %d.13 'v' Parameter
// CHECK-NEXT:            Decl %d.14 'arguments' Var Arguments
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.8

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ClassDeclaration Scope %s.2
// CHECK-NEXT:        Id 'A' [D:%d.1 E:%d.7 'A']
// CHECK-NEXT:        ClassBody
// CHECK-NEXT:            ClassPrivateProperty
// CHECK-NEXT:                Id 'f1' [D:E:%d.2 '#f1']
// CHECK-NEXT:            MethodDefinition
// CHECK-NEXT:                PrivateName
// CHECK-NEXT:                    Id 'm1' [D:E:%d.3 '#m1']
// CHECK-NEXT:                FunctionExpression
// CHECK-NEXT:                    BlockStatement
// CHECK-NEXT:            MethodDefinition
// CHECK-NEXT:                PrivateName
// CHECK-NEXT:                    Id 'x' [D:E:%d.4 '#x']
// CHECK-NEXT:                FunctionExpression
// CHECK-NEXT:                    BlockStatement
// CHECK-NEXT:            MethodDefinition
// CHECK-NEXT:                PrivateName
// CHECK-NEXT:                    Id 'x' [D:E:%d.4 '#x']
// CHECK-NEXT:                FunctionExpression
// CHECK-NEXT:                    Id 'v' [D:E:%d.10 'v']
// CHECK-NEXT:                    BlockStatement
// CHECK-NEXT:            MethodDefinition
// CHECK-NEXT:                PrivateName
// CHECK-NEXT:                    Id 'onlyGetter' [D:E:%d.5 '#onlyGetter']
// CHECK-NEXT:                FunctionExpression
// CHECK-NEXT:                    BlockStatement
// CHECK-NEXT:            MethodDefinition
// CHECK-NEXT:                PrivateName
// CHECK-NEXT:                    Id 'onlySetter' [D:E:%d.6 '#onlySetter']
// CHECK-NEXT:                FunctionExpression
// CHECK-NEXT:                    Id 'v' [D:E:%d.13 'v']
// CHECK-NEXT:                    BlockStatement
