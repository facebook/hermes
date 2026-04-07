/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -typed -dump-sema %s | %FileCheckOrRegen %s --match-full-lines

(function() {
// The resulting type is a union of its elements.
var unionArr = [1, "", undefined];

// Assigning to an annotated variable lets us use its type.
var annotatedArr: (string | number | void)[] = [1, ""];

// The two arrays have the same type.
annotatedArr = unionArr

class Base {}
class Derived extends Base {
    constructor(){
        super();
    }
}

var baseArr: Base[] = [new Base(), new Derived()];

var numArr: number[] = [1, 2, 3];
var strArr: (number | string)[] = ["a", "b", "c", ...numArr];
})();

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%class.2 = class(Base {
// CHECK-NEXT:  %homeObject: %class.11
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.3 = class_constructor(%class.2)
// CHECK-NEXT:%class.4 = class(Derived extends %class.2 {
// CHECK-NEXT:  %constructor: %function.6
// CHECK-NEXT:  %homeObject: %class.12
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.5 = class_constructor(%class.4)
// CHECK-NEXT:%function.6 = function(this: %class.4): void
// CHECK-NEXT:%class.7 = class(Array<%union.13>)
// CHECK-NEXT:%class.8 = class(Array<%class.2>)
// CHECK-NEXT:%class.9 = class(Array<number>)
// CHECK-NEXT:%class.10 = class(Array<%union.14>)
// CHECK-NEXT:%class.11 = class( {
// CHECK-NEXT:})
// CHECK-NEXT:%class.12 = class( extends %class.11 {
// CHECK-NEXT:})
// CHECK-NEXT:%union.13 = union(void | string | number)
// CHECK-NEXT:%union.14 = union(string | number)

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'arguments' Var Arguments
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.3 'unionArr' Var : %class.7
// CHECK-NEXT:            Decl %d.4 'annotatedArr' Var : %class.7
// CHECK-NEXT:            Decl %d.5 'Base' Class : %class_constructor.3
// CHECK-NEXT:            Decl %d.6 'Derived' Class : %class_constructor.5
// CHECK-NEXT:            Decl %d.7 'baseArr' Var : %class.8
// CHECK-NEXT:            Decl %d.8 'numArr' Var : %class.9
// CHECK-NEXT:            Decl %d.9 'strArr' Var : %class.10
// CHECK-NEXT:            Decl %d.10 'arguments' Var Arguments
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:            Scope %s.4
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.5
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.6
// CHECK-NEXT:                Decl %d.11 'arguments' Var Arguments

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            CallExpression : any
// CHECK-NEXT:                FunctionExpression : %untyped_function.1
// CHECK-NEXT:                    BlockStatement
// CHECK-NEXT:                        VariableDeclaration
// CHECK-NEXT:                            VariableDeclarator
// CHECK-NEXT:                                ArrayExpression : %class.7
// CHECK-NEXT:                                    NumericLiteral : number
// CHECK-NEXT:                                    StringLiteral : string
// CHECK-NEXT:                                    Id 'undefined' [D:E:%d.12 'undefined'] : void
// CHECK-NEXT:                                Id 'unionArr' [D:E:%d.3 'unionArr']
// CHECK-NEXT:                        VariableDeclaration
// CHECK-NEXT:                            VariableDeclarator
// CHECK-NEXT:                                ArrayExpression : %class.7
// CHECK-NEXT:                                    NumericLiteral : number
// CHECK-NEXT:                                    StringLiteral : string
// CHECK-NEXT:                                Id 'annotatedArr' [D:E:%d.4 'annotatedArr']
// CHECK-NEXT:                        ExpressionStatement
// CHECK-NEXT:                            AssignmentExpression : %class.7
// CHECK-NEXT:                                Id 'annotatedArr' [D:E:%d.4 'annotatedArr'] : %class.7
// CHECK-NEXT:                                Id 'unionArr' [D:E:%d.3 'unionArr'] : %class.7
// CHECK-NEXT:                        ClassDeclaration Scope %s.3
// CHECK-NEXT:                            Id 'Base' [D:E:%d.5 'Base']
// CHECK-NEXT:                            ClassBody
// CHECK-NEXT:                        ClassDeclaration Scope %s.4
// CHECK-NEXT:                            Id 'Derived' [D:E:%d.6 'Derived']
// CHECK-NEXT:                            Id 'Base' [D:E:%d.5 'Base'] : %class_constructor.3
// CHECK-NEXT:                            ClassBody
// CHECK-NEXT:                                MethodDefinition : %function.6
// CHECK-NEXT:                                    Id 'constructor'
// CHECK-NEXT:                                    FunctionExpression : %function.6
// CHECK-NEXT:                                        BlockStatement
// CHECK-NEXT:                                            ExpressionStatement
// CHECK-NEXT:                                                CallExpression
// CHECK-NEXT:                                                    Super
// CHECK-NEXT:                        VariableDeclaration
// CHECK-NEXT:                            VariableDeclarator
// CHECK-NEXT:                                ArrayExpression : %class.8
// CHECK-NEXT:                                    NewExpression : %class.2
// CHECK-NEXT:                                        Id 'Base' [D:E:%d.5 'Base'] : %class_constructor.3
// CHECK-NEXT:                                    NewExpression : %class.4
// CHECK-NEXT:                                        Id 'Derived' [D:E:%d.6 'Derived'] : %class_constructor.5
// CHECK-NEXT:                                Id 'baseArr' [D:E:%d.7 'baseArr']
// CHECK-NEXT:                        VariableDeclaration
// CHECK-NEXT:                            VariableDeclarator
// CHECK-NEXT:                                ArrayExpression : %class.9
// CHECK-NEXT:                                    NumericLiteral : number
// CHECK-NEXT:                                    NumericLiteral : number
// CHECK-NEXT:                                    NumericLiteral : number
// CHECK-NEXT:                                Id 'numArr' [D:E:%d.8 'numArr']
// CHECK-NEXT:                        VariableDeclaration
// CHECK-NEXT:                            VariableDeclarator
// CHECK-NEXT:                                ArrayExpression : %class.10
// CHECK-NEXT:                                    StringLiteral : string
// CHECK-NEXT:                                    StringLiteral : string
// CHECK-NEXT:                                    StringLiteral : string
// CHECK-NEXT:                                    SpreadElement
// CHECK-NEXT:                                        Id 'numArr' [D:E:%d.8 'numArr'] : %class.9
// CHECK-NEXT:                                Id 'strArr' [D:E:%d.9 'strArr']
