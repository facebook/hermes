/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -fno-std-globals -typed -dump-sema %s | %FileCheckOrRegen %s --match-full-lines

// Only methods are readonly. A field that merely holds a function, and an
// accessor with a setter, both stay writable.

function mk(): number {
  return 1;
}

class A {
  field: () => number = mk;
  @Hermes.final get acc(): number {
    return 2;
  }
  @Hermes.final set acc(v: number) {}
}

function write(a: A) {
  a.field = mk;
  a.acc = 3;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%class.2 = class(A {
// CHECK-NEXT:  %homeObject: %class.8
// CHECK-NEXT:  field: %function.4
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.3 = class_constructor(%class.2)
// CHECK-NEXT:%function.4 = function(): number
// CHECK-NEXT:%function.5 = function(this: %class.2): number
// CHECK-NEXT:%function.6 = function(this: %class.2, v: number): any
// CHECK-NEXT:%function.7 = function(a: %class.2): any
// CHECK-NEXT:%class.8 = class( {
// CHECK-NEXT:  acc [final]: number
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict mayReachImplicitReturn
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'mk' Var : %function.4
// CHECK-NEXT:        Decl %d.3 'A' Class : %class_constructor.3
// CHECK-NEXT:        Decl %d.4 'write' Var : %function.7
// CHECK-NEXT:        Decl %d.5 'arguments' Var Arguments
// CHECK-NEXT:        hoistedFunction mk
// CHECK-NEXT:        hoistedFunction write
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.6 'acc' Const
// CHECK-NEXT:            Decl %d.7 'acc' Const
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.3
// CHECK-NEXT:            Decl %d.8 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.4
// CHECK-NEXT:            Decl %d.9 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.5
// CHECK-NEXT:            Decl %d.10 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.6
// CHECK-NEXT:            Decl %d.11 'v' Parameter : any
// CHECK-NEXT:            Decl %d.12 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.7
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.8
// CHECK-NEXT:            Decl %d.13 'a' Parameter : %class.2
// CHECK-NEXT:            Decl %d.14 'arguments' Var Arguments

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        FunctionDeclaration : %function.4
// CHECK-NEXT:            Id 'mk' [D:E:%d.2 'mk']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    NumericLiteral : 1
// CHECK-NEXT:        ClassDeclaration Scope %s.2
// CHECK-NEXT:            Id 'A' [D:E:%d.3 'A']
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:                ClassProperty : %function.4
// CHECK-NEXT:                    Id 'field'
// CHECK-NEXT:                    Id 'mk' [D:E:%d.2 'mk'] : %function.4
// CHECK-NEXT:                MethodDefinition : %function.5
// CHECK-NEXT:                    Id 'acc' [D:E:%d.6 'acc']
// CHECK-NEXT:                    FunctionExpression : %function.5
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            ReturnStatement
// CHECK-NEXT:                                NumericLiteral : 2
// CHECK-NEXT:                    Decorator
// CHECK-NEXT:                        MemberExpression
// CHECK-NEXT:                            Id 'Hermes'
// CHECK-NEXT:                            Id 'final'
// CHECK-NEXT:                MethodDefinition : %function.6
// CHECK-NEXT:                    Id 'acc' [D:E:%d.7 'acc']
// CHECK-NEXT:                    FunctionExpression : %function.6
// CHECK-NEXT:                        Id 'v' [D:E:%d.11 'v']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                    Decorator
// CHECK-NEXT:                        MemberExpression
// CHECK-NEXT:                            Id 'Hermes'
// CHECK-NEXT:                            Id 'final'
// CHECK-NEXT:        FunctionDeclaration : %function.7
// CHECK-NEXT:            Id 'write' [D:E:%d.4 'write']
// CHECK-NEXT:            Id 'a' [D:E:%d.13 'a']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    AssignmentExpression : %function.4
// CHECK-NEXT:                        MemberExpression : %function.4
// CHECK-NEXT:                            Id 'a' [D:E:%d.13 'a'] : %class.2
// CHECK-NEXT:                            Id 'field'
// CHECK-NEXT:                        Id 'mk' [D:E:%d.2 'mk'] : %function.4
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    AssignmentExpression : 3
// CHECK-NEXT:                        MemberExpression : number
// CHECK-NEXT:                            Id 'a' [D:E:%d.13 'a'] : %class.2
// CHECK-NEXT:                            Id 'acc'
// CHECK-NEXT:                        NumericLiteral : 3
