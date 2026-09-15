/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals -typed -dump-sema %s | %FileCheckOrRegen %s --match-full-lines

// Static members on generic classes should see type parameters as empty,
// since it's not possible to write e.g. Foo<number>.bar().

class Foo<T> {
  x: T;
  instance_method(): T {
    return this.x;
  }
  static static_method(): void {
    let y: T;
  }
  static static_returns_t(): T {
    return (1: any);
  }
  static static_prop: T;
}

let f = new Foo<number>();
f.instance_method();

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%class.2 = class(Foo {
// CHECK-NEXT:  %homeObject: %class.7
// CHECK-NEXT:  %staticObject: %class.8
// CHECK-NEXT:  x: number
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.3 = class_constructor(%class.2)
// CHECK-NEXT:%function.4 = function(this: %class.2): number
// CHECK-NEXT:%function.5 = function(this: %class_constructor.3): void
// CHECK-NEXT:%function.6 = function(this: %class_constructor.3): empty
// CHECK-NEXT:%class.7 = class( {
// CHECK-NEXT:  instance_method [final]: %function.4
// CHECK-NEXT:})
// CHECK-NEXT:%class.8 = class( {
// CHECK-NEXT:  static_method [final]: %function.5
// CHECK-NEXT:  static_returns_t [final]: %function.6
// CHECK-NEXT:  static_prop: empty
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict mayReachImplicitReturn
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'Foo' Class
// CHECK-NEXT:        Decl %d.3 'f' Let : %class.2
// CHECK-NEXT:        Decl %d.4 'arguments' Var Arguments
// CHECK-NEXT:        Decl %d.5 'Foo' Class : %class_constructor.3
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:        Scope %s.3
// CHECK-NEXT:            Decl %d.6 'static_method' Const
// CHECK-NEXT:            Decl %d.7 'static_returns_t' Const
// CHECK-NEXT:            Decl %d.8 'static_prop' Const
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.4
// CHECK-NEXT:            Decl %d.9 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.5
// CHECK-NEXT:            Decl %d.10 'y' Let
// CHECK-NEXT:            Decl %d.11 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.6
// CHECK-NEXT:            Decl %d.12 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.7
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.8
// CHECK-NEXT:            Decl %d.13 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.9
// CHECK-NEXT:            Decl %d.14 'y' Let : empty
// CHECK-NEXT:            Decl %d.15 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.10
// CHECK-NEXT:            Decl %d.16 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.11

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        ClassDeclaration Scope %s.3
// CHECK-NEXT:            Id 'Foo' [D:E:%d.5 'Foo']
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:                ClassProperty : number
// CHECK-NEXT:                    Id 'x'
// CHECK-NEXT:                MethodDefinition : %function.4
// CHECK-NEXT:                    Id 'instance_method'
// CHECK-NEXT:                    FunctionExpression : %function.4
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            ReturnStatement
// CHECK-NEXT:                                MemberExpression : number
// CHECK-NEXT:                                    ThisExpression : %class.2
// CHECK-NEXT:                                    Id 'x'
// CHECK-NEXT:                MethodDefinition : %function.5
// CHECK-NEXT:                    Id 'static_method' [D:E:%d.6 'static_method']
// CHECK-NEXT:                    FunctionExpression : %function.5
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            VariableDeclaration
// CHECK-NEXT:                                VariableDeclarator
// CHECK-NEXT:                                    Id 'y' [D:E:%d.14 'y']
// CHECK-NEXT:                MethodDefinition : %function.6
// CHECK-NEXT:                    Id 'static_returns_t' [D:E:%d.7 'static_returns_t']
// CHECK-NEXT:                    FunctionExpression : %function.6
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            ReturnStatement
// CHECK-NEXT:                                ImplicitCheckedCast : empty
// CHECK-NEXT:                                    TypeCastExpression : any
// CHECK-NEXT:                                        NumericLiteral : number
// CHECK-NEXT:                ClassProperty : empty
// CHECK-NEXT:                    Id 'static_prop' [D:E:%d.8 'static_prop']
// CHECK-NEXT:        ClassDeclaration Scope %s.2
// CHECK-NEXT:            Id 'Foo' [D:E:%d.2 'Foo']
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:                ClassProperty
// CHECK-NEXT:                    Id 'x'
// CHECK-NEXT:                MethodDefinition
// CHECK-NEXT:                    Id 'instance_method'
// CHECK-NEXT:                    FunctionExpression
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            ReturnStatement
// CHECK-NEXT:                                MemberExpression
// CHECK-NEXT:                                    ThisExpression
// CHECK-NEXT:                                    Id 'x'
// CHECK-NEXT:                MethodDefinition
// CHECK-NEXT:                    Id 'static_method'
// CHECK-NEXT:                    FunctionExpression
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            VariableDeclaration
// CHECK-NEXT:                                VariableDeclarator
// CHECK-NEXT:                                    Id 'y' [D:E:%d.10 'y']
// CHECK-NEXT:                MethodDefinition
// CHECK-NEXT:                    Id 'static_returns_t'
// CHECK-NEXT:                    FunctionExpression
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            ReturnStatement
// CHECK-NEXT:                                TypeCastExpression
// CHECK-NEXT:                                    NumericLiteral
// CHECK-NEXT:                ClassProperty
// CHECK-NEXT:                    Id 'static_prop'
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                NewExpression : %class.2
// CHECK-NEXT:                    Id 'Foo' [D:E:%d.5 'Foo'] : %class_constructor.3
// CHECK-NEXT:                    TypeParameterInstantiation
// CHECK-NEXT:                        NumberTypeAnnotation
// CHECK-NEXT:                Id 'f' [D:E:%d.3 'f']
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            CallExpression : number
// CHECK-NEXT:                MemberExpression : %function.4
// CHECK-NEXT:                    Id 'f' [D:E:%d.3 'f'] : %class.2
// CHECK-NEXT:                    Id 'instance_method'
