/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -typed -dump-sema %s | %FileCheckOrRegen --match-full-lines %s

// All type parameters are bound before bounds are resolved, so a bound may
// reference its own or a later parameter.

class Base {}

// Self-reference: the bound names the parameter it belongs to.
function selfRef<T: T>(x: T): void {}
selfRef<Base>(new Base());

// Forward reference: the bound names a later parameter.
function fwdRef<T: U, U>(x: T, y: U): void {}
fwdRef<Base, Base>(new Base(), new Base());

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%class.2 = class(Base {
// CHECK-NEXT:  %homeObject: %class.6
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.3 = class_constructor(%class.2)
// CHECK-NEXT:%function.4 = function(x: %class.2): void
// CHECK-NEXT:%function.5 = function(x: %class.2, y: %class.2): void
// CHECK-NEXT:%class.6 = class( {
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict mayReachImplicitReturn
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'Base' Class : %class_constructor.3
// CHECK-NEXT:        Decl %d.3 'selfRef' Var
// CHECK-NEXT:        Decl %d.4 'fwdRef' Var
// CHECK-NEXT:        Decl %d.5 'arguments' Var Arguments
// CHECK-NEXT:        Decl %d.6 'selfRef' Var : %function.4
// CHECK-NEXT:        Decl %d.7 'fwdRef' Var : %function.5
// CHECK-NEXT:        hoistedFunction selfRef
// CHECK-NEXT:        hoistedFunction fwdRef
// CHECK-NEXT:        hoistedFunction selfRef
// CHECK-NEXT:        hoistedFunction fwdRef
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.3
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.4
// CHECK-NEXT:            Decl %d.8 'x' Parameter
// CHECK-NEXT:            Decl %d.9 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.5
// CHECK-NEXT:            Decl %d.10 'x' Parameter
// CHECK-NEXT:            Decl %d.11 'y' Parameter
// CHECK-NEXT:            Decl %d.12 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.6
// CHECK-NEXT:            Decl %d.13 'x' Parameter : %class.2
// CHECK-NEXT:            Decl %d.14 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.7
// CHECK-NEXT:            Decl %d.15 'x' Parameter : %class.2
// CHECK-NEXT:            Decl %d.16 'y' Parameter : %class.2
// CHECK-NEXT:            Decl %d.17 'arguments' Var Arguments

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        ClassDeclaration Scope %s.2
// CHECK-NEXT:            Id 'Base' [D:E:%d.2 'Base']
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:        FunctionDeclaration : %function.4
// CHECK-NEXT:            Id 'selfRef' [D:E:%d.6 'selfRef']
// CHECK-NEXT:            Id 'x' [D:E:%d.13 'x']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:        FunctionDeclaration
// CHECK-NEXT:            Id 'selfRef' [D:E:%d.3 'selfRef']
// CHECK-NEXT:            Id 'x' [D:E:%d.8 'x']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            CallExpression : void
// CHECK-NEXT:                Id 'selfRef' [D:E:%d.6 'selfRef'] : %function.4
// CHECK-NEXT:                TypeParameterInstantiation
// CHECK-NEXT:                    GenericTypeAnnotation
// CHECK-NEXT:                        Id 'Base'
// CHECK-NEXT:                NewExpression : %class.2
// CHECK-NEXT:                    Id 'Base' [D:E:%d.2 'Base'] : %class_constructor.3
// CHECK-NEXT:        FunctionDeclaration : %function.5
// CHECK-NEXT:            Id 'fwdRef' [D:E:%d.7 'fwdRef']
// CHECK-NEXT:            Id 'x' [D:E:%d.15 'x']
// CHECK-NEXT:            Id 'y' [D:E:%d.16 'y']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:        FunctionDeclaration
// CHECK-NEXT:            Id 'fwdRef' [D:E:%d.4 'fwdRef']
// CHECK-NEXT:            Id 'x' [D:E:%d.10 'x']
// CHECK-NEXT:            Id 'y' [D:E:%d.11 'y']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            CallExpression : void
// CHECK-NEXT:                Id 'fwdRef' [D:E:%d.7 'fwdRef'] : %function.5
// CHECK-NEXT:                TypeParameterInstantiation
// CHECK-NEXT:                    GenericTypeAnnotation
// CHECK-NEXT:                        Id 'Base'
// CHECK-NEXT:                    GenericTypeAnnotation
// CHECK-NEXT:                        Id 'Base'
// CHECK-NEXT:                NewExpression : %class.2
// CHECK-NEXT:                    Id 'Base' [D:E:%d.2 'Base'] : %class_constructor.3
// CHECK-NEXT:                NewExpression : %class.2
// CHECK-NEXT:                    Id 'Base' [D:E:%d.2 'Base'] : %class_constructor.3
