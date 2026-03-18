/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals -Werror -typed -dump-sema %s | %FileCheckOrRegen --match-full-lines %s

/// Assigning ?Foo to Foo | void needs to cast to the correct type.

class Foo {
  key: string;
  constructor(key: string) { this.key = key; }
}

function maybeFoo(s: string): ?Foo {
  if (s === "a") return new Foo("a");
  return;
}

const v: Foo | void = maybeFoo("a");

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%class.2 = class(Foo {
// CHECK-NEXT:  %constructor: %function.3
// CHECK-NEXT:  %homeObject: %class.4
// CHECK-NEXT:  key: string
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.5 = class_constructor(%class.2)
// CHECK-NEXT:%function.3 = function(this: %class.2, key: string): void
// CHECK-NEXT:%class.4 = class( {
// CHECK-NEXT:})
// CHECK-NEXT:%union.6 = union(void | %class.2)
// CHECK-NEXT:%union.7 = union(void | null | %class.2)
// CHECK-NEXT:%function.8 = function(s: string): %union.7
// CHECK-NEXT:%object.9 = object({
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'Foo' Class : %class_constructor.5
// CHECK-NEXT:            Decl %d.3 'maybeFoo' Var : %function.8
// CHECK-NEXT:            Decl %d.4 'v' Const : %union.6
// CHECK-NEXT:            Decl %d.5 'arguments' Var Arguments
// CHECK-NEXT:            hoistedFunction maybeFoo
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.4
// CHECK-NEXT:                Decl %d.6 'key' Parameter : string
// CHECK-NEXT:                Decl %d.7 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.5
// CHECK-NEXT:                Decl %d.8 's' Parameter : string
// CHECK-NEXT:                Decl %d.9 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    ClassDeclaration Scope %s.3
// CHECK-NEXT:                        Id 'Foo' [D:E:%d.2 'Foo']
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            ClassProperty : string
// CHECK-NEXT:                                Id 'key'
// CHECK-NEXT:                            MethodDefinition : %function.3
// CHECK-NEXT:                                Id 'constructor'
// CHECK-NEXT:                                FunctionExpression : %function.3
// CHECK-NEXT:                                    Id 'key' [D:E:%d.6 'key']
// CHECK-NEXT:                                    BlockStatement
// CHECK-NEXT:                                        ExpressionStatement
// CHECK-NEXT:                                            AssignmentExpression : string
// CHECK-NEXT:                                                MemberExpression : string
// CHECK-NEXT:                                                    ThisExpression : %class.2
// CHECK-NEXT:                                                    Id 'key'
// CHECK-NEXT:                                                Id 'key' [D:E:%d.6 'key'] : string
// CHECK-NEXT:                    FunctionDeclaration : %function.8
// CHECK-NEXT:                        Id 'maybeFoo' [D:E:%d.3 'maybeFoo']
// CHECK-NEXT:                        Id 's' [D:E:%d.8 's']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            IfStatement
// CHECK-NEXT:                                BinaryExpression : boolean
// CHECK-NEXT:                                    Id 's' [D:E:%d.8 's'] : string
// CHECK-NEXT:                                    StringLiteral : string
// CHECK-NEXT:                                ReturnStatement
// CHECK-NEXT:                                    NewExpression : %class.2
// CHECK-NEXT:                                        Id 'Foo' [D:E:%d.2 'Foo'] : %class_constructor.5
// CHECK-NEXT:                                        StringLiteral : string
// CHECK-NEXT:                            ReturnStatement
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            ImplicitCheckedCast : %union.6
// CHECK-NEXT:                                CallExpression : %union.7
// CHECK-NEXT:                                    Id 'maybeFoo' [D:E:%d.3 'maybeFoo'] : %function.8
// CHECK-NEXT:                                    StringLiteral : string
// CHECK-NEXT:                            Id 'v' [D:E:%d.4 'v']
// CHECK-NEXT:            ObjectExpression : %object.9
