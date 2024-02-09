/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals -typed -dump-sema %s | %FileCheckOrRegen %s --match-full-lines

class A {
  c: C<number> | null;
}
class B {
  func(): void {}
}
class C<Props> {
  vals: B[];
  foo(): void {
    let b: B = this.vals[0];
    b.func();
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%class.2 = class(A {
// CHECK-NEXT:  %homeObject: %class.3
// CHECK-NEXT:  c: %union.4
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.5 = class_constructor(%class.2)
// CHECK-NEXT:%class.6 = class(B {
// CHECK-NEXT:  %homeObject: %class.7
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.8 = class_constructor(%class.6)
// CHECK-NEXT:%class.9 = class(C {
// CHECK-NEXT:  %homeObject: %class.10
// CHECK-NEXT:  vals: %array.11
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.12 = class_constructor(%class.9)
// CHECK-NEXT:%union.4 = union(null | %class.9)
// CHECK-NEXT:%class.3 = class( {
// CHECK-NEXT:})
// CHECK-NEXT:%function.13 = function(this: %class.6): void
// CHECK-NEXT:%class.7 = class( {
// CHECK-NEXT:  func [final]: %function.13
// CHECK-NEXT:})
// CHECK-NEXT:%array.11 = array(%class.6)
// CHECK-NEXT:%function.14 = function(this: %class.9): void
// CHECK-NEXT:%class.10 = class( {
// CHECK-NEXT:  foo [final]: %function.14
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'A' Class : %class_constructor.5
// CHECK-NEXT:            Decl %d.3 'B' Class : %class_constructor.8
// CHECK-NEXT:            Decl %d.4 'C' Class
// CHECK-NEXT:            Decl %d.5 'arguments' Var Arguments
// CHECK-NEXT:            Decl %d.6 'C' Class : %class_constructor.12
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:                Decl %d.7 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.4
// CHECK-NEXT:                Decl %d.8 'b' Let
// CHECK-NEXT:                Decl %d.9 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.5
// CHECK-NEXT:                Decl %d.10 'b' Let : %class.6
// CHECK-NEXT:                Decl %d.11 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    ClassDeclaration
// CHECK-NEXT:                        Id 'A' [D:E:%d.2 'A']
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            ClassProperty : %union.4
// CHECK-NEXT:                                Id 'c'
// CHECK-NEXT:                    ClassDeclaration
// CHECK-NEXT:                        Id 'B' [D:E:%d.3 'B']
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            MethodDefinition : %function.13
// CHECK-NEXT:                                Id 'func'
// CHECK-NEXT:                                FunctionExpression : %function.13
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                    ClassDeclaration
// CHECK-NEXT:                        Id 'C' [D:E:%d.6 'C']
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            ClassProperty : %array.11
// CHECK-NEXT:                                Id 'vals'
// CHECK-NEXT:                            MethodDefinition : %function.14
// CHECK-NEXT:                                Id 'foo'
// CHECK-NEXT:                                FunctionExpression : %function.14
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        VariableDeclaration
// CHECK-NEXT:                                            VariableDeclarator
// CHECK-NEXT:                                                MemberExpression : %class.6
// CHECK-NEXT:                                                    MemberExpression : %array.11
// CHECK-NEXT:                                                        ThisExpression : %class.9
// CHECK-NEXT:                                                        Id 'vals'
// CHECK-NEXT:                                                    NumericLiteral : number
// CHECK-NEXT:                                                Id 'b' [D:E:%d.10 'b']
// CHECK-NEXT:                                        ExpressionStatement
// CHECK-NEXT:                                            CallExpression : void
// CHECK-NEXT:                                                MemberExpression : %function.13
// CHECK-NEXT:                                                    Id 'b' [D:E:%d.10 'b'] : %class.6
// CHECK-NEXT:                                                    Id 'func'
// CHECK-NEXT:                    ClassDeclaration
// CHECK-NEXT:                        Id 'C' [D:E:%d.4 'C']
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            ClassProperty
// CHECK-NEXT:                                Id 'vals'
// CHECK-NEXT:                            MethodDefinition
// CHECK-NEXT:                                Id 'foo'
// CHECK-NEXT:                                FunctionExpression
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        VariableDeclaration
// CHECK-NEXT:                                            VariableDeclarator
// CHECK-NEXT:                                                MemberExpression
// CHECK-NEXT:                                                    MemberExpression
// CHECK-NEXT:                                                        ThisExpression
// CHECK-NEXT:                                                        Id 'vals'
// CHECK-NEXT:                                                    NumericLiteral
// CHECK-NEXT:                                                Id 'b' [D:E:%d.8 'b']
// CHECK-NEXT:                                        ExpressionStatement
// CHECK-NEXT:                                            CallExpression
// CHECK-NEXT:                                                MemberExpression
// CHECK-NEXT:                                                    Id 'b' [D:E:%d.8 'b']
// CHECK-NEXT:                                                    Id 'func'
// CHECK-NEXT:            ObjectExpression
