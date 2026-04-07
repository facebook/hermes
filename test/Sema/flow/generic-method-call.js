/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals -typed -dump-sema %s | %FileCheckOrRegen %s --match-full-lines

class Box {
  value: number;

  constructor() {
    this.value = 0;
  }

  @Hermes.final
  identity<T>(x: T): T {
    return x;
  }

  @Hermes.final
  pair<T, U>(x: T, y: U): T {
    return x;
  }
}

let box: Box = new Box();

// Test explicit type arguments.
let a: number = box.identity<number>(42);
let b: string = box.identity<string>("hello");

// Test type inference.
let c: number = box.identity(42);
let d: string = box.identity("hello");

// Test multiple type parameters with explicit args.
let e: number = box.pair<number, string>(10, "test");

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%class.2 = class(Box {
// CHECK-NEXT:  %constructor: %function.3
// CHECK-NEXT:  %homeObject: %class.4
// CHECK-NEXT:  value: number
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.5 = class_constructor(%class.2)
// CHECK-NEXT:%function.3 = function(this: %class.2): void
// CHECK-NEXT:%class.4 = class( {
// CHECK-NEXT:  identity [final]: generic
// CHECK-NEXT:  pair [final]: generic
// CHECK-NEXT:})
// CHECK-NEXT:%function.6 = function(this: %class.2, x: number): number
// CHECK-NEXT:%function.7 = function(this: %class.2, x: string): string
// CHECK-NEXT:%function.8 = function(this: %class.2, x: number, y: string): number
// CHECK-NEXT:%object.9 = object({
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'Box' Class : %class_constructor.5
// CHECK-NEXT:            Decl %d.3 'box' Let : %class.2
// CHECK-NEXT:            Decl %d.4 'a' Let : number
// CHECK-NEXT:            Decl %d.5 'b' Let : string
// CHECK-NEXT:            Decl %d.6 'c' Let : number
// CHECK-NEXT:            Decl %d.7 'd' Let : string
// CHECK-NEXT:            Decl %d.8 'e' Let : number
// CHECK-NEXT:            Decl %d.9 'arguments' Var Arguments
// CHECK-NEXT:            Decl %d.10 'identity' Const
// CHECK-NEXT:            Decl %d.11 'identity' Const
// CHECK-NEXT:            Decl %d.12 'pair' Const
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.4
// CHECK-NEXT:                Decl %d.13 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.5
// CHECK-NEXT:                Decl %d.14 'x' Parameter
// CHECK-NEXT:                Decl %d.15 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.6
// CHECK-NEXT:                Decl %d.16 'x' Parameter
// CHECK-NEXT:                Decl %d.17 'y' Parameter
// CHECK-NEXT:                Decl %d.18 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.7
// CHECK-NEXT:                Decl %d.19 'x' Parameter : number
// CHECK-NEXT:                Decl %d.20 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.8
// CHECK-NEXT:                Decl %d.21 'x' Parameter : string
// CHECK-NEXT:                Decl %d.22 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.9
// CHECK-NEXT:                Decl %d.23 'x' Parameter : number
// CHECK-NEXT:                Decl %d.24 'y' Parameter : string
// CHECK-NEXT:                Decl %d.25 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    ClassDeclaration Scope %s.3
// CHECK-NEXT:                        Id 'Box' [D:E:%d.2 'Box']
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            ClassProperty : number
// CHECK-NEXT:                                Id 'value'
// CHECK-NEXT:                            MethodDefinition : %function.3
// CHECK-NEXT:                                Id 'constructor'
// CHECK-NEXT:                                FunctionExpression : %function.3
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        ExpressionStatement
// CHECK-NEXT:                                            AssignmentExpression : number
// CHECK-NEXT:                                                MemberExpression : number
// CHECK-NEXT:                                                    ThisExpression : %class.2
// CHECK-NEXT:                                                    Id 'value'
// CHECK-NEXT:                                                NumericLiteral : number
// CHECK-NEXT:                            MethodDefinition : %function.6
// CHECK-NEXT:                                Id 'identity'
// CHECK-NEXT:                                FunctionExpression : %function.6
// CHECK-NEXT:                                    Id 'x' [D:E:%d.19 'x']
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        ReturnStatement
// CHECK-NEXT:                                            Id 'x' [D:E:%d.19 'x'] : number
// CHECK-NEXT:                                    TypeParameterDeclaration
// CHECK-NEXT:                                        TypeParameter
// CHECK-NEXT:                                Decorator
// CHECK-NEXT:                                    MemberExpression
// CHECK-NEXT:                                        Id 'Hermes'
// CHECK-NEXT:                                        Id 'final'
// CHECK-NEXT:                            MethodDefinition : %function.7
// CHECK-NEXT:                                Id 'identity'
// CHECK-NEXT:                                FunctionExpression : %function.7
// CHECK-NEXT:                                    Id 'x' [D:E:%d.21 'x']
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        ReturnStatement
// CHECK-NEXT:                                            Id 'x' [D:E:%d.21 'x'] : string
// CHECK-NEXT:                                    TypeParameterDeclaration
// CHECK-NEXT:                                        TypeParameter
// CHECK-NEXT:                                Decorator
// CHECK-NEXT:                                    MemberExpression
// CHECK-NEXT:                                        Id 'Hermes'
// CHECK-NEXT:                                        Id 'final'
// CHECK-NEXT:                            MethodDefinition : generic
// CHECK-NEXT:                                Id 'identity'
// CHECK-NEXT:                                FunctionExpression : generic
// CHECK-NEXT:                                    Id 'x' [D:E:%d.14 'x']
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        ReturnStatement
// CHECK-NEXT:                                            Id 'x' [D:E:%d.14 'x']
// CHECK-NEXT:                                    TypeParameterDeclaration
// CHECK-NEXT:                                        TypeParameter
// CHECK-NEXT:                                Decorator
// CHECK-NEXT:                                    MemberExpression
// CHECK-NEXT:                                        Id 'Hermes'
// CHECK-NEXT:                                        Id 'final'
// CHECK-NEXT:                            MethodDefinition : %function.8
// CHECK-NEXT:                                Id 'pair'
// CHECK-NEXT:                                FunctionExpression : %function.8
// CHECK-NEXT:                                    Id 'x' [D:E:%d.23 'x']
// CHECK-NEXT:                                    Id 'y' [D:E:%d.24 'y']
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        ReturnStatement
// CHECK-NEXT:                                            Id 'x' [D:E:%d.23 'x'] : number
// CHECK-NEXT:                                    TypeParameterDeclaration
// CHECK-NEXT:                                        TypeParameter
// CHECK-NEXT:                                        TypeParameter
// CHECK-NEXT:                                Decorator
// CHECK-NEXT:                                    MemberExpression
// CHECK-NEXT:                                        Id 'Hermes'
// CHECK-NEXT:                                        Id 'final'
// CHECK-NEXT:                            MethodDefinition : generic
// CHECK-NEXT:                                Id 'pair'
// CHECK-NEXT:                                FunctionExpression : generic
// CHECK-NEXT:                                    Id 'x' [D:E:%d.16 'x']
// CHECK-NEXT:                                    Id 'y' [D:E:%d.17 'y']
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        ReturnStatement
// CHECK-NEXT:                                            Id 'x' [D:E:%d.16 'x']
// CHECK-NEXT:                                    TypeParameterDeclaration
// CHECK-NEXT:                                        TypeParameter
// CHECK-NEXT:                                        TypeParameter
// CHECK-NEXT:                                Decorator
// CHECK-NEXT:                                    MemberExpression
// CHECK-NEXT:                                        Id 'Hermes'
// CHECK-NEXT:                                        Id 'final'
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            NewExpression : %class.2
// CHECK-NEXT:                                Id 'Box' [D:E:%d.2 'Box'] : %class_constructor.5
// CHECK-NEXT:                            Id 'box' [D:E:%d.3 'box']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            CallExpression : number
// CHECK-NEXT:                                MemberExpression : %function.6
// CHECK-NEXT:                                    Id 'box' [D:E:%d.3 'box'] : %class.2
// CHECK-NEXT:                                    Id 'identity' [D:E:%d.10 'identity']
// CHECK-NEXT:                                TypeParameterInstantiation
// CHECK-NEXT:                                    NumberTypeAnnotation
// CHECK-NEXT:                                NumericLiteral : number
// CHECK-NEXT:                            Id 'a' [D:E:%d.4 'a']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            CallExpression : string
// CHECK-NEXT:                                MemberExpression : %function.7
// CHECK-NEXT:                                    Id 'box' [D:E:%d.3 'box'] : %class.2
// CHECK-NEXT:                                    Id 'identity' [D:E:%d.11 'identity']
// CHECK-NEXT:                                TypeParameterInstantiation
// CHECK-NEXT:                                    StringTypeAnnotation
// CHECK-NEXT:                                StringLiteral : string
// CHECK-NEXT:                            Id 'b' [D:E:%d.5 'b']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            CallExpression : number
// CHECK-NEXT:                                MemberExpression : %function.6
// CHECK-NEXT:                                    Id 'box' [D:E:%d.3 'box'] : %class.2
// CHECK-NEXT:                                    Id 'identity' [D:E:%d.10 'identity']
// CHECK-NEXT:                                NumericLiteral : number
// CHECK-NEXT:                            Id 'c' [D:E:%d.6 'c']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            CallExpression : string
// CHECK-NEXT:                                MemberExpression : %function.7
// CHECK-NEXT:                                    Id 'box' [D:E:%d.3 'box'] : %class.2
// CHECK-NEXT:                                    Id 'identity' [D:E:%d.11 'identity']
// CHECK-NEXT:                                StringLiteral : string
// CHECK-NEXT:                            Id 'd' [D:E:%d.7 'd']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            CallExpression : number
// CHECK-NEXT:                                MemberExpression : %function.8
// CHECK-NEXT:                                    Id 'box' [D:E:%d.3 'box'] : %class.2
// CHECK-NEXT:                                    Id 'pair' [D:E:%d.12 'pair']
// CHECK-NEXT:                                TypeParameterInstantiation
// CHECK-NEXT:                                    NumberTypeAnnotation
// CHECK-NEXT:                                    StringTypeAnnotation
// CHECK-NEXT:                                NumericLiteral : number
// CHECK-NEXT:                                StringLiteral : string
// CHECK-NEXT:                            Id 'e' [D:E:%d.8 'e']
// CHECK-NEXT:            ObjectExpression : %object.9
