/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror --typed --dump-sema -fno-std-globals %s | %FileCheckOrRegen %s --match-full-lines

function main(x) {
  function f1(): ?number {
    if (x) return 1;
  }

  function f2(): number {
    if (x) return 1;
    else return 2;
  }

  function f3(): number {
    try {
      throw 1;
    } finally {
      return 2;
    }
  }

  function f4(): number {
    try {
      return 1;
    } finally {
      return 2;
    }
  }

  function f5(): number {
    try {
      return 1;
    } catch {
      return 2;
    }
  }

  function f6(): number {
    try {
      return 1;
    } finally {
    }
  }

  function f7(): number {
    try {
      try {
        return 1;
      } finally {}
    } catch {
      return 2;
    }
  }

  function f8(): number {
    label: {
      try {
        break label;
      } finally {
        return 1;
      }
    }
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%union.2 = union(void | null | number)
// CHECK-NEXT:%function.3 = function(): %union.2
// CHECK-NEXT:%function.4 = function(): number
// CHECK-NEXT:%object.5 = object({
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'main' Var : %untyped_function.1
// CHECK-NEXT:            Decl %d.3 'arguments' Var Arguments
// CHECK-NEXT:            hoistedFunction main
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:                Decl %d.4 'x' Parameter : any
// CHECK-NEXT:                Decl %d.5 'f1' Var : %function.3
// CHECK-NEXT:                Decl %d.6 'f2' Var : %function.4
// CHECK-NEXT:                Decl %d.7 'f3' Var : %function.4
// CHECK-NEXT:                Decl %d.8 'f4' Var : %function.4
// CHECK-NEXT:                Decl %d.9 'f5' Var : %function.4
// CHECK-NEXT:                Decl %d.10 'f6' Var : %function.4
// CHECK-NEXT:                Decl %d.11 'f7' Var : %function.4
// CHECK-NEXT:                Decl %d.12 'f8' Var : %function.4
// CHECK-NEXT:                Decl %d.13 'arguments' Var Arguments
// CHECK-NEXT:                hoistedFunction f1
// CHECK-NEXT:                hoistedFunction f2
// CHECK-NEXT:                hoistedFunction f3
// CHECK-NEXT:                hoistedFunction f4
// CHECK-NEXT:                hoistedFunction f5
// CHECK-NEXT:                hoistedFunction f6
// CHECK-NEXT:                hoistedFunction f7
// CHECK-NEXT:                hoistedFunction f8
// CHECK-NEXT:            Func strict
// CHECK-NEXT:                Scope %s.4
// CHECK-NEXT:                    Decl %d.14 'arguments' Var Arguments
// CHECK-NEXT:            Func strict
// CHECK-NEXT:                Scope %s.5
// CHECK-NEXT:                    Decl %d.15 'arguments' Var Arguments
// CHECK-NEXT:            Func strict
// CHECK-NEXT:                Scope %s.6
// CHECK-NEXT:                    Decl %d.16 'arguments' Var Arguments
// CHECK-NEXT:                    Scope %s.7
// CHECK-NEXT:                    Scope %s.8
// CHECK-NEXT:            Func strict
// CHECK-NEXT:                Scope %s.9
// CHECK-NEXT:                    Decl %d.17 'arguments' Var Arguments
// CHECK-NEXT:                    Scope %s.10
// CHECK-NEXT:                    Scope %s.11
// CHECK-NEXT:            Func strict
// CHECK-NEXT:                Scope %s.12
// CHECK-NEXT:                    Decl %d.18 'arguments' Var Arguments
// CHECK-NEXT:                    Scope %s.13
// CHECK-NEXT:                    Scope %s.14
// CHECK-NEXT:                        Scope %s.15
// CHECK-NEXT:            Func strict
// CHECK-NEXT:                Scope %s.16
// CHECK-NEXT:                    Decl %d.19 'arguments' Var Arguments
// CHECK-NEXT:                    Scope %s.17
// CHECK-NEXT:                    Scope %s.18
// CHECK-NEXT:            Func strict
// CHECK-NEXT:                Scope %s.19
// CHECK-NEXT:                    Decl %d.20 'arguments' Var Arguments
// CHECK-NEXT:                    Scope %s.20
// CHECK-NEXT:                        Scope %s.21
// CHECK-NEXT:                        Scope %s.22
// CHECK-NEXT:                    Scope %s.23
// CHECK-NEXT:                        Scope %s.24
// CHECK-NEXT:            Func strict
// CHECK-NEXT:                Scope %s.25
// CHECK-NEXT:                    Decl %d.21 'arguments' Var Arguments
// CHECK-NEXT:                    Scope %s.26
// CHECK-NEXT:                        Scope %s.27
// CHECK-NEXT:                        Scope %s.28

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    FunctionDeclaration : %untyped_function.1
// CHECK-NEXT:                        Id 'main' [D:E:%d.2 'main']
// CHECK-NEXT:                        Id 'x' [D:E:%d.4 'x']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            FunctionDeclaration : %function.3
// CHECK-NEXT:                                Id 'f1' [D:E:%d.5 'f1']
// CHECK-NEXT:                                BlockStatement
// CHECK-NEXT:                                    IfStatement
// CHECK-NEXT:                                        Id 'x' [D:E:%d.4 'x'] : any
// CHECK-NEXT:                                        ReturnStatement
// CHECK-NEXT:                                            NumericLiteral : number
// CHECK-NEXT:                            FunctionDeclaration : %function.4
// CHECK-NEXT:                                Id 'f2' [D:E:%d.6 'f2']
// CHECK-NEXT:                                BlockStatement
// CHECK-NEXT:                                    IfStatement
// CHECK-NEXT:                                        Id 'x' [D:E:%d.4 'x'] : any
// CHECK-NEXT:                                        ReturnStatement
// CHECK-NEXT:                                            NumericLiteral : number
// CHECK-NEXT:                                        ReturnStatement
// CHECK-NEXT:                                            NumericLiteral : number
// CHECK-NEXT:                            FunctionDeclaration : %function.4
// CHECK-NEXT:                                Id 'f3' [D:E:%d.7 'f3']
// CHECK-NEXT:                                BlockStatement
// CHECK-NEXT:                                    TryStatement
// CHECK-NEXT:                                        BlockStatement Scope %s.7
// CHECK-NEXT:                                            ThrowStatement
// CHECK-NEXT:                                                NumericLiteral
// CHECK-NEXT:                                        BlockStatement Scope %s.8
// CHECK-NEXT:                                            ReturnStatement
// CHECK-NEXT:                                                NumericLiteral : number
// CHECK-NEXT:                            FunctionDeclaration : %function.4
// CHECK-NEXT:                                Id 'f4' [D:E:%d.8 'f4']
// CHECK-NEXT:                                BlockStatement
// CHECK-NEXT:                                    TryStatement
// CHECK-NEXT:                                        BlockStatement Scope %s.10
// CHECK-NEXT:                                            ReturnStatement
// CHECK-NEXT:                                                NumericLiteral : number
// CHECK-NEXT:                                        BlockStatement Scope %s.11
// CHECK-NEXT:                                            ReturnStatement
// CHECK-NEXT:                                                NumericLiteral : number
// CHECK-NEXT:                            FunctionDeclaration : %function.4
// CHECK-NEXT:                                Id 'f5' [D:E:%d.9 'f5']
// CHECK-NEXT:                                BlockStatement
// CHECK-NEXT:                                    TryStatement
// CHECK-NEXT:                                        BlockStatement Scope %s.13
// CHECK-NEXT:                                            ReturnStatement
// CHECK-NEXT:                                                NumericLiteral : number
// CHECK-NEXT:                                        CatchClause Scope %s.14
// CHECK-NEXT:                                            BlockStatement Scope %s.15
// CHECK-NEXT:                                                ReturnStatement
// CHECK-NEXT:                                                    NumericLiteral : number
// CHECK-NEXT:                            FunctionDeclaration : %function.4
// CHECK-NEXT:                                Id 'f6' [D:E:%d.10 'f6']
// CHECK-NEXT:                                BlockStatement
// CHECK-NEXT:                                    TryStatement
// CHECK-NEXT:                                        BlockStatement Scope %s.17
// CHECK-NEXT:                                            ReturnStatement
// CHECK-NEXT:                                                NumericLiteral : number
// CHECK-NEXT:                                        BlockStatement Scope %s.18
// CHECK-NEXT:                            FunctionDeclaration : %function.4
// CHECK-NEXT:                                Id 'f7' [D:E:%d.11 'f7']
// CHECK-NEXT:                                BlockStatement
// CHECK-NEXT:                                    TryStatement
// CHECK-NEXT:                                        BlockStatement Scope %s.20
// CHECK-NEXT:                                            TryStatement
// CHECK-NEXT:                                                BlockStatement Scope %s.21
// CHECK-NEXT:                                                    ReturnStatement
// CHECK-NEXT:                                                        NumericLiteral : number
// CHECK-NEXT:                                                BlockStatement Scope %s.22
// CHECK-NEXT:                                        CatchClause Scope %s.23
// CHECK-NEXT:                                            BlockStatement Scope %s.24
// CHECK-NEXT:                                                ReturnStatement
// CHECK-NEXT:                                                    NumericLiteral : number
// CHECK-NEXT:                            FunctionDeclaration : %function.4
// CHECK-NEXT:                                Id 'f8' [D:E:%d.12 'f8']
// CHECK-NEXT:                                BlockStatement
// CHECK-NEXT:                                    LabeledStatement
// CHECK-NEXT:                                        Id 'label'
// CHECK-NEXT:                                        BlockStatement Scope %s.26
// CHECK-NEXT:                                            TryStatement
// CHECK-NEXT:                                                BlockStatement Scope %s.27
// CHECK-NEXT:                                                    BreakStatement
// CHECK-NEXT:                                                        Id 'label'
// CHECK-NEXT:                                                BlockStatement Scope %s.28
// CHECK-NEXT:                                                    ReturnStatement
// CHECK-NEXT:                                                        NumericLiteral : number
// CHECK-NEXT:            ObjectExpression : %object.5
