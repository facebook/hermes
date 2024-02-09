/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals -typed -dump-sema %s | %FileCheckOrRegen %s --match-full-lines

class C<T> {
  x: T;

  constructor(x: T) {
    this.x = x;
  }
}

// All of this should result in only one specialization of C<number | string>.
var c1: C<number | string> = new C<number | string>(3);
c1 = new C<number | string>('abc');

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%union.2 = union(string | number)
// CHECK-NEXT:%class.3 = class(C {
// CHECK-NEXT:  %constructor: %function.4
// CHECK-NEXT:  %homeObject: %class.5
// CHECK-NEXT:  x: %union.2
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.6 = class_constructor(%class.3)
// CHECK-NEXT:%function.4 = function(this: %class.3, x: %union.2): void
// CHECK-NEXT:%class.5 = class( {
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'C' Class
// CHECK-NEXT:            Decl %d.3 'c1' Var : %class.3
// CHECK-NEXT:            Decl %d.4 'arguments' Var Arguments
// CHECK-NEXT:            Decl %d.5 'C' Class : %class_constructor.6
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:                Decl %d.6 'x' Parameter
// CHECK-NEXT:                Decl %d.7 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.4
// CHECK-NEXT:                Decl %d.8 'x' Parameter : %union.2
// CHECK-NEXT:                Decl %d.9 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    ClassDeclaration
// CHECK-NEXT:                        Id 'C' [D:E:%d.5 'C']
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            ClassProperty : %union.2
// CHECK-NEXT:                                Id 'x'
// CHECK-NEXT:                            MethodDefinition : %function.4
// CHECK-NEXT:                                Id 'constructor'
// CHECK-NEXT:                                FunctionExpression : %function.4
// CHECK-NEXT:                                    Id 'x' [D:E:%d.8 'x']
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        ExpressionStatement
// CHECK-NEXT:                                            AssignmentExpression : %union.2
// CHECK-NEXT:                                                MemberExpression : %union.2
// CHECK-NEXT:                                                    ThisExpression : %class.3
// CHECK-NEXT:                                                    Id 'x'
// CHECK-NEXT:                                                Id 'x' [D:E:%d.8 'x'] : %union.2
// CHECK-NEXT:                    ClassDeclaration
// CHECK-NEXT:                        Id 'C' [D:E:%d.2 'C']
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            ClassProperty
// CHECK-NEXT:                                Id 'x'
// CHECK-NEXT:                            MethodDefinition
// CHECK-NEXT:                                Id 'constructor'
// CHECK-NEXT:                                FunctionExpression
// CHECK-NEXT:                                    Id 'x' [D:E:%d.6 'x']
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        ExpressionStatement
// CHECK-NEXT:                                            AssignmentExpression
// CHECK-NEXT:                                                MemberExpression
// CHECK-NEXT:                                                    ThisExpression
// CHECK-NEXT:                                                    Id 'x'
// CHECK-NEXT:                                                Id 'x' [D:E:%d.6 'x']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            NewExpression : %class.3
// CHECK-NEXT:                                Id 'C' [D:E:%d.5 'C'] : %class_constructor.6
// CHECK-NEXT:                                TypeParameterInstantiation
// CHECK-NEXT:                                    UnionTypeAnnotation
// CHECK-NEXT:                                        NumberTypeAnnotation
// CHECK-NEXT:                                        StringTypeAnnotation
// CHECK-NEXT:                                NumericLiteral : number
// CHECK-NEXT:                            Id 'c1' [D:E:%d.3 'c1']
// CHECK-NEXT:                    ExpressionStatement
// CHECK-NEXT:                        AssignmentExpression : %class.3
// CHECK-NEXT:                            Id 'c1' [D:E:%d.3 'c1'] : %class.3
// CHECK-NEXT:                            NewExpression : %class.3
// CHECK-NEXT:                                Id 'C' [D:E:%d.5 'C'] : %class_constructor.6
// CHECK-NEXT:                                TypeParameterInstantiation
// CHECK-NEXT:                                    UnionTypeAnnotation
// CHECK-NEXT:                                        NumberTypeAnnotation
// CHECK-NEXT:                                        StringTypeAnnotation
// CHECK-NEXT:                                StringLiteral : string
// CHECK-NEXT:            ObjectExpression
