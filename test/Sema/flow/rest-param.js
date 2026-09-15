/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -typed -dump-sema %s | %FileCheckOrRegen %s --match-full-lines

// Rest param in function declaration.
function foo(x: number, ...args: Array<string>): void {}

// Call with zero rest args.
foo(1);

// Call with one rest arg.
foo(1, "a");

// Call with multiple rest args.
foo(1, "a", "b", "c");

// Rest param as only param.
function bar(...items: Array<number>): void {}
bar();
bar(1, 2, 3);

// Rest param in function type annotation.
type F = (x: number, ...rest: Array<string>) => void;

// Flow: assign function with rest to matching type.
let f: F = foo;

// Type alias to Array used as rest param type.
type MyArr = Array<string>;
type G = (...rest: MyArr) => void;

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%function.2 = function(x: number, ...rest: %class.3): void
// CHECK-NEXT:%class.3 = class(Array<string>)
// CHECK-NEXT:%class.4 = class(Array<number>)
// CHECK-NEXT:%function.5 = function(...items: %class.4): void

// CHECK:SemContext
// CHECK-NEXT:Func strict mayReachImplicitReturn
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'foo' Var : %function.2
// CHECK-NEXT:        Decl %d.3 'bar' Var : %function.5
// CHECK-NEXT:        Decl %d.4 'f' Let : %function.2
// CHECK-NEXT:        Decl %d.5 'arguments' Var Arguments
// CHECK-NEXT:        hoistedFunction foo
// CHECK-NEXT:        hoistedFunction bar
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.6 'x' Parameter : number
// CHECK-NEXT:            Decl %d.7 'args' Parameter : %class.3
// CHECK-NEXT:            Decl %d.8 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.3
// CHECK-NEXT:            Decl %d.9 'items' Parameter : %class.4
// CHECK-NEXT:            Decl %d.10 'arguments' Var Arguments

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        FunctionDeclaration : %function.2
// CHECK-NEXT:            Id 'foo' [D:E:%d.2 'foo']
// CHECK-NEXT:            Id 'x' [D:E:%d.6 'x']
// CHECK-NEXT:            RestElement
// CHECK-NEXT:                Id 'args' [D:E:%d.7 'args']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            CallExpression : void
// CHECK-NEXT:                Id 'foo' [D:E:%d.2 'foo'] : %function.2
// CHECK-NEXT:                NumericLiteral : number
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            CallExpression : void
// CHECK-NEXT:                Id 'foo' [D:E:%d.2 'foo'] : %function.2
// CHECK-NEXT:                NumericLiteral : number
// CHECK-NEXT:                StringLiteral : string
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            CallExpression : void
// CHECK-NEXT:                Id 'foo' [D:E:%d.2 'foo'] : %function.2
// CHECK-NEXT:                NumericLiteral : number
// CHECK-NEXT:                StringLiteral : string
// CHECK-NEXT:                StringLiteral : string
// CHECK-NEXT:                StringLiteral : string
// CHECK-NEXT:        FunctionDeclaration : %function.5
// CHECK-NEXT:            Id 'bar' [D:E:%d.3 'bar']
// CHECK-NEXT:            RestElement
// CHECK-NEXT:                Id 'items' [D:E:%d.9 'items']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            CallExpression : void
// CHECK-NEXT:                Id 'bar' [D:E:%d.3 'bar'] : %function.5
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            CallExpression : void
// CHECK-NEXT:                Id 'bar' [D:E:%d.3 'bar'] : %function.5
// CHECK-NEXT:                NumericLiteral : number
// CHECK-NEXT:                NumericLiteral : number
// CHECK-NEXT:                NumericLiteral : number
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'F'
// CHECK-NEXT:            FunctionTypeAnnotation
// CHECK-NEXT:                FunctionTypeParam
// CHECK-NEXT:                    Id 'x'
// CHECK-NEXT:                    NumberTypeAnnotation
// CHECK-NEXT:                VoidTypeAnnotation
// CHECK-NEXT:                FunctionTypeParam
// CHECK-NEXT:                    Id 'rest'
// CHECK-NEXT:                    GenericTypeAnnotation
// CHECK-NEXT:                        Id 'Array'
// CHECK-NEXT:                        TypeParameterInstantiation
// CHECK-NEXT:                            StringTypeAnnotation
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                Id 'foo' [D:E:%d.2 'foo'] : %function.2
// CHECK-NEXT:                Id 'f' [D:E:%d.4 'f']
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'MyArr'
// CHECK-NEXT:            GenericTypeAnnotation
// CHECK-NEXT:                Id 'Array'
// CHECK-NEXT:                TypeParameterInstantiation
// CHECK-NEXT:                    StringTypeAnnotation
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'G'
// CHECK-NEXT:            FunctionTypeAnnotation
// CHECK-NEXT:                VoidTypeAnnotation
// CHECK-NEXT:                FunctionTypeParam
// CHECK-NEXT:                    Id 'rest'
// CHECK-NEXT:                    GenericTypeAnnotation
// CHECK-NEXT:                        Id 'MyArr'
