/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals -typed -dump-sema %s | %FileCheckOrRegen %s --match-full-lines

class AAA {
  // Instantiating B<number> here shouldn't fail.
  x: BBB<number> | null;

  constructor() {
    this.x = null;
  }
}

class BBB<T> {
  root: AAA;

  constructor(root: AAA, val: T) {
    root.x;
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%class.2 = class(AAA {
// CHECK-NEXT:  %constructor: %function.3
// CHECK-NEXT:  %homeObject: %class.4
// CHECK-NEXT:  x: %union.5
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.6 = class_constructor(%class.2)
// CHECK-NEXT:%class.7 = class(BBB {
// CHECK-NEXT:  %constructor: %function.8
// CHECK-NEXT:  %homeObject: %class.9
// CHECK-NEXT:  root: %class.2
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.10 = class_constructor(%class.7)
// CHECK-NEXT:%union.5 = union(null | %class.7)
// CHECK-NEXT:%function.3 = function(this: %class.2): void
// CHECK-NEXT:%class.4 = class( {
// CHECK-NEXT:})
// CHECK-NEXT:%function.8 = function(this: %class.7, root: %class.2, val: number): void
// CHECK-NEXT:%class.9 = class( {
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'AAA' Class : %class_constructor.6
// CHECK-NEXT:            Decl %d.3 'BBB' Class
// CHECK-NEXT:            Decl %d.4 'arguments' Var Arguments
// CHECK-NEXT:            Decl %d.5 'BBB' Class : %class_constructor.10
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:                Decl %d.6 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.4
// CHECK-NEXT:                Decl %d.7 'root' Parameter
// CHECK-NEXT:                Decl %d.8 'val' Parameter
// CHECK-NEXT:                Decl %d.9 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.5
// CHECK-NEXT:                Decl %d.10 'root' Parameter : %class.2
// CHECK-NEXT:                Decl %d.11 'val' Parameter : number
// CHECK-NEXT:                Decl %d.12 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    ClassDeclaration
// CHECK-NEXT:                        Id 'AAA' [D:E:%d.2 'AAA']
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            ClassProperty : %union.5
// CHECK-NEXT:                                Id 'x'
// CHECK-NEXT:                            MethodDefinition : %function.3
// CHECK-NEXT:                                Id 'constructor'
// CHECK-NEXT:                                FunctionExpression : %function.3
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        ExpressionStatement
// CHECK-NEXT:                                            AssignmentExpression : null
// CHECK-NEXT:                                                MemberExpression : %union.5
// CHECK-NEXT:                                                    ThisExpression : %class.2
// CHECK-NEXT:                                                    Id 'x'
// CHECK-NEXT:                                                NullLiteral : null
// CHECK-NEXT:                    ClassDeclaration
// CHECK-NEXT:                        Id 'BBB' [D:E:%d.5 'BBB']
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            ClassProperty : %class.2
// CHECK-NEXT:                                Id 'root'
// CHECK-NEXT:                            MethodDefinition : %function.8
// CHECK-NEXT:                                Id 'constructor'
// CHECK-NEXT:                                FunctionExpression : %function.8
// CHECK-NEXT:                                    Id 'root' [D:E:%d.10 'root']
// CHECK-NEXT:                                    Id 'val' [D:E:%d.11 'val']
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        ExpressionStatement
// CHECK-NEXT:                                            MemberExpression : %union.5
// CHECK-NEXT:                                                Id 'root' [D:E:%d.10 'root'] : %class.2
// CHECK-NEXT:                                                Id 'x'
// CHECK-NEXT:                    ClassDeclaration
// CHECK-NEXT:                        Id 'BBB' [D:E:%d.3 'BBB']
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            ClassProperty
// CHECK-NEXT:                                Id 'root'
// CHECK-NEXT:                            MethodDefinition
// CHECK-NEXT:                                Id 'constructor'
// CHECK-NEXT:                                FunctionExpression
// CHECK-NEXT:                                    Id 'root' [D:E:%d.7 'root']
// CHECK-NEXT:                                    Id 'val' [D:E:%d.8 'val']
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        ExpressionStatement
// CHECK-NEXT:                                            MemberExpression
// CHECK-NEXT:                                                Id 'root' [D:E:%d.7 'root']
// CHECK-NEXT:                                                Id 'x'
// CHECK-NEXT:            ObjectExpression
