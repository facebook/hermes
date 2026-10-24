/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -typed -dump-sema -fno-std-globals %s | %FileCheckOrRegen --match-full-lines %s

class A {
  x: number = 0;
  foo(): number {
    return 1;
  }
  inherited(): number {
    return 2;
  }
}

class B extends A {
  foo(): number {
    return 3;
  }
  own(): number {
    return 4;
  }
}

// The type of `.prototype` is the home object, so methods can be read off it.
let p = A.prototype;
let f = A.prototype.foo;
A.prototype.foo.call(new A());
B.prototype.foo.call(new B());
B.prototype.inherited.call(new B());
B.prototype.own.call(new B());

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%class.2 = class(A {
// CHECK-NEXT:  %homeObject: %class.7
// CHECK-NEXT:  x: number
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.3 = class_constructor(%class.2)
// CHECK-NEXT:%class.4 = class(B extends %class.2 {
// CHECK-NEXT:  %homeObject: %class.9
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.5 = class_constructor(%class.4)
// CHECK-NEXT:%function.6 = function(this: %class.2): number
// CHECK-NEXT:%class.7 = class( {
// CHECK-NEXT:  foo [overridden]: %function.6
// CHECK-NEXT:  inherited [final]: %function.6
// CHECK-NEXT:})
// CHECK-NEXT:%function.8 = function(this: %class.4): number
// CHECK-NEXT:%class.9 = class( extends %class.7 {
// CHECK-NEXT:  foo [final]: %function.8
// CHECK-NEXT:  own [final]: %function.8
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict mayReachImplicitReturn
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'A' Class : %class_constructor.3
// CHECK-NEXT:        Decl %d.3 'B' Class : %class_constructor.5
// CHECK-NEXT:        Decl %d.4 'p' Let : %class.7
// CHECK-NEXT:        Decl %d.5 'f' Let : %function.6
// CHECK-NEXT:        Decl %d.6 'arguments' Var Arguments
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:        Scope %s.3
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.4
// CHECK-NEXT:            Decl %d.7 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.5
// CHECK-NEXT:            Decl %d.8 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.6
// CHECK-NEXT:            Decl %d.9 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.7
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.8
// CHECK-NEXT:            Decl %d.10 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.9
// CHECK-NEXT:            Decl %d.11 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.10

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        ClassDeclaration Scope %s.2
// CHECK-NEXT:            Id 'A' [D:E:%d.2 'A']
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:                ClassProperty : number
// CHECK-NEXT:                    Id 'x'
// CHECK-NEXT:                    NumericLiteral : 0
// CHECK-NEXT:                MethodDefinition : %function.6
// CHECK-NEXT:                    Id 'foo'
// CHECK-NEXT:                    FunctionExpression : %function.6
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            ReturnStatement
// CHECK-NEXT:                                NumericLiteral : 1
// CHECK-NEXT:                MethodDefinition : %function.6
// CHECK-NEXT:                    Id 'inherited'
// CHECK-NEXT:                    FunctionExpression : %function.6
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            ReturnStatement
// CHECK-NEXT:                                NumericLiteral : 2
// CHECK-NEXT:        ClassDeclaration Scope %s.3
// CHECK-NEXT:            Id 'B' [D:E:%d.3 'B']
// CHECK-NEXT:            Id 'A' [D:E:%d.2 'A'] : %class_constructor.3
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:                MethodDefinition : %function.8
// CHECK-NEXT:                    Id 'foo'
// CHECK-NEXT:                    FunctionExpression : %function.8
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            ReturnStatement
// CHECK-NEXT:                                NumericLiteral : 3
// CHECK-NEXT:                MethodDefinition : %function.8
// CHECK-NEXT:                    Id 'own'
// CHECK-NEXT:                    FunctionExpression : %function.8
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            ReturnStatement
// CHECK-NEXT:                                NumericLiteral : 4
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                MemberExpression : %class.7
// CHECK-NEXT:                    Id 'A' [D:E:%d.2 'A'] : %class_constructor.3
// CHECK-NEXT:                    Id 'prototype'
// CHECK-NEXT:                Id 'p' [D:E:%d.4 'p']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                MemberExpression : %function.6
// CHECK-NEXT:                    MemberExpression : %class.7
// CHECK-NEXT:                        Id 'A' [D:E:%d.2 'A'] : %class_constructor.3
// CHECK-NEXT:                        Id 'prototype'
// CHECK-NEXT:                    Id 'foo'
// CHECK-NEXT:                Id 'f' [D:E:%d.5 'f']
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            CallExpression : number
// CHECK-NEXT:                MemberExpression
// CHECK-NEXT:                    MemberExpression : %function.6
// CHECK-NEXT:                        MemberExpression : %class.7
// CHECK-NEXT:                            Id 'A' [D:E:%d.2 'A'] : %class_constructor.3
// CHECK-NEXT:                            Id 'prototype'
// CHECK-NEXT:                        Id 'foo'
// CHECK-NEXT:                    Id 'call'
// CHECK-NEXT:                NewExpression : %class.2
// CHECK-NEXT:                    Id 'A' [D:E:%d.2 'A'] : %class_constructor.3
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            CallExpression : number
// CHECK-NEXT:                MemberExpression
// CHECK-NEXT:                    MemberExpression : %function.8
// CHECK-NEXT:                        MemberExpression : %class.9
// CHECK-NEXT:                            Id 'B' [D:E:%d.3 'B'] : %class_constructor.5
// CHECK-NEXT:                            Id 'prototype'
// CHECK-NEXT:                        Id 'foo'
// CHECK-NEXT:                    Id 'call'
// CHECK-NEXT:                NewExpression : %class.4
// CHECK-NEXT:                    Id 'B' [D:E:%d.3 'B'] : %class_constructor.5
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            CallExpression : number
// CHECK-NEXT:                MemberExpression
// CHECK-NEXT:                    MemberExpression : %function.6
// CHECK-NEXT:                        MemberExpression : %class.9
// CHECK-NEXT:                            Id 'B' [D:E:%d.3 'B'] : %class_constructor.5
// CHECK-NEXT:                            Id 'prototype'
// CHECK-NEXT:                        Id 'inherited'
// CHECK-NEXT:                    Id 'call'
// CHECK-NEXT:                NewExpression : %class.4
// CHECK-NEXT:                    Id 'B' [D:E:%d.3 'B'] : %class_constructor.5
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            CallExpression : number
// CHECK-NEXT:                MemberExpression
// CHECK-NEXT:                    MemberExpression : %function.8
// CHECK-NEXT:                        MemberExpression : %class.9
// CHECK-NEXT:                            Id 'B' [D:E:%d.3 'B'] : %class_constructor.5
// CHECK-NEXT:                            Id 'prototype'
// CHECK-NEXT:                        Id 'own'
// CHECK-NEXT:                    Id 'call'
// CHECK-NEXT:                NewExpression : %class.4
// CHECK-NEXT:                    Id 'B' [D:E:%d.3 'B'] : %class_constructor.5
