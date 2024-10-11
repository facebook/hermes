/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes %s -dump-sema -fno-std-globals | %FileCheckOrRegen --match-full-lines %s

var obj = {
  m1() {
    let x = super.x;
    return (() => { return super.x; });
  }
};

(function () {
  class A {
    foo() {
      let x = super.x;
      return (() => { return super.x; });
    }
    *foo() {
      let x = super.x;
      return (() => { return super.x; });
    }
    async foo() {
      let x = super.x;
      return (() => { return super.x; });
    }
    static foo() {
      let x = super.x;
      return (() => { return super.x; });
    }
    static *foo() {
      let x = super.x;
      return (() => { return super.x; });
    }
    static async foo() {
      let x = super.x;
      return (() => { return super.x; });
    }
  };
});

(function () {
  class A {
    static foo = super.x;
    static bar = (() => { return super.x; })();
    instFoo = super.x;
    instBar = (() => { return super.x; })();
  };
});

// Auto-generated content below. Please do not modify manually.

// CHECK:SemContext
// CHECK-NEXT:Func loose
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'obj' GlobalProperty
// CHECK-NEXT:    Func loose
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.2 'x' Let
// CHECK-NEXT:            Decl %d.3 'arguments' Var Arguments
// CHECK-NEXT:        Func loose
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:    Func loose
// CHECK-NEXT:        Scope %s.4
// CHECK-NEXT:            Decl %d.4 'A' Class
// CHECK-NEXT:            Decl %d.5 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.5
// CHECK-NEXT:                Decl %d.6 'x' Let
// CHECK-NEXT:                Decl %d.7 'arguments' Var Arguments
// CHECK-NEXT:            Func strict
// CHECK-NEXT:                Scope %s.6
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.7
// CHECK-NEXT:                Decl %d.8 'x' Let
// CHECK-NEXT:                Decl %d.9 'arguments' Var Arguments
// CHECK-NEXT:            Func strict
// CHECK-NEXT:                Scope %s.8
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.9
// CHECK-NEXT:                Decl %d.10 'x' Let
// CHECK-NEXT:                Decl %d.11 'arguments' Var Arguments
// CHECK-NEXT:            Func strict
// CHECK-NEXT:                Scope %s.10
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.11
// CHECK-NEXT:                Decl %d.12 'x' Let
// CHECK-NEXT:                Decl %d.13 'arguments' Var Arguments
// CHECK-NEXT:            Func strict
// CHECK-NEXT:                Scope %s.12
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.13
// CHECK-NEXT:                Decl %d.14 'x' Let
// CHECK-NEXT:                Decl %d.15 'arguments' Var Arguments
// CHECK-NEXT:            Func strict
// CHECK-NEXT:                Scope %s.14
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.15
// CHECK-NEXT:                Decl %d.16 'x' Let
// CHECK-NEXT:                Decl %d.17 'arguments' Var Arguments
// CHECK-NEXT:            Func strict
// CHECK-NEXT:                Scope %s.16
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.17
// CHECK-NEXT:    Func loose
// CHECK-NEXT:        Scope %s.18
// CHECK-NEXT:            Decl %d.18 'A' Class
// CHECK-NEXT:            Decl %d.19 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.19
// CHECK-NEXT:            Func strict
// CHECK-NEXT:                Scope %s.20
// CHECK-NEXT:            Func strict
// CHECK-NEXT:                Scope %s.21
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.22

// CHECK:Program Scope %s.1
// CHECK-NEXT:    VariableDeclaration
// CHECK-NEXT:        VariableDeclarator
// CHECK-NEXT:            ObjectExpression
// CHECK-NEXT:                Property
// CHECK-NEXT:                    Id 'm1'
// CHECK-NEXT:                    FunctionExpression
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            VariableDeclaration
// CHECK-NEXT:                                VariableDeclarator
// CHECK-NEXT:                                    MemberExpression
// CHECK-NEXT:                                        Super
// CHECK-NEXT:                                        Id 'x'
// CHECK-NEXT:                                    Id 'x' [D:E:%d.2 'x']
// CHECK-NEXT:                            ReturnStatement
// CHECK-NEXT:                                ArrowFunctionExpression
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        ReturnStatement
// CHECK-NEXT:                                            MemberExpression
// CHECK-NEXT:                                                Super
// CHECK-NEXT:                                                Id 'x'
// CHECK-NEXT:            Id 'obj' [D:E:%d.1 'obj']
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        FunctionExpression
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ClassDeclaration
// CHECK-NEXT:                    Id 'A' [D:E:%d.4 'A']
// CHECK-NEXT:                    ClassBody
// CHECK-NEXT:                        MethodDefinition
// CHECK-NEXT:                            Id 'foo'
// CHECK-NEXT:                            FunctionExpression
// CHECK-NEXT:                                BlockStatement
// CHECK-NEXT:                                    VariableDeclaration
// CHECK-NEXT:                                        VariableDeclarator
// CHECK-NEXT:                                            MemberExpression
// CHECK-NEXT:                                                Super
// CHECK-NEXT:                                                Id 'x'
// CHECK-NEXT:                                            Id 'x' [D:E:%d.6 'x']
// CHECK-NEXT:                                    ReturnStatement
// CHECK-NEXT:                                        ArrowFunctionExpression
// CHECK-NEXT:                                            BlockStatement
// CHECK-NEXT:                                                ReturnStatement
// CHECK-NEXT:                                                    MemberExpression
// CHECK-NEXT:                                                        Super
// CHECK-NEXT:                                                        Id 'x'
// CHECK-NEXT:                        MethodDefinition
// CHECK-NEXT:                            Id 'foo'
// CHECK-NEXT:                            FunctionExpression
// CHECK-NEXT:                                BlockStatement
// CHECK-NEXT:                                    VariableDeclaration
// CHECK-NEXT:                                        VariableDeclarator
// CHECK-NEXT:                                            MemberExpression
// CHECK-NEXT:                                                Super
// CHECK-NEXT:                                                Id 'x'
// CHECK-NEXT:                                            Id 'x' [D:E:%d.8 'x']
// CHECK-NEXT:                                    ReturnStatement
// CHECK-NEXT:                                        ArrowFunctionExpression
// CHECK-NEXT:                                            BlockStatement
// CHECK-NEXT:                                                ReturnStatement
// CHECK-NEXT:                                                    MemberExpression
// CHECK-NEXT:                                                        Super
// CHECK-NEXT:                                                        Id 'x'
// CHECK-NEXT:                        MethodDefinition
// CHECK-NEXT:                            Id 'foo'
// CHECK-NEXT:                            FunctionExpression
// CHECK-NEXT:                                BlockStatement
// CHECK-NEXT:                                    VariableDeclaration
// CHECK-NEXT:                                        VariableDeclarator
// CHECK-NEXT:                                            MemberExpression
// CHECK-NEXT:                                                Super
// CHECK-NEXT:                                                Id 'x'
// CHECK-NEXT:                                            Id 'x' [D:E:%d.10 'x']
// CHECK-NEXT:                                    ReturnStatement
// CHECK-NEXT:                                        ArrowFunctionExpression
// CHECK-NEXT:                                            BlockStatement
// CHECK-NEXT:                                                ReturnStatement
// CHECK-NEXT:                                                    MemberExpression
// CHECK-NEXT:                                                        Super
// CHECK-NEXT:                                                        Id 'x'
// CHECK-NEXT:                        MethodDefinition
// CHECK-NEXT:                            Id 'foo'
// CHECK-NEXT:                            FunctionExpression
// CHECK-NEXT:                                BlockStatement
// CHECK-NEXT:                                    VariableDeclaration
// CHECK-NEXT:                                        VariableDeclarator
// CHECK-NEXT:                                            MemberExpression
// CHECK-NEXT:                                                Super
// CHECK-NEXT:                                                Id 'x'
// CHECK-NEXT:                                            Id 'x' [D:E:%d.12 'x']
// CHECK-NEXT:                                    ReturnStatement
// CHECK-NEXT:                                        ArrowFunctionExpression
// CHECK-NEXT:                                            BlockStatement
// CHECK-NEXT:                                                ReturnStatement
// CHECK-NEXT:                                                    MemberExpression
// CHECK-NEXT:                                                        Super
// CHECK-NEXT:                                                        Id 'x'
// CHECK-NEXT:                        MethodDefinition
// CHECK-NEXT:                            Id 'foo'
// CHECK-NEXT:                            FunctionExpression
// CHECK-NEXT:                                BlockStatement
// CHECK-NEXT:                                    VariableDeclaration
// CHECK-NEXT:                                        VariableDeclarator
// CHECK-NEXT:                                            MemberExpression
// CHECK-NEXT:                                                Super
// CHECK-NEXT:                                                Id 'x'
// CHECK-NEXT:                                            Id 'x' [D:E:%d.14 'x']
// CHECK-NEXT:                                    ReturnStatement
// CHECK-NEXT:                                        ArrowFunctionExpression
// CHECK-NEXT:                                            BlockStatement
// CHECK-NEXT:                                                ReturnStatement
// CHECK-NEXT:                                                    MemberExpression
// CHECK-NEXT:                                                        Super
// CHECK-NEXT:                                                        Id 'x'
// CHECK-NEXT:                        MethodDefinition
// CHECK-NEXT:                            Id 'foo'
// CHECK-NEXT:                            FunctionExpression
// CHECK-NEXT:                                BlockStatement
// CHECK-NEXT:                                    VariableDeclaration
// CHECK-NEXT:                                        VariableDeclarator
// CHECK-NEXT:                                            MemberExpression
// CHECK-NEXT:                                                Super
// CHECK-NEXT:                                                Id 'x'
// CHECK-NEXT:                                            Id 'x' [D:E:%d.16 'x']
// CHECK-NEXT:                                    ReturnStatement
// CHECK-NEXT:                                        ArrowFunctionExpression
// CHECK-NEXT:                                            BlockStatement
// CHECK-NEXT:                                                ReturnStatement
// CHECK-NEXT:                                                    MemberExpression
// CHECK-NEXT:                                                        Super
// CHECK-NEXT:                                                        Id 'x'
// CHECK-NEXT:                EmptyStatement
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        FunctionExpression
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ClassDeclaration
// CHECK-NEXT:                    Id 'A' [D:E:%d.18 'A']
// CHECK-NEXT:                    ClassBody
// CHECK-NEXT:                        ClassProperty
// CHECK-NEXT:                            Id 'foo'
// CHECK-NEXT:                            MemberExpression
// CHECK-NEXT:                                Super
// CHECK-NEXT:                                Id 'x'
// CHECK-NEXT:                        ClassProperty
// CHECK-NEXT:                            Id 'bar'
// CHECK-NEXT:                            CallExpression
// CHECK-NEXT:                                ArrowFunctionExpression
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        ReturnStatement
// CHECK-NEXT:                                            MemberExpression
// CHECK-NEXT:                                                Super
// CHECK-NEXT:                                                Id 'x'
// CHECK-NEXT:                        ClassProperty
// CHECK-NEXT:                            Id 'instFoo'
// CHECK-NEXT:                            MemberExpression
// CHECK-NEXT:                                Super
// CHECK-NEXT:                                Id 'x'
// CHECK-NEXT:                        ClassProperty
// CHECK-NEXT:                            Id 'instBar'
// CHECK-NEXT:                            CallExpression
// CHECK-NEXT:                                ArrowFunctionExpression
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        ReturnStatement
// CHECK-NEXT:                                            MemberExpression
// CHECK-NEXT:                                                Super
// CHECK-NEXT:                                                Id 'x'
// CHECK-NEXT:                EmptyStatement
