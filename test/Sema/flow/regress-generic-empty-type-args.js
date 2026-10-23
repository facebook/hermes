/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -typed -dump-sema %s | %FileCheckOrRegen --match-full-lines %s

// An empty explicit type-argument list `<>` used to abort with an assertion
// failure (an empty ArrayRef aliased the DenseMap empty/tombstone key). Now
// both infer the type arguments.

class BoxNew<T> {
  v: T;
  constructor(v: T) {
    this.v = v;
  }
}
new BoxNew<>(1);

class C {
  @Hermes.final
  m<T>(x: T): void {}
}
new C().m<>(1);

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%class.2 = class(C {
// CHECK-NEXT:  %homeObject: %class.8
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.3 = class_constructor(%class.2)
// CHECK-NEXT:%class.4 = class(BoxNew {
// CHECK-NEXT:  %constructor: %function.6
// CHECK-NEXT:  %homeObject: %class.9
// CHECK-NEXT:  v: number
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.5 = class_constructor(%class.4)
// CHECK-NEXT:%function.6 = function(this: %class.4, v: number): void
// CHECK-NEXT:%function.7 = function(this: %class.2, x: number): void
// CHECK-NEXT:%class.8 = class( {
// CHECK-NEXT:  m [final]: generic
// CHECK-NEXT:})
// CHECK-NEXT:%class.9 = class( {
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict mayReachImplicitReturn
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'BoxNew' Class
// CHECK-NEXT:        Decl %d.3 'C' Class : %class_constructor.3
// CHECK-NEXT:        Decl %d.4 'arguments' Var Arguments
// CHECK-NEXT:        Decl %d.5 'BoxNew' Class : %class_constructor.5
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:        Scope %s.3
// CHECK-NEXT:            Decl %d.6 'm' Const
// CHECK-NEXT:        Scope %s.4
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.5
// CHECK-NEXT:            Decl %d.7 'v' Parameter
// CHECK-NEXT:            Decl %d.8 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.6
// CHECK-NEXT:            Decl %d.9 'x' Parameter
// CHECK-NEXT:            Decl %d.10 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.7
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.8
// CHECK-NEXT:            Decl %d.11 'v' Parameter : number
// CHECK-NEXT:            Decl %d.12 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.9
// CHECK-NEXT:            Decl %d.13 'x' Parameter : number
// CHECK-NEXT:            Decl %d.14 'arguments' Var Arguments

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        ClassDeclaration Scope %s.4
// CHECK-NEXT:            Id 'BoxNew' [D:E:%d.5 'BoxNew']
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:                ClassProperty : number
// CHECK-NEXT:                    Id 'v'
// CHECK-NEXT:                MethodDefinition : %function.6
// CHECK-NEXT:                    Id 'constructor'
// CHECK-NEXT:                    FunctionExpression : %function.6
// CHECK-NEXT:                        Id 'v' [D:E:%d.11 'v']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            ExpressionStatement
// CHECK-NEXT:                                AssignmentExpression : number
// CHECK-NEXT:                                    MemberExpression : number
// CHECK-NEXT:                                        ThisExpression : %class.4
// CHECK-NEXT:                                        Id 'v'
// CHECK-NEXT:                                    Id 'v' [D:E:%d.11 'v'] : number
// CHECK-NEXT:        ClassDeclaration Scope %s.2
// CHECK-NEXT:            Id 'BoxNew' [D:E:%d.2 'BoxNew']
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:                ClassProperty
// CHECK-NEXT:                    Id 'v'
// CHECK-NEXT:                MethodDefinition
// CHECK-NEXT:                    Id 'constructor'
// CHECK-NEXT:                    FunctionExpression
// CHECK-NEXT:                        Id 'v' [D:E:%d.7 'v']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            ExpressionStatement
// CHECK-NEXT:                                AssignmentExpression
// CHECK-NEXT:                                    MemberExpression
// CHECK-NEXT:                                        ThisExpression
// CHECK-NEXT:                                        Id 'v'
// CHECK-NEXT:                                    Id 'v' [D:E:%d.7 'v']
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            NewExpression : %class.4
// CHECK-NEXT:                Id 'BoxNew' [D:E:%d.5 'BoxNew'] : %class_constructor.5
// CHECK-NEXT:                TypeParameterInstantiation
// CHECK-NEXT:                NumericLiteral : 1
// CHECK-NEXT:        ClassDeclaration Scope %s.3
// CHECK-NEXT:            Id 'C' [D:E:%d.3 'C']
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:                MethodDefinition : %function.7
// CHECK-NEXT:                    Id 'm'
// CHECK-NEXT:                    FunctionExpression : %function.7
// CHECK-NEXT:                        Id 'x' [D:E:%d.13 'x']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                    Decorator
// CHECK-NEXT:                        MemberExpression
// CHECK-NEXT:                            Id 'Hermes'
// CHECK-NEXT:                            Id 'final'
// CHECK-NEXT:                MethodDefinition : generic
// CHECK-NEXT:                    Id 'm'
// CHECK-NEXT:                    FunctionExpression : generic
// CHECK-NEXT:                        Id 'x' [D:E:%d.9 'x']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                    Decorator
// CHECK-NEXT:                        MemberExpression
// CHECK-NEXT:                            Id 'Hermes'
// CHECK-NEXT:                            Id 'final'
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            CallExpression : void
// CHECK-NEXT:                MemberExpression : %function.7
// CHECK-NEXT:                    NewExpression : %class.2
// CHECK-NEXT:                        Id 'C' [D:E:%d.3 'C'] : %class_constructor.3
// CHECK-NEXT:                    Id 'm' [D:E:%d.6 'm']
// CHECK-NEXT:                TypeParameterInstantiation
// CHECK-NEXT:                NumericLiteral : 1
