/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals -typed -dump-sema %s | %FileCheckOrRegen %s --match-full-lines

class ID<T> {
  val: T;

  constructor(val: T) {
    this.val = val;
  }
}

const i1: ID<number> = new ID<number>(1);
const n: number = i1.val;

const i2: ID<string> = new ID<string>('abc');
const s: string = i2.val;

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%class.2 = class(ID {
// CHECK-NEXT:  %constructor: %function.3
// CHECK-NEXT:  %homeObject: %class.4
// CHECK-NEXT:  val: number
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.5 = class_constructor(%class.2)
// CHECK-NEXT:%function.3 = function(this: %class.2, val: number): void
// CHECK-NEXT:%class.4 = class( {
// CHECK-NEXT:})
// CHECK-NEXT:%class.6 = class(ID {
// CHECK-NEXT:  %constructor: %function.7
// CHECK-NEXT:  %homeObject: %class.8
// CHECK-NEXT:  val: string
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.9 = class_constructor(%class.6)
// CHECK-NEXT:%function.7 = function(this: %class.6, val: string): void
// CHECK-NEXT:%class.8 = class( {
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'ID' Class
// CHECK-NEXT:            Decl %d.3 'i1' Const : %class.2
// CHECK-NEXT:            Decl %d.4 'n' Const : number
// CHECK-NEXT:            Decl %d.5 'i2' Const : %class.6
// CHECK-NEXT:            Decl %d.6 's' Const : string
// CHECK-NEXT:            Decl %d.7 'arguments' Var Arguments
// CHECK-NEXT:            Decl %d.8 'ID' Class : %class_constructor.5
// CHECK-NEXT:            Decl %d.9 'ID' Class : %class_constructor.9
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:                Decl %d.10 'val' Parameter
// CHECK-NEXT:                Decl %d.11 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.4
// CHECK-NEXT:                Decl %d.12 'val' Parameter : number
// CHECK-NEXT:                Decl %d.13 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.5
// CHECK-NEXT:                Decl %d.14 'val' Parameter : string
// CHECK-NEXT:                Decl %d.15 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    ClassDeclaration
// CHECK-NEXT:                        Id 'ID' [D:E:%d.8 'ID']
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            ClassProperty : number
// CHECK-NEXT:                                Id 'val'
// CHECK-NEXT:                            MethodDefinition : %function.3
// CHECK-NEXT:                                Id 'constructor'
// CHECK-NEXT:                                FunctionExpression : %function.3
// CHECK-NEXT:                                    Id 'val' [D:E:%d.12 'val']
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        ExpressionStatement
// CHECK-NEXT:                                            AssignmentExpression : number
// CHECK-NEXT:                                                MemberExpression : number
// CHECK-NEXT:                                                    ThisExpression : %class.2
// CHECK-NEXT:                                                    Id 'val'
// CHECK-NEXT:                                                Id 'val' [D:E:%d.12 'val'] : number
// CHECK-NEXT:                    ClassDeclaration
// CHECK-NEXT:                        Id 'ID' [D:E:%d.9 'ID']
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            ClassProperty : string
// CHECK-NEXT:                                Id 'val'
// CHECK-NEXT:                            MethodDefinition : %function.7
// CHECK-NEXT:                                Id 'constructor'
// CHECK-NEXT:                                FunctionExpression : %function.7
// CHECK-NEXT:                                    Id 'val' [D:E:%d.14 'val']
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        ExpressionStatement
// CHECK-NEXT:                                            AssignmentExpression : string
// CHECK-NEXT:                                                MemberExpression : string
// CHECK-NEXT:                                                    ThisExpression : %class.6
// CHECK-NEXT:                                                    Id 'val'
// CHECK-NEXT:                                                Id 'val' [D:E:%d.14 'val'] : string
// CHECK-NEXT:                    ClassDeclaration
// CHECK-NEXT:                        Id 'ID' [D:E:%d.2 'ID']
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            ClassProperty
// CHECK-NEXT:                                Id 'val'
// CHECK-NEXT:                            MethodDefinition
// CHECK-NEXT:                                Id 'constructor'
// CHECK-NEXT:                                FunctionExpression
// CHECK-NEXT:                                    Id 'val' [D:E:%d.10 'val']
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        ExpressionStatement
// CHECK-NEXT:                                            AssignmentExpression
// CHECK-NEXT:                                                MemberExpression
// CHECK-NEXT:                                                    ThisExpression
// CHECK-NEXT:                                                    Id 'val'
// CHECK-NEXT:                                                Id 'val' [D:E:%d.10 'val']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            NewExpression : %class.2
// CHECK-NEXT:                                Id 'ID' [D:E:%d.8 'ID'] : %class_constructor.5
// CHECK-NEXT:                                TypeParameterInstantiation
// CHECK-NEXT:                                    NumberTypeAnnotation
// CHECK-NEXT:                                NumericLiteral : number
// CHECK-NEXT:                            Id 'i1' [D:E:%d.3 'i1']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            MemberExpression : number
// CHECK-NEXT:                                Id 'i1' [D:E:%d.3 'i1'] : %class.2
// CHECK-NEXT:                                Id 'val'
// CHECK-NEXT:                            Id 'n' [D:E:%d.4 'n']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            NewExpression : %class.6
// CHECK-NEXT:                                Id 'ID' [D:E:%d.9 'ID'] : %class_constructor.9
// CHECK-NEXT:                                TypeParameterInstantiation
// CHECK-NEXT:                                    StringTypeAnnotation
// CHECK-NEXT:                                StringLiteral : string
// CHECK-NEXT:                            Id 'i2' [D:E:%d.5 'i2']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            MemberExpression : string
// CHECK-NEXT:                                Id 'i2' [D:E:%d.5 'i2'] : %class.6
// CHECK-NEXT:                                Id 'val'
// CHECK-NEXT:                            Id 's' [D:E:%d.6 's']
// CHECK-NEXT:            ObjectExpression
