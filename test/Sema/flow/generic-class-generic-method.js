/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals -typed -dump-sema %s | %FileCheckOrRegen %s --match-full-lines

class A<T> {
  @Hermes.final
  foo<U>(x: T, y: U): [T, U, number] {
    var z: number = 123;
    return [x, y, z];
  }
}

var a: A<number> = new A<number>();
var xyz: [number, string, number] = a.foo<string>(123, 'str');

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%class.2 = class(A {
// CHECK-NEXT:  %homeObject: %class.6
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.3 = class_constructor(%class.2)
// CHECK-NEXT:%tuple.4 = tuple(number, string, number)
// CHECK-NEXT:%function.5 = function(this: %class.2, x: number, y: string): %tuple.4
// CHECK-NEXT:%class.6 = class( {
// CHECK-NEXT:  foo [final]: generic
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'A' Class
// CHECK-NEXT:        Decl %d.3 'a' Var : %class.2
// CHECK-NEXT:        Decl %d.4 'xyz' Var : %tuple.4
// CHECK-NEXT:        Decl %d.5 'arguments' Var Arguments
// CHECK-NEXT:        Decl %d.6 'A' Class : %class_constructor.3
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:        Scope %s.3
// CHECK-NEXT:            Decl %d.7 'foo' Const
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.4
// CHECK-NEXT:            Decl %d.8 'x' Parameter
// CHECK-NEXT:            Decl %d.9 'y' Parameter
// CHECK-NEXT:            Decl %d.10 'z' Var
// CHECK-NEXT:            Decl %d.11 'arguments' Var Arguments
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.5
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.6
// CHECK-NEXT:            Decl %d.12 'x' Parameter
// CHECK-NEXT:            Decl %d.13 'y' Parameter
// CHECK-NEXT:            Decl %d.14 'z' Var
// CHECK-NEXT:            Decl %d.15 'arguments' Var Arguments
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.7
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.8
// CHECK-NEXT:            Decl %d.16 'x' Parameter : number
// CHECK-NEXT:            Decl %d.17 'y' Parameter : string
// CHECK-NEXT:            Decl %d.18 'z' Var : number
// CHECK-NEXT:            Decl %d.19 'arguments' Var Arguments

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        ClassDeclaration Scope %s.3
// CHECK-NEXT:            Id 'A' [D:E:%d.6 'A']
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:                MethodDefinition : %function.5
// CHECK-NEXT:                    Id 'foo'
// CHECK-NEXT:                    FunctionExpression : %function.5
// CHECK-NEXT:                        Id 'x' [D:E:%d.16 'x']
// CHECK-NEXT:                        Id 'y' [D:E:%d.17 'y']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            VariableDeclaration
// CHECK-NEXT:                                VariableDeclarator
// CHECK-NEXT:                                    NumericLiteral : number
// CHECK-NEXT:                                    Id 'z' [D:E:%d.18 'z']
// CHECK-NEXT:                            ReturnStatement
// CHECK-NEXT:                                ArrayExpression : %tuple.4
// CHECK-NEXT:                                    Id 'x' [D:E:%d.16 'x'] : number
// CHECK-NEXT:                                    Id 'y' [D:E:%d.17 'y'] : string
// CHECK-NEXT:                                    Id 'z' [D:E:%d.18 'z'] : number
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                    Decorator
// CHECK-NEXT:                        MemberExpression
// CHECK-NEXT:                            Id 'Hermes'
// CHECK-NEXT:                            Id 'final'
// CHECK-NEXT:                MethodDefinition : generic
// CHECK-NEXT:                    Id 'foo'
// CHECK-NEXT:                    FunctionExpression : generic
// CHECK-NEXT:                        Id 'x' [D:E:%d.12 'x']
// CHECK-NEXT:                        Id 'y' [D:E:%d.13 'y']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            VariableDeclaration
// CHECK-NEXT:                                VariableDeclarator
// CHECK-NEXT:                                    NumericLiteral
// CHECK-NEXT:                                    Id 'z' [D:E:%d.14 'z']
// CHECK-NEXT:                            ReturnStatement
// CHECK-NEXT:                                ArrayExpression
// CHECK-NEXT:                                    Id 'x' [D:E:%d.12 'x']
// CHECK-NEXT:                                    Id 'y' [D:E:%d.13 'y']
// CHECK-NEXT:                                    Id 'z' [D:E:%d.14 'z']
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                    Decorator
// CHECK-NEXT:                        MemberExpression
// CHECK-NEXT:                            Id 'Hermes'
// CHECK-NEXT:                            Id 'final'
// CHECK-NEXT:        ClassDeclaration Scope %s.2
// CHECK-NEXT:            Id 'A' [D:E:%d.2 'A']
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:                MethodDefinition
// CHECK-NEXT:                    Id 'foo'
// CHECK-NEXT:                    FunctionExpression
// CHECK-NEXT:                        Id 'x' [D:E:%d.8 'x']
// CHECK-NEXT:                        Id 'y' [D:E:%d.9 'y']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            VariableDeclaration
// CHECK-NEXT:                                VariableDeclarator
// CHECK-NEXT:                                    NumericLiteral
// CHECK-NEXT:                                    Id 'z' [D:E:%d.10 'z']
// CHECK-NEXT:                            ReturnStatement
// CHECK-NEXT:                                ArrayExpression
// CHECK-NEXT:                                    Id 'x' [D:E:%d.8 'x']
// CHECK-NEXT:                                    Id 'y' [D:E:%d.9 'y']
// CHECK-NEXT:                                    Id 'z' [D:E:%d.10 'z']
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                    Decorator
// CHECK-NEXT:                        MemberExpression
// CHECK-NEXT:                            Id 'Hermes'
// CHECK-NEXT:                            Id 'final'
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                NewExpression : %class.2
// CHECK-NEXT:                    Id 'A' [D:E:%d.6 'A'] : %class_constructor.3
// CHECK-NEXT:                    TypeParameterInstantiation
// CHECK-NEXT:                        NumberTypeAnnotation
// CHECK-NEXT:                Id 'a' [D:E:%d.3 'a']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                CallExpression : %tuple.4
// CHECK-NEXT:                    MemberExpression : %function.5
// CHECK-NEXT:                        Id 'a' [D:E:%d.3 'a'] : %class.2
// CHECK-NEXT:                        Id 'foo' [D:E:%d.7 'foo']
// CHECK-NEXT:                    TypeParameterInstantiation
// CHECK-NEXT:                        StringTypeAnnotation
// CHECK-NEXT:                    NumericLiteral : number
// CHECK-NEXT:                    StringLiteral : string
// CHECK-NEXT:                Id 'xyz' [D:E:%d.4 'xyz']
