/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -dump-sema %s | %FileCheckOrRegen %s --match-full-lines

// Test that fn.call(thisArg, args...) is typechecked like $SHBuiltin.call.

class C {
  x: number = 0;
}

function withThis(this: C, n: number): number {
  return this.x + n;
}

function noThis(a: number, b: string): boolean {
  return b.length > a;
}

function test(): void {
  let c = new C();
  let r1: number = withThis.call(c, 5);
  let r2: boolean = noThis.call(undefined, 1, "hi");
}

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%class.2 = class(C {
// CHECK-NEXT:  %homeObject: %class.7
// CHECK-NEXT:  x: number
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.3 = class_constructor(%class.2)
// CHECK-NEXT:%function.4 = function(this: %class.2, n: number): number
// CHECK-NEXT:%function.5 = function(a: number, b: string): boolean
// CHECK-NEXT:%function.6 = function(): void
// CHECK-NEXT:%class.7 = class( {
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict mayReachImplicitReturn
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'C' Class : %class_constructor.3
// CHECK-NEXT:        Decl %d.3 'withThis' Var : %function.4
// CHECK-NEXT:        Decl %d.4 'noThis' Var : %function.5
// CHECK-NEXT:        Decl %d.5 'test' Var : %function.6
// CHECK-NEXT:        Decl %d.6 'arguments' Var Arguments
// CHECK-NEXT:        hoistedFunction withThis
// CHECK-NEXT:        hoistedFunction noThis
// CHECK-NEXT:        hoistedFunction test
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.3
// CHECK-NEXT:            Decl %d.7 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.4
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.5
// CHECK-NEXT:            Decl %d.8 'this' Parameter : %class.2
// CHECK-NEXT:            Decl %d.9 'n' Parameter : number
// CHECK-NEXT:            Decl %d.10 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.6
// CHECK-NEXT:            Decl %d.11 'a' Parameter : number
// CHECK-NEXT:            Decl %d.12 'b' Parameter : string
// CHECK-NEXT:            Decl %d.13 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.7
// CHECK-NEXT:            Decl %d.14 'c' Let : %class.2
// CHECK-NEXT:            Decl %d.15 'r1' Let : number
// CHECK-NEXT:            Decl %d.16 'r2' Let : boolean
// CHECK-NEXT:            Decl %d.17 'arguments' Var Arguments

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        ClassDeclaration Scope %s.2
// CHECK-NEXT:            Id 'C' [D:E:%d.2 'C']
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:                ClassProperty : number
// CHECK-NEXT:                    Id 'x'
// CHECK-NEXT:                    NumericLiteral : number
// CHECK-NEXT:        FunctionDeclaration : %function.4
// CHECK-NEXT:            Id 'withThis' [D:E:%d.3 'withThis']
// CHECK-NEXT:            Id 'this' [D:E:%d.8 'this']
// CHECK-NEXT:            Id 'n' [D:E:%d.9 'n']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    BinaryExpression : number
// CHECK-NEXT:                        MemberExpression : number
// CHECK-NEXT:                            ThisExpression : %class.2
// CHECK-NEXT:                            Id 'x'
// CHECK-NEXT:                        BinOp +
// CHECK-NEXT:                        Id 'n' [D:E:%d.9 'n'] : number
// CHECK-NEXT:        FunctionDeclaration : %function.5
// CHECK-NEXT:            Id 'noThis' [D:E:%d.4 'noThis']
// CHECK-NEXT:            Id 'a' [D:E:%d.11 'a']
// CHECK-NEXT:            Id 'b' [D:E:%d.12 'b']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    BinaryExpression : boolean
// CHECK-NEXT:                        MemberExpression : number
// CHECK-NEXT:                            Id 'b' [D:E:%d.12 'b'] : string
// CHECK-NEXT:                            Id 'length'
// CHECK-NEXT:                        Id 'a' [D:E:%d.11 'a'] : number
// CHECK-NEXT:        FunctionDeclaration : %function.6
// CHECK-NEXT:            Id 'test' [D:E:%d.5 'test']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                VariableDeclaration
// CHECK-NEXT:                    VariableDeclarator
// CHECK-NEXT:                        NewExpression : %class.2
// CHECK-NEXT:                            Id 'C' [D:E:%d.2 'C'] : %class_constructor.3
// CHECK-NEXT:                        Id 'c' [D:E:%d.14 'c']
// CHECK-NEXT:                VariableDeclaration
// CHECK-NEXT:                    VariableDeclarator
// CHECK-NEXT:                        CallExpression : number
// CHECK-NEXT:                            MemberExpression
// CHECK-NEXT:                                Id 'withThis' [D:E:%d.3 'withThis'] : %function.4
// CHECK-NEXT:                                Id 'call'
// CHECK-NEXT:                            Id 'c' [D:E:%d.14 'c'] : %class.2
// CHECK-NEXT:                            NumericLiteral : number
// CHECK-NEXT:                        Id 'r1' [D:E:%d.15 'r1']
// CHECK-NEXT:                VariableDeclaration
// CHECK-NEXT:                    VariableDeclarator
// CHECK-NEXT:                        CallExpression : boolean
// CHECK-NEXT:                            MemberExpression
// CHECK-NEXT:                                Id 'noThis' [D:E:%d.4 'noThis'] : %function.5
// CHECK-NEXT:                                Id 'call'
// CHECK-NEXT:                            Id 'undefined' [D:E:%d.18 'undefined'] : void
// CHECK-NEXT:                            NumericLiteral : number
// CHECK-NEXT:                            StringLiteral : string
// CHECK-NEXT:                        Id 'r2' [D:E:%d.16 'r2']
