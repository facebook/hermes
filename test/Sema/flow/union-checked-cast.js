/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals -Werror -typed -dump-sema %s | %FileCheckOrRegen --match-full-lines %s

/// When flowing a type into a union, propagate needCheckedCast.
/// A function with fewer params flowing into a union with a function arm
/// that has more params should produce an ImplicitCheckedCast.

// Needs checked cast: fewer params flowing into union with more params.
type Fn = (x: number, y: number) => void;
function takeUnion(f: Fn | string): void {}
function foo(f: (x: number) => void): void {
  takeUnion(f);
}

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%union.2 = union(string | %function.6)
// CHECK-NEXT:%function.3 = function(f: %union.2): void
// CHECK-NEXT:%function.4 = function(x: number): void
// CHECK-NEXT:%function.5 = function(f: %function.4): void
// CHECK-NEXT:%function.6 = function(x: number, y: number): void

// CHECK:SemContext
// CHECK-NEXT:Func strict mayReachImplicitReturn
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'takeUnion' Var : %function.3
// CHECK-NEXT:        Decl %d.3 'foo' Var : %function.5
// CHECK-NEXT:        Decl %d.4 'arguments' Var Arguments
// CHECK-NEXT:        hoistedFunction takeUnion
// CHECK-NEXT:        hoistedFunction foo
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.5 'f' Parameter : %union.2
// CHECK-NEXT:            Decl %d.6 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.3
// CHECK-NEXT:            Decl %d.7 'f' Parameter : %function.4
// CHECK-NEXT:            Decl %d.8 'arguments' Var Arguments

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'Fn'
// CHECK-NEXT:            FunctionTypeAnnotation
// CHECK-NEXT:                FunctionTypeParam
// CHECK-NEXT:                    Id 'x'
// CHECK-NEXT:                    NumberTypeAnnotation
// CHECK-NEXT:                FunctionTypeParam
// CHECK-NEXT:                    Id 'y'
// CHECK-NEXT:                    NumberTypeAnnotation
// CHECK-NEXT:                VoidTypeAnnotation
// CHECK-NEXT:        FunctionDeclaration : %function.3
// CHECK-NEXT:            Id 'takeUnion' [D:E:%d.2 'takeUnion']
// CHECK-NEXT:            Id 'f' [D:E:%d.5 'f']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:        FunctionDeclaration : %function.5
// CHECK-NEXT:            Id 'foo' [D:E:%d.3 'foo']
// CHECK-NEXT:            Id 'f' [D:E:%d.7 'f']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ExpressionStatement
// CHECK-NEXT:                    CallExpression : void
// CHECK-NEXT:                        Id 'takeUnion' [D:E:%d.2 'takeUnion'] : %function.3
// CHECK-NEXT:                        ImplicitCheckedCast : %union.2
// CHECK-NEXT:                            Id 'f' [D:E:%d.7 'f'] : %function.4
