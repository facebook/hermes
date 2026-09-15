/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -typed -dump-sema -fno-std-globals %s | %FileCheckOrRegen --match-full-lines %s

class Foo {
  @Hermes.final @Hermes.overload
  bar(x: number): number { return x; }
  @Hermes.final @Hermes.overload
  bar(x: string): string { return x; }

  @Hermes.final @Hermes.overload
  multi(x: number): number { return x; }
  @Hermes.final @Hermes.overload
  multi(x: number, y: number): number { return x + y; }
  @Hermes.final @Hermes.overload
  multi(x: string): string { return x; }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%class_constructor.2 = class_constructor(%class.6)
// CHECK-NEXT:%function.3 = function(this: %class.6, x: number): number
// CHECK-NEXT:%function.4 = function(this: %class.6, x: string): string
// CHECK-NEXT:%function.5 = function(this: %class.6, x: number, y: number): number
// CHECK-NEXT:%class.6 = class(Foo {
// CHECK-NEXT:  %homeObject: %class.7
// CHECK-NEXT:})
// CHECK-NEXT:%class.7 = class( {
// CHECK-NEXT:  bar [final] [overloaded]: %function.3 %function.4
// CHECK-NEXT:  multi [final] [overloaded]: %function.3 %function.5 %function.4
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict mayReachImplicitReturn
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'Foo' Class : %class_constructor.2
// CHECK-NEXT:        Decl %d.3 'arguments' Var Arguments
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.4 'bar' Const
// CHECK-NEXT:            Decl %d.5 'bar' Const
// CHECK-NEXT:            Decl %d.6 'multi' Const
// CHECK-NEXT:            Decl %d.7 'multi' Const
// CHECK-NEXT:            Decl %d.8 'multi' Const
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.3
// CHECK-NEXT:            Decl %d.9 'x' Parameter : number
// CHECK-NEXT:            Decl %d.10 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.4
// CHECK-NEXT:            Decl %d.11 'x' Parameter : string
// CHECK-NEXT:            Decl %d.12 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.5
// CHECK-NEXT:            Decl %d.13 'x' Parameter : number
// CHECK-NEXT:            Decl %d.14 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.6
// CHECK-NEXT:            Decl %d.15 'x' Parameter : number
// CHECK-NEXT:            Decl %d.16 'y' Parameter : number
// CHECK-NEXT:            Decl %d.17 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.7
// CHECK-NEXT:            Decl %d.18 'x' Parameter : string
// CHECK-NEXT:            Decl %d.19 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.8

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        ClassDeclaration Scope %s.2
// CHECK-NEXT:            Id 'Foo' [D:E:%d.2 'Foo']
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:                MethodDefinition : %function.3
// CHECK-NEXT:                    Id 'bar' [D:E:%d.4 'bar']
// CHECK-NEXT:                    FunctionExpression : %function.3
// CHECK-NEXT:                        Id 'x' [D:E:%d.9 'x']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            ReturnStatement
// CHECK-NEXT:                                Id 'x' [D:E:%d.9 'x'] : number
// CHECK-NEXT:                    Decorator
// CHECK-NEXT:                        MemberExpression
// CHECK-NEXT:                            Id 'Hermes'
// CHECK-NEXT:                            Id 'final'
// CHECK-NEXT:                    Decorator
// CHECK-NEXT:                        MemberExpression
// CHECK-NEXT:                            Id 'Hermes'
// CHECK-NEXT:                            Id 'overload'
// CHECK-NEXT:                MethodDefinition : %function.4
// CHECK-NEXT:                    Id 'bar' [D:E:%d.5 'bar']
// CHECK-NEXT:                    FunctionExpression : %function.4
// CHECK-NEXT:                        Id 'x' [D:E:%d.11 'x']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            ReturnStatement
// CHECK-NEXT:                                Id 'x' [D:E:%d.11 'x'] : string
// CHECK-NEXT:                    Decorator
// CHECK-NEXT:                        MemberExpression
// CHECK-NEXT:                            Id 'Hermes'
// CHECK-NEXT:                            Id 'final'
// CHECK-NEXT:                    Decorator
// CHECK-NEXT:                        MemberExpression
// CHECK-NEXT:                            Id 'Hermes'
// CHECK-NEXT:                            Id 'overload'
// CHECK-NEXT:                MethodDefinition : %function.3
// CHECK-NEXT:                    Id 'multi' [D:E:%d.6 'multi']
// CHECK-NEXT:                    FunctionExpression : %function.3
// CHECK-NEXT:                        Id 'x' [D:E:%d.13 'x']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            ReturnStatement
// CHECK-NEXT:                                Id 'x' [D:E:%d.13 'x'] : number
// CHECK-NEXT:                    Decorator
// CHECK-NEXT:                        MemberExpression
// CHECK-NEXT:                            Id 'Hermes'
// CHECK-NEXT:                            Id 'final'
// CHECK-NEXT:                    Decorator
// CHECK-NEXT:                        MemberExpression
// CHECK-NEXT:                            Id 'Hermes'
// CHECK-NEXT:                            Id 'overload'
// CHECK-NEXT:                MethodDefinition : %function.5
// CHECK-NEXT:                    Id 'multi' [D:E:%d.7 'multi']
// CHECK-NEXT:                    FunctionExpression : %function.5
// CHECK-NEXT:                        Id 'x' [D:E:%d.15 'x']
// CHECK-NEXT:                        Id 'y' [D:E:%d.16 'y']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            ReturnStatement
// CHECK-NEXT:                                BinaryExpression : number
// CHECK-NEXT:                                    Id 'x' [D:E:%d.15 'x'] : number
// CHECK-NEXT:                                    BinOp +
// CHECK-NEXT:                                    Id 'y' [D:E:%d.16 'y'] : number
// CHECK-NEXT:                    Decorator
// CHECK-NEXT:                        MemberExpression
// CHECK-NEXT:                            Id 'Hermes'
// CHECK-NEXT:                            Id 'final'
// CHECK-NEXT:                    Decorator
// CHECK-NEXT:                        MemberExpression
// CHECK-NEXT:                            Id 'Hermes'
// CHECK-NEXT:                            Id 'overload'
// CHECK-NEXT:                MethodDefinition : %function.4
// CHECK-NEXT:                    Id 'multi' [D:E:%d.8 'multi']
// CHECK-NEXT:                    FunctionExpression : %function.4
// CHECK-NEXT:                        Id 'x' [D:E:%d.18 'x']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            ReturnStatement
// CHECK-NEXT:                                Id 'x' [D:E:%d.18 'x'] : string
// CHECK-NEXT:                    Decorator
// CHECK-NEXT:                        MemberExpression
// CHECK-NEXT:                            Id 'Hermes'
// CHECK-NEXT:                            Id 'final'
// CHECK-NEXT:                    Decorator
// CHECK-NEXT:                        MemberExpression
// CHECK-NEXT:                            Id 'Hermes'
// CHECK-NEXT:                            Id 'overload'
