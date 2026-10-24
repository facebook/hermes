/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -typed -dump-sema %s | %FileCheckOrRegen --match-full-lines %s

'use strict';

// An untyped getter has no return type annotation, so the field type is 'any'.
class Foo {
  @Hermes.final
  get x() {}
}

let f = new Foo();
f.x;

// An untyped getter merged with a setter, in either order.
class Bar {
  @Hermes.final
  set x(v: any) {}
  @Hermes.final
  get x() {}
}

class Baz {
  @Hermes.final
  get x() {}
  @Hermes.final
  set x(v: any) {}
}

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%class.2 = class(Foo {
// CHECK-NEXT:  %homeObject: %class.8
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.3 = class_constructor(%class.2)
// CHECK-NEXT:%class_constructor.4 = class_constructor(%class.9)
// CHECK-NEXT:%class_constructor.5 = class_constructor(%class.10)
// CHECK-NEXT:%function.6 = function(this: %class.9, v: any): void
// CHECK-NEXT:%function.7 = function(this: %class.10, v: any): void
// CHECK-NEXT:%class.8 = class( {
// CHECK-NEXT:  x [final]: any
// CHECK-NEXT:})
// CHECK-NEXT:%class.9 = class(Bar {
// CHECK-NEXT:  %homeObject: %class.11
// CHECK-NEXT:})
// CHECK-NEXT:%class.10 = class(Baz {
// CHECK-NEXT:  %homeObject: %class.12
// CHECK-NEXT:})
// CHECK-NEXT:%class.11 = class( {
// CHECK-NEXT:  x [final]: any
// CHECK-NEXT:})
// CHECK-NEXT:%class.12 = class( {
// CHECK-NEXT:  x [final]: any
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict mayReachImplicitReturn
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'Foo' Class : %class_constructor.3
// CHECK-NEXT:        Decl %d.3 'f' Let : %class.2
// CHECK-NEXT:        Decl %d.4 'Bar' Class : %class_constructor.4
// CHECK-NEXT:        Decl %d.5 'Baz' Class : %class_constructor.5
// CHECK-NEXT:        Decl %d.6 'arguments' Var Arguments
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.7 'x' Const
// CHECK-NEXT:        Scope %s.3
// CHECK-NEXT:            Decl %d.8 'x' Const
// CHECK-NEXT:            Decl %d.9 'x' Const
// CHECK-NEXT:        Scope %s.4
// CHECK-NEXT:            Decl %d.10 'x' Const
// CHECK-NEXT:            Decl %d.11 'x' Const
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.5
// CHECK-NEXT:            Decl %d.12 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.6
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.7
// CHECK-NEXT:            Decl %d.13 'v' Parameter : any
// CHECK-NEXT:            Decl %d.14 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.8
// CHECK-NEXT:            Decl %d.15 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.9
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.10
// CHECK-NEXT:            Decl %d.16 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.11
// CHECK-NEXT:            Decl %d.17 'v' Parameter : any
// CHECK-NEXT:            Decl %d.18 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.12

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            StringLiteral : "use strict"
// CHECK-NEXT:        ClassDeclaration Scope %s.2
// CHECK-NEXT:            Id 'Foo' [D:E:%d.2 'Foo']
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:                MethodDefinition : %untyped_function.1
// CHECK-NEXT:                    Id 'x' [D:E:%d.7 'x']
// CHECK-NEXT:                    FunctionExpression : %untyped_function.1
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                    Decorator
// CHECK-NEXT:                        MemberExpression
// CHECK-NEXT:                            Id 'Hermes'
// CHECK-NEXT:                            Id 'final'
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                NewExpression : %class.2
// CHECK-NEXT:                    Id 'Foo' [D:E:%d.2 'Foo'] : %class_constructor.3
// CHECK-NEXT:                Id 'f' [D:E:%d.3 'f']
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            MemberExpression : any
// CHECK-NEXT:                Id 'f' [D:E:%d.3 'f'] : %class.2
// CHECK-NEXT:                Id 'x'
// CHECK-NEXT:        ClassDeclaration Scope %s.3
// CHECK-NEXT:            Id 'Bar' [D:E:%d.4 'Bar']
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:                MethodDefinition : %function.6
// CHECK-NEXT:                    Id 'x' [D:E:%d.8 'x']
// CHECK-NEXT:                    FunctionExpression : %function.6
// CHECK-NEXT:                        Id 'v' [D:E:%d.13 'v']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                    Decorator
// CHECK-NEXT:                        MemberExpression
// CHECK-NEXT:                            Id 'Hermes'
// CHECK-NEXT:                            Id 'final'
// CHECK-NEXT:                MethodDefinition : %untyped_function.1
// CHECK-NEXT:                    Id 'x' [D:E:%d.9 'x']
// CHECK-NEXT:                    FunctionExpression : %untyped_function.1
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                    Decorator
// CHECK-NEXT:                        MemberExpression
// CHECK-NEXT:                            Id 'Hermes'
// CHECK-NEXT:                            Id 'final'
// CHECK-NEXT:        ClassDeclaration Scope %s.4
// CHECK-NEXT:            Id 'Baz' [D:E:%d.5 'Baz']
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:                MethodDefinition : %untyped_function.1
// CHECK-NEXT:                    Id 'x' [D:E:%d.10 'x']
// CHECK-NEXT:                    FunctionExpression : %untyped_function.1
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                    Decorator
// CHECK-NEXT:                        MemberExpression
// CHECK-NEXT:                            Id 'Hermes'
// CHECK-NEXT:                            Id 'final'
// CHECK-NEXT:                MethodDefinition : %function.7
// CHECK-NEXT:                    Id 'x' [D:E:%d.11 'x']
// CHECK-NEXT:                    FunctionExpression : %function.7
// CHECK-NEXT:                        Id 'v' [D:E:%d.17 'v']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                    Decorator
// CHECK-NEXT:                        MemberExpression
// CHECK-NEXT:                            Id 'Hermes'
// CHECK-NEXT:                            Id 'final'
