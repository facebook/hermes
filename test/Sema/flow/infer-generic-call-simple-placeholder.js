/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -dump-sema %s | %FileCheckOrRegen %s --match-full-lines

// Simple placeholder (bare T) should be resolved before complex constraints,
// so that complex constraints referencing T can use the resolved type.

// T inferred as string from 2nd arg, arrow param x gets type string.
function foo<T>(f: (x: T) => T, x: T): void {
  f(x);
}
foo(x => x, 'hello');

// T inferred as void when no argument is passed for optional param.
function bar<T>(x?: T): T {
  return x;
}
let v1: void = bar();

// T inferred from simple arg, used in complex constraint with one param.
function baz<T>(x: T, f: (a: T) => string): void {
  f(x);
}
baz('hello', a => '');

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%function.2 = function(x: string): string
// CHECK-NEXT:%function.3 = function(f: %function.2, x: string): void
// CHECK-NEXT:%function.4 = function(x: void): void
// CHECK-NEXT:%function.5 = function(x: string, f: %function.2): void

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'foo' Var
// CHECK-NEXT:        Decl %d.3 'bar' Var
// CHECK-NEXT:        Decl %d.4 'v1' Let : void
// CHECK-NEXT:        Decl %d.5 'baz' Var
// CHECK-NEXT:        Decl %d.6 'arguments' Var Arguments
// CHECK-NEXT:        Decl %d.7 'foo' Var : %function.3
// CHECK-NEXT:        Decl %d.8 'bar' Var : %function.4
// CHECK-NEXT:        Decl %d.9 'baz' Var : %function.5
// CHECK-NEXT:        hoistedFunction foo
// CHECK-NEXT:        hoistedFunction bar
// CHECK-NEXT:        hoistedFunction baz
// CHECK-NEXT:        hoistedFunction foo
// CHECK-NEXT:        hoistedFunction bar
// CHECK-NEXT:        hoistedFunction baz
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.10 'f' Parameter
// CHECK-NEXT:            Decl %d.11 'x' Parameter
// CHECK-NEXT:            Decl %d.12 'arguments' Var Arguments
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.3
// CHECK-NEXT:            Decl %d.13 'x' Parameter : string
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.4
// CHECK-NEXT:            Decl %d.14 'x' Parameter
// CHECK-NEXT:            Decl %d.15 'arguments' Var Arguments
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.5
// CHECK-NEXT:            Decl %d.16 'x' Parameter
// CHECK-NEXT:            Decl %d.17 'f' Parameter
// CHECK-NEXT:            Decl %d.18 'arguments' Var Arguments
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.6
// CHECK-NEXT:            Decl %d.19 'a' Parameter : string
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.7
// CHECK-NEXT:            Decl %d.20 'f' Parameter : %function.2
// CHECK-NEXT:            Decl %d.21 'x' Parameter : string
// CHECK-NEXT:            Decl %d.22 'arguments' Var Arguments
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.8
// CHECK-NEXT:            Decl %d.23 'x' Parameter : void
// CHECK-NEXT:            Decl %d.24 'arguments' Var Arguments
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.9
// CHECK-NEXT:            Decl %d.25 'x' Parameter : string
// CHECK-NEXT:            Decl %d.26 'f' Parameter : %function.2
// CHECK-NEXT:            Decl %d.27 'arguments' Var Arguments

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        FunctionDeclaration : %function.3
// CHECK-NEXT:            Id 'foo' [D:E:%d.7 'foo']
// CHECK-NEXT:            Id 'f' [D:E:%d.20 'f']
// CHECK-NEXT:            Id 'x' [D:E:%d.21 'x']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    CallExpression : string
// CHECK-NEXT:                        Id 'f' [D:E:%d.20 'f'] : %function.2
// CHECK-NEXT:                        Id 'x' [D:E:%d.21 'x'] : string
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:        FunctionDeclaration
// CHECK-NEXT:            Id 'foo' [D:E:%d.2 'foo']
// CHECK-NEXT:            Id 'f' [D:E:%d.10 'f']
// CHECK-NEXT:            Id 'x' [D:E:%d.11 'x']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    CallExpression
// CHECK-NEXT:                        Id 'f' [D:E:%d.10 'f']
// CHECK-NEXT:                        Id 'x' [D:E:%d.11 'x']
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            CallExpression : void
// CHECK-NEXT:                Id 'foo' [D:E:%d.7 'foo'] : %function.3
// CHECK-NEXT:                ArrowFunctionExpression : %function.2
// CHECK-NEXT:                    Id 'x' [D:E:%d.13 'x']
// CHECK-NEXT:                    BlockStatement
// CHECK-NEXT:                        ReturnStatement
// CHECK-NEXT:                            Id 'x' [D:E:%d.13 'x'] : string
// CHECK-NEXT:                StringLiteral : string
// CHECK-NEXT:        FunctionDeclaration : %function.4
// CHECK-NEXT:            Id 'bar' [D:E:%d.8 'bar']
// CHECK-NEXT:            Id 'x' [D:E:%d.23 'x']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    Id 'x' [D:E:%d.23 'x'] : void
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:        FunctionDeclaration
// CHECK-NEXT:            Id 'bar' [D:E:%d.3 'bar']
// CHECK-NEXT:            Id 'x' [D:E:%d.14 'x']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    Id 'x' [D:E:%d.14 'x']
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                CallExpression : void
// CHECK-NEXT:                    Id 'bar' [D:E:%d.8 'bar'] : %function.4
// CHECK-NEXT:                Id 'v1' [D:E:%d.4 'v1']
// CHECK-NEXT:        FunctionDeclaration : %function.5
// CHECK-NEXT:            Id 'baz' [D:E:%d.9 'baz']
// CHECK-NEXT:            Id 'x' [D:E:%d.25 'x']
// CHECK-NEXT:            Id 'f' [D:E:%d.26 'f']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    CallExpression : string
// CHECK-NEXT:                        Id 'f' [D:E:%d.26 'f'] : %function.2
// CHECK-NEXT:                        Id 'x' [D:E:%d.25 'x'] : string
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:        FunctionDeclaration
// CHECK-NEXT:            Id 'baz' [D:E:%d.5 'baz']
// CHECK-NEXT:            Id 'x' [D:E:%d.16 'x']
// CHECK-NEXT:            Id 'f' [D:E:%d.17 'f']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    CallExpression
// CHECK-NEXT:                        Id 'f' [D:E:%d.17 'f']
// CHECK-NEXT:                        Id 'x' [D:E:%d.16 'x']
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            CallExpression : void
// CHECK-NEXT:                Id 'baz' [D:E:%d.9 'baz'] : %function.5
// CHECK-NEXT:                StringLiteral : string
// CHECK-NEXT:                ArrowFunctionExpression : %function.2
// CHECK-NEXT:                    Id 'a' [D:E:%d.19 'a']
// CHECK-NEXT:                    BlockStatement
// CHECK-NEXT:                        ReturnStatement
// CHECK-NEXT:                            StringLiteral : string
