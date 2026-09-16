/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals -typed -dump-sema %s | %FileCheckOrRegen %s --match-full-lines

// Partial type arguments: the leading ones are explicit, the trailing ones are
// inferred from the call arguments.

// Function: T explicit, U inferred.
function makePair<T, U>(a: T, b: U): [T, U] {
  return [a, b];
}
let p = makePair<number>(1, 'a');

// Three type params: T and U explicit, V inferred.
function pick<T, U, V>(a: T, b: U, c: V): U {
  return b;
}
let r = pick<number, string>(1, 'x', true);

// Trailing param inferred through a complex (tuple) constraint.
function firstOf<T, U>(a: T, b: [U, U]): T {
  return a;
}
let s = firstOf<number>(1, ['x', 'y']);

// Method: T explicit, U inferred.
class C {
  @Hermes.final
  foo<T, U>(a: T, b: U): U {
    return b;
  }
}
let c2: C = new C();
let x = c2.foo<number>(1, 'hi');

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%class.2 = class(C {
// CHECK-NEXT:  %homeObject: %class.10
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.3 = class_constructor(%class.2)
// CHECK-NEXT:%tuple.4 = tuple(number, string)
// CHECK-NEXT:%function.5 = function(a: number, b: string): %tuple.4
// CHECK-NEXT:%function.6 = function(a: number, b: string, c: boolean): string
// CHECK-NEXT:%tuple.7 = tuple(string, string)
// CHECK-NEXT:%function.8 = function(a: number, b: %tuple.7): number
// CHECK-NEXT:%function.9 = function(this: %class.2, a: number, b: string): string
// CHECK-NEXT:%class.10 = class( {
// CHECK-NEXT:  foo [final]: generic
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict mayReachImplicitReturn
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'makePair' Var
// CHECK-NEXT:        Decl %d.3 'p' Let : %tuple.4
// CHECK-NEXT:        Decl %d.4 'pick' Var
// CHECK-NEXT:        Decl %d.5 'r' Let : string
// CHECK-NEXT:        Decl %d.6 'firstOf' Var
// CHECK-NEXT:        Decl %d.7 's' Let : number
// CHECK-NEXT:        Decl %d.8 'C' Class : %class_constructor.3
// CHECK-NEXT:        Decl %d.9 'c2' Let : %class.2
// CHECK-NEXT:        Decl %d.10 'x' Let : string
// CHECK-NEXT:        Decl %d.11 'arguments' Var Arguments
// CHECK-NEXT:        Decl %d.12 'makePair' Var : %function.5
// CHECK-NEXT:        Decl %d.13 'pick' Var : %function.6
// CHECK-NEXT:        Decl %d.14 'firstOf' Var : %function.8
// CHECK-NEXT:        hoistedFunction makePair
// CHECK-NEXT:        hoistedFunction pick
// CHECK-NEXT:        hoistedFunction firstOf
// CHECK-NEXT:        hoistedFunction makePair
// CHECK-NEXT:        hoistedFunction pick
// CHECK-NEXT:        hoistedFunction firstOf
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.15 'foo' Const
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.3
// CHECK-NEXT:            Decl %d.16 'a' Parameter
// CHECK-NEXT:            Decl %d.17 'b' Parameter
// CHECK-NEXT:            Decl %d.18 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.4
// CHECK-NEXT:            Decl %d.19 'a' Parameter
// CHECK-NEXT:            Decl %d.20 'b' Parameter
// CHECK-NEXT:            Decl %d.21 'c' Parameter
// CHECK-NEXT:            Decl %d.22 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.5
// CHECK-NEXT:            Decl %d.23 'a' Parameter
// CHECK-NEXT:            Decl %d.24 'b' Parameter
// CHECK-NEXT:            Decl %d.25 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.6
// CHECK-NEXT:            Decl %d.26 'a' Parameter
// CHECK-NEXT:            Decl %d.27 'b' Parameter
// CHECK-NEXT:            Decl %d.28 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.7
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.8
// CHECK-NEXT:            Decl %d.29 'a' Parameter : number
// CHECK-NEXT:            Decl %d.30 'b' Parameter : string
// CHECK-NEXT:            Decl %d.31 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.9
// CHECK-NEXT:            Decl %d.32 'a' Parameter : number
// CHECK-NEXT:            Decl %d.33 'b' Parameter : string
// CHECK-NEXT:            Decl %d.34 'c' Parameter : boolean
// CHECK-NEXT:            Decl %d.35 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.10
// CHECK-NEXT:            Decl %d.36 'a' Parameter : number
// CHECK-NEXT:            Decl %d.37 'b' Parameter : %tuple.7
// CHECK-NEXT:            Decl %d.38 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.11
// CHECK-NEXT:            Decl %d.39 'a' Parameter : number
// CHECK-NEXT:            Decl %d.40 'b' Parameter : string
// CHECK-NEXT:            Decl %d.41 'arguments' Var Arguments

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        FunctionDeclaration : %function.5
// CHECK-NEXT:            Id 'makePair' [D:E:%d.12 'makePair']
// CHECK-NEXT:            Id 'a' [D:E:%d.29 'a']
// CHECK-NEXT:            Id 'b' [D:E:%d.30 'b']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    ArrayExpression : %tuple.4
// CHECK-NEXT:                        Id 'a' [D:E:%d.29 'a'] : number
// CHECK-NEXT:                        Id 'b' [D:E:%d.30 'b'] : string
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:        FunctionDeclaration
// CHECK-NEXT:            Id 'makePair' [D:E:%d.2 'makePair']
// CHECK-NEXT:            Id 'a' [D:E:%d.16 'a']
// CHECK-NEXT:            Id 'b' [D:E:%d.17 'b']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    ArrayExpression
// CHECK-NEXT:                        Id 'a' [D:E:%d.16 'a']
// CHECK-NEXT:                        Id 'b' [D:E:%d.17 'b']
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                CallExpression : %tuple.4
// CHECK-NEXT:                    Id 'makePair' [D:E:%d.12 'makePair'] : %function.5
// CHECK-NEXT:                    TypeParameterInstantiation
// CHECK-NEXT:                        NumberTypeAnnotation
// CHECK-NEXT:                    NumericLiteral : 1
// CHECK-NEXT:                    StringLiteral : "a"
// CHECK-NEXT:                Id 'p' [D:E:%d.3 'p']
// CHECK-NEXT:        FunctionDeclaration : %function.6
// CHECK-NEXT:            Id 'pick' [D:E:%d.13 'pick']
// CHECK-NEXT:            Id 'a' [D:E:%d.32 'a']
// CHECK-NEXT:            Id 'b' [D:E:%d.33 'b']
// CHECK-NEXT:            Id 'c' [D:E:%d.34 'c']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    Id 'b' [D:E:%d.33 'b'] : string
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:        FunctionDeclaration
// CHECK-NEXT:            Id 'pick' [D:E:%d.4 'pick']
// CHECK-NEXT:            Id 'a' [D:E:%d.19 'a']
// CHECK-NEXT:            Id 'b' [D:E:%d.20 'b']
// CHECK-NEXT:            Id 'c' [D:E:%d.21 'c']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    Id 'b' [D:E:%d.20 'b']
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                CallExpression : string
// CHECK-NEXT:                    Id 'pick' [D:E:%d.13 'pick'] : %function.6
// CHECK-NEXT:                    TypeParameterInstantiation
// CHECK-NEXT:                        NumberTypeAnnotation
// CHECK-NEXT:                        StringTypeAnnotation
// CHECK-NEXT:                    NumericLiteral : 1
// CHECK-NEXT:                    StringLiteral : "x"
// CHECK-NEXT:                    BooleanLiteral : true
// CHECK-NEXT:                Id 'r' [D:E:%d.5 'r']
// CHECK-NEXT:        FunctionDeclaration : %function.8
// CHECK-NEXT:            Id 'firstOf' [D:E:%d.14 'firstOf']
// CHECK-NEXT:            Id 'a' [D:E:%d.36 'a']
// CHECK-NEXT:            Id 'b' [D:E:%d.37 'b']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    Id 'a' [D:E:%d.36 'a'] : number
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:        FunctionDeclaration
// CHECK-NEXT:            Id 'firstOf' [D:E:%d.6 'firstOf']
// CHECK-NEXT:            Id 'a' [D:E:%d.23 'a']
// CHECK-NEXT:            Id 'b' [D:E:%d.24 'b']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    Id 'a' [D:E:%d.23 'a']
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                CallExpression : number
// CHECK-NEXT:                    Id 'firstOf' [D:E:%d.14 'firstOf'] : %function.8
// CHECK-NEXT:                    TypeParameterInstantiation
// CHECK-NEXT:                        NumberTypeAnnotation
// CHECK-NEXT:                    NumericLiteral : 1
// CHECK-NEXT:                    ArrayExpression : %tuple.7
// CHECK-NEXT:                        StringLiteral : "x"
// CHECK-NEXT:                        StringLiteral : "y"
// CHECK-NEXT:                Id 's' [D:E:%d.7 's']
// CHECK-NEXT:        ClassDeclaration Scope %s.2
// CHECK-NEXT:            Id 'C' [D:E:%d.8 'C']
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:                MethodDefinition : %function.9
// CHECK-NEXT:                    Id 'foo'
// CHECK-NEXT:                    FunctionExpression : %function.9
// CHECK-NEXT:                        Id 'a' [D:E:%d.39 'a']
// CHECK-NEXT:                        Id 'b' [D:E:%d.40 'b']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            ReturnStatement
// CHECK-NEXT:                                Id 'b' [D:E:%d.40 'b'] : string
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                    Decorator
// CHECK-NEXT:                        MemberExpression
// CHECK-NEXT:                            Id 'Hermes'
// CHECK-NEXT:                            Id 'final'
// CHECK-NEXT:                MethodDefinition : generic
// CHECK-NEXT:                    Id 'foo'
// CHECK-NEXT:                    FunctionExpression : generic
// CHECK-NEXT:                        Id 'a' [D:E:%d.26 'a']
// CHECK-NEXT:                        Id 'b' [D:E:%d.27 'b']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            ReturnStatement
// CHECK-NEXT:                                Id 'b' [D:E:%d.27 'b']
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                    Decorator
// CHECK-NEXT:                        MemberExpression
// CHECK-NEXT:                            Id 'Hermes'
// CHECK-NEXT:                            Id 'final'
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                NewExpression : %class.2
// CHECK-NEXT:                    Id 'C' [D:E:%d.8 'C'] : %class_constructor.3
// CHECK-NEXT:                Id 'c2' [D:E:%d.9 'c2']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                CallExpression : string
// CHECK-NEXT:                    MemberExpression : %function.9
// CHECK-NEXT:                        Id 'c2' [D:E:%d.9 'c2'] : %class.2
// CHECK-NEXT:                        Id 'foo' [D:E:%d.15 'foo']
// CHECK-NEXT:                    TypeParameterInstantiation
// CHECK-NEXT:                        NumberTypeAnnotation
// CHECK-NEXT:                    NumericLiteral : 1
// CHECK-NEXT:                    StringLiteral : "hi"
// CHECK-NEXT:                Id 'x' [D:E:%d.10 'x']
