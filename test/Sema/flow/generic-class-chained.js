/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals -typed -dump-sema %s | %FileCheckOrRegen %s --match-full-lines

class A {
  x: B<number>;
}

class B<T> {
  foo() {
    let c: C<number> = new C<number>();
    c.run();
  }
}

class C<T> {
  run(): void {}
}

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%class.2 = class(A {
// CHECK-NEXT:  %homeObject: %class.3
// CHECK-NEXT:  x: %class.4
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.5 = class_constructor(%class.2)
// CHECK-NEXT:%class.4 = class(B {
// CHECK-NEXT:  %homeObject: %class.6
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.7 = class_constructor(%class.4)
// CHECK-NEXT:%class.3 = class( {
// CHECK-NEXT:})
// CHECK-NEXT:%class.6 = class( {
// CHECK-NEXT:  foo [final]: %untyped_function.1
// CHECK-NEXT:})
// CHECK-NEXT:%class.8 = class(C {
// CHECK-NEXT:  %homeObject: %class.9
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.10 = class_constructor(%class.8)
// CHECK-NEXT:%function.11 = function(this: %class.8): void
// CHECK-NEXT:%class.9 = class( {
// CHECK-NEXT:  run [final]: %function.11
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'A' Class : %class_constructor.5
// CHECK-NEXT:            Decl %d.3 'B' Class
// CHECK-NEXT:            Decl %d.4 'C' Class
// CHECK-NEXT:            Decl %d.5 'arguments' Var Arguments
// CHECK-NEXT:            Decl %d.6 'B' Class : %class_constructor.7
// CHECK-NEXT:            Decl %d.7 'C' Class : %class_constructor.10
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:                Decl %d.8 'c' Let
// CHECK-NEXT:                Decl %d.9 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.4
// CHECK-NEXT:                Decl %d.10 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.5
// CHECK-NEXT:                Decl %d.11 'c' Let : %class.8
// CHECK-NEXT:                Decl %d.12 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.6
// CHECK-NEXT:                Decl %d.13 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    ClassDeclaration
// CHECK-NEXT:                        Id 'A' [D:E:%d.2 'A']
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            ClassProperty : %class.4
// CHECK-NEXT:                                Id 'x'
// CHECK-NEXT:                    ClassDeclaration
// CHECK-NEXT:                        Id 'B' [D:E:%d.6 'B']
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            MethodDefinition : %untyped_function.1
// CHECK-NEXT:                                Id 'foo'
// CHECK-NEXT:                                FunctionExpression : %untyped_function.1
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        VariableDeclaration
// CHECK-NEXT:                                            VariableDeclarator
// CHECK-NEXT:                                                NewExpression : %class.8
// CHECK-NEXT:                                                    Id 'C' [D:E:%d.7 'C'] : %class_constructor.10
// CHECK-NEXT:                                                    TypeParameterInstantiation
// CHECK-NEXT:                                                        NumberTypeAnnotation
// CHECK-NEXT:                                                Id 'c' [D:E:%d.11 'c']
// CHECK-NEXT:                                        ExpressionStatement
// CHECK-NEXT:                                            CallExpression : void
// CHECK-NEXT:                                                MemberExpression : %function.11
// CHECK-NEXT:                                                    Id 'c' [D:E:%d.11 'c'] : %class.8
// CHECK-NEXT:                                                    Id 'run'
// CHECK-NEXT:                    ClassDeclaration
// CHECK-NEXT:                        Id 'B' [D:E:%d.3 'B']
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            MethodDefinition
// CHECK-NEXT:                                Id 'foo'
// CHECK-NEXT:                                FunctionExpression
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        VariableDeclaration
// CHECK-NEXT:                                            VariableDeclarator
// CHECK-NEXT:                                                NewExpression
// CHECK-NEXT:                                                    Id 'C' [D:E:%d.4 'C']
// CHECK-NEXT:                                                    TypeParameterInstantiation
// CHECK-NEXT:                                                        NumberTypeAnnotation
// CHECK-NEXT:                                                Id 'c' [D:E:%d.8 'c']
// CHECK-NEXT:                                        ExpressionStatement
// CHECK-NEXT:                                            CallExpression
// CHECK-NEXT:                                                MemberExpression
// CHECK-NEXT:                                                    Id 'c' [D:E:%d.8 'c']
// CHECK-NEXT:                                                    Id 'run'
// CHECK-NEXT:                    ClassDeclaration
// CHECK-NEXT:                        Id 'C' [D:E:%d.7 'C']
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            MethodDefinition : %function.11
// CHECK-NEXT:                                Id 'run'
// CHECK-NEXT:                                FunctionExpression : %function.11
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                    ClassDeclaration
// CHECK-NEXT:                        Id 'C' [D:E:%d.4 'C']
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            MethodDefinition
// CHECK-NEXT:                                Id 'run'
// CHECK-NEXT:                                FunctionExpression
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:            ObjectExpression
