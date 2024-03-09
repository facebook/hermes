/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals --typed --dump-sema %s | %FileCheckOrRegen %s --match-full-lines

'use strict';

class C0 {
    x = 7;
    m(): number {
        return this.x + 1;
    }
}

class C1 extends C0 {
    x = super.m() + 1;
    y: number;
    z: number = this.x + 2;
    constructor() {
        super();
        this.y = super.m() + 1;
    }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%class.2 = class(C0 {
// CHECK-NEXT:  %homeObject: %class.3
// CHECK-NEXT:  x: any
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.4 = class_constructor(%class.2)
// CHECK-NEXT:%class.5 = class(C1 extends %class.2 {
// CHECK-NEXT:  %constructor: %function.6
// CHECK-NEXT:  %homeObject: %class.7
// CHECK-NEXT:  x: any
// CHECK-NEXT:  y: number
// CHECK-NEXT:  z: number
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.8 = class_constructor(%class.5)
// CHECK-NEXT:%function.9 = function(this: %class.2): number
// CHECK-NEXT:%class.3 = class( {
// CHECK-NEXT:  m [final]: %function.9
// CHECK-NEXT:})
// CHECK-NEXT:%function.6 = function(this: %class.5): void
// CHECK-NEXT:%class.7 = class( extends %class.3 {
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'C0' Class : %class_constructor.4
// CHECK-NEXT:            Decl %d.3 'C1' Class : %class_constructor.8
// CHECK-NEXT:            Decl %d.4 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.4
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.5
// CHECK-NEXT:                Decl %d.5 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.6
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.7
// CHECK-NEXT:                Decl %d.6 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    ExpressionStatement
// CHECK-NEXT:                        StringLiteral : string
// CHECK-NEXT:                    ClassDeclaration
// CHECK-NEXT:                        Id 'C0' [D:E:%d.2 'C0']
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            ClassProperty : any
// CHECK-NEXT:                                Id 'x'
// CHECK-NEXT:                                NumericLiteral : number
// CHECK-NEXT:                            MethodDefinition : %function.9
// CHECK-NEXT:                                Id 'm'
// CHECK-NEXT:                                FunctionExpression : %function.9
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        ReturnStatement
// CHECK-NEXT:                                            ImplicitCheckedCast : number
// CHECK-NEXT:                                                BinaryExpression : any
// CHECK-NEXT:                                                    MemberExpression : any
// CHECK-NEXT:                                                        ThisExpression : %class.2
// CHECK-NEXT:                                                        Id 'x'
// CHECK-NEXT:                                                    NumericLiteral : number
// CHECK-NEXT:                    ClassDeclaration
// CHECK-NEXT:                        Id 'C1' [D:E:%d.3 'C1']
// CHECK-NEXT:                        Id 'C0' [D:E:%d.2 'C0'] : %class_constructor.4
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            ClassProperty : any
// CHECK-NEXT:                                Id 'x'
// CHECK-NEXT:                                BinaryExpression : number
// CHECK-NEXT:                                    CallExpression : number
// CHECK-NEXT:                                        MemberExpression : %function.9
// CHECK-NEXT:                                            Super : %class.2
// CHECK-NEXT:                                            Id 'm'
// CHECK-NEXT:                                    NumericLiteral : number
// CHECK-NEXT:                            ClassProperty : number
// CHECK-NEXT:                                Id 'y'
// CHECK-NEXT:                            ClassProperty : number
// CHECK-NEXT:                                Id 'z'
// CHECK-NEXT:                                BinaryExpression : any
// CHECK-NEXT:                                    MemberExpression : any
// CHECK-NEXT:                                        ThisExpression : any
// CHECK-NEXT:                                        Id 'x'
// CHECK-NEXT:                                    NumericLiteral : number
// CHECK-NEXT:                            MethodDefinition : %function.6
// CHECK-NEXT:                                Id 'constructor'
// CHECK-NEXT:                                FunctionExpression : %function.6
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        ExpressionStatement
// CHECK-NEXT:                                            CallExpression
// CHECK-NEXT:                                                Super
// CHECK-NEXT:                                        ExpressionStatement
// CHECK-NEXT:                                            AssignmentExpression : number
// CHECK-NEXT:                                                MemberExpression : number
// CHECK-NEXT:                                                    ThisExpression : %class.5
// CHECK-NEXT:                                                    Id 'y'
// CHECK-NEXT:                                                BinaryExpression : number
// CHECK-NEXT:                                                    CallExpression : number
// CHECK-NEXT:                                                        MemberExpression : %function.9
// CHECK-NEXT:                                                            Super : %class.2
// CHECK-NEXT:                                                            Id 'm'
// CHECK-NEXT:                                                    NumericLiteral : number
// CHECK-NEXT:            ObjectExpression
