/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -fno-std-globals -typed -dump-sema %s | %FileCheckOrRegen %s --match-full-lines

// Structurally identical types are interned to a single TypeInfo, which the
// `a == b` fast path answers before any cycle bookkeeping. Every pair below is
// therefore kept distinct by a nominal field, so the recursion is real.
class C {}
class D extends C {}

type A = {readonly next: A | null, readonly v: D};
type B = {readonly next: B | null, readonly v: C};

// The cycle assumption is what makes this work: checking `next` recurses back
// into A -> B. Splitting the cycle key by checked-cast mode must not break it.
function convertRead(value: A): B {
  return value;
}

// Contravariant fields invert the direction, so the recursion has to run
// through a covariant `next` for the two sides to remain distinct.
type WriteA = {readonly next: WriteA | null, -v: C};
type WriteB = {readonly next: WriteB | null, -v: D};

function convertWrite(value: WriteA): WriteB {
  return value;
}

// Same, with the nominal difference behind a function type, so the cycle is
// closed across the BaseFunctionType overload's return type check.
type FnHolderA = {readonly next: FnHolderA | null, readonly fn: () => D};
type FnHolderB = {readonly next: FnHolderB | null, readonly fn: () => C};

function convertFunction(value: FnHolderA): FnHolderB {
  return value;
}

class Base {
  method(value: A): B {
    return value;
  }
}

// The override must be accepted: the parameter check is Base's A -> Derived's
// B, which is the recursive relation above, so the cycle assumption has to
// resolve it positively.
class Derived extends Base {
  method(value: B): B {
    return value;
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%class_constructor.2 = class_constructor(%class.15)
// CHECK-NEXT:%class_constructor.3 = class_constructor(%class.16)
// CHECK-NEXT:%object.4 = object({
// CHECK-NEXT:  +next: %union.17
// CHECK-NEXT:  +v: %class.16
// CHECK-NEXT:})
// CHECK-NEXT:%object.5 = object({
// CHECK-NEXT:  +next: %union.18
// CHECK-NEXT:  +v: %class.15
// CHECK-NEXT:})
// CHECK-NEXT:%object.6 = object({
// CHECK-NEXT:  +next: %union.19
// CHECK-NEXT:  -v: %class.15
// CHECK-NEXT:})
// CHECK-NEXT:%object.7 = object({
// CHECK-NEXT:  +next: %union.20
// CHECK-NEXT:  +fn: %function.21
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.8 = class_constructor(%class.22)
// CHECK-NEXT:%class_constructor.9 = class_constructor(%class.23)
// CHECK-NEXT:%function.10 = function(this: %class.22, value: %object.4): %object.5
// CHECK-NEXT:%function.11 = function(this: %class.23, value: %object.5): %object.5
// CHECK-NEXT:%function.12 = function(value: %object.4): %object.5
// CHECK-NEXT:%function.13 = function(value: %object.6): %object.24
// CHECK-NEXT:%function.14 = function(value: %object.7): %object.25
// CHECK-NEXT:%class.15 = class(C {
// CHECK-NEXT:  %homeObject: %class.26
// CHECK-NEXT:})
// CHECK-NEXT:%class.16 = class(D extends %class.15 {
// CHECK-NEXT:  %homeObject: %class.27
// CHECK-NEXT:})
// CHECK-NEXT:%union.17 = union(null | %object.4)
// CHECK-NEXT:%union.18 = union(null | %object.5)
// CHECK-NEXT:%union.19 = union(null | %object.6)
// CHECK-NEXT:%union.20 = union(null | %object.7)
// CHECK-NEXT:%function.21 = function(): %class.16
// CHECK-NEXT:%class.22 = class(Base {
// CHECK-NEXT:  %homeObject: %class.28
// CHECK-NEXT:})
// CHECK-NEXT:%class.23 = class(Derived extends %class.22 {
// CHECK-NEXT:  %homeObject: %class.29
// CHECK-NEXT:})
// CHECK-NEXT:%object.24 = object({
// CHECK-NEXT:  +next: %union.30
// CHECK-NEXT:  -v: %class.16
// CHECK-NEXT:})
// CHECK-NEXT:%object.25 = object({
// CHECK-NEXT:  +next: %union.31
// CHECK-NEXT:  +fn: %function.32
// CHECK-NEXT:})
// CHECK-NEXT:%class.26 = class( {
// CHECK-NEXT:})
// CHECK-NEXT:%class.27 = class( extends %class.26 {
// CHECK-NEXT:})
// CHECK-NEXT:%class.28 = class( {
// CHECK-NEXT:  method [overridden]: %function.10
// CHECK-NEXT:})
// CHECK-NEXT:%class.29 = class( extends %class.28 {
// CHECK-NEXT:  method [final]: %function.11
// CHECK-NEXT:})
// CHECK-NEXT:%union.30 = union(null | %object.24)
// CHECK-NEXT:%union.31 = union(null | %object.25)
// CHECK-NEXT:%function.32 = function(): %class.15

// CHECK:SemContext
// CHECK-NEXT:Func strict mayReachImplicitReturn
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'C' Class : %class_constructor.2
// CHECK-NEXT:        Decl %d.3 'D' Class : %class_constructor.3
// CHECK-NEXT:        Decl %d.4 'convertRead' Var : %function.12
// CHECK-NEXT:        Decl %d.5 'convertWrite' Var : %function.13
// CHECK-NEXT:        Decl %d.6 'convertFunction' Var : %function.14
// CHECK-NEXT:        Decl %d.7 'Base' Class : %class_constructor.8
// CHECK-NEXT:        Decl %d.8 'Derived' Class : %class_constructor.9
// CHECK-NEXT:        Decl %d.9 'arguments' Var Arguments
// CHECK-NEXT:        hoistedFunction convertRead
// CHECK-NEXT:        hoistedFunction convertWrite
// CHECK-NEXT:        hoistedFunction convertFunction
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:        Scope %s.3
// CHECK-NEXT:        Scope %s.4
// CHECK-NEXT:        Scope %s.5
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.6
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.7
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.8
// CHECK-NEXT:            Decl %d.10 'value' Parameter : %object.4
// CHECK-NEXT:            Decl %d.11 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.9
// CHECK-NEXT:            Decl %d.12 'value' Parameter : %object.6
// CHECK-NEXT:            Decl %d.13 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.10
// CHECK-NEXT:            Decl %d.14 'value' Parameter : %object.7
// CHECK-NEXT:            Decl %d.15 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.11
// CHECK-NEXT:            Decl %d.16 'value' Parameter : %object.4
// CHECK-NEXT:            Decl %d.17 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.12
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.13
// CHECK-NEXT:            Decl %d.18 'value' Parameter : %object.5
// CHECK-NEXT:            Decl %d.19 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.14

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        ClassDeclaration Scope %s.2
// CHECK-NEXT:            Id 'C' [D:E:%d.2 'C']
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:        ClassDeclaration Scope %s.3
// CHECK-NEXT:            Id 'D' [D:E:%d.3 'D']
// CHECK-NEXT:            Id 'C' [D:E:%d.2 'C'] : %class_constructor.2
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'A'
// CHECK-NEXT:            ObjectTypeAnnotation
// CHECK-NEXT:                ObjectTypeProperty
// CHECK-NEXT:                    Id 'next'
// CHECK-NEXT:                    UnionTypeAnnotation
// CHECK-NEXT:                        GenericTypeAnnotation
// CHECK-NEXT:                            Id 'A'
// CHECK-NEXT:                        NullLiteralTypeAnnotation
// CHECK-NEXT:                    Variance
// CHECK-NEXT:                ObjectTypeProperty
// CHECK-NEXT:                    Id 'v'
// CHECK-NEXT:                    GenericTypeAnnotation
// CHECK-NEXT:                        Id 'D'
// CHECK-NEXT:                    Variance
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'B'
// CHECK-NEXT:            ObjectTypeAnnotation
// CHECK-NEXT:                ObjectTypeProperty
// CHECK-NEXT:                    Id 'next'
// CHECK-NEXT:                    UnionTypeAnnotation
// CHECK-NEXT:                        GenericTypeAnnotation
// CHECK-NEXT:                            Id 'B'
// CHECK-NEXT:                        NullLiteralTypeAnnotation
// CHECK-NEXT:                    Variance
// CHECK-NEXT:                ObjectTypeProperty
// CHECK-NEXT:                    Id 'v'
// CHECK-NEXT:                    GenericTypeAnnotation
// CHECK-NEXT:                        Id 'C'
// CHECK-NEXT:                    Variance
// CHECK-NEXT:        FunctionDeclaration : %function.12
// CHECK-NEXT:            Id 'convertRead' [D:E:%d.4 'convertRead']
// CHECK-NEXT:            Id 'value' [D:E:%d.10 'value']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    Id 'value' [D:E:%d.10 'value'] : %object.4
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'WriteA'
// CHECK-NEXT:            ObjectTypeAnnotation
// CHECK-NEXT:                ObjectTypeProperty
// CHECK-NEXT:                    Id 'next'
// CHECK-NEXT:                    UnionTypeAnnotation
// CHECK-NEXT:                        GenericTypeAnnotation
// CHECK-NEXT:                            Id 'WriteA'
// CHECK-NEXT:                        NullLiteralTypeAnnotation
// CHECK-NEXT:                    Variance
// CHECK-NEXT:                ObjectTypeProperty
// CHECK-NEXT:                    Id 'v'
// CHECK-NEXT:                    GenericTypeAnnotation
// CHECK-NEXT:                        Id 'C'
// CHECK-NEXT:                    Variance
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'WriteB'
// CHECK-NEXT:            ObjectTypeAnnotation
// CHECK-NEXT:                ObjectTypeProperty
// CHECK-NEXT:                    Id 'next'
// CHECK-NEXT:                    UnionTypeAnnotation
// CHECK-NEXT:                        GenericTypeAnnotation
// CHECK-NEXT:                            Id 'WriteB'
// CHECK-NEXT:                        NullLiteralTypeAnnotation
// CHECK-NEXT:                    Variance
// CHECK-NEXT:                ObjectTypeProperty
// CHECK-NEXT:                    Id 'v'
// CHECK-NEXT:                    GenericTypeAnnotation
// CHECK-NEXT:                        Id 'D'
// CHECK-NEXT:                    Variance
// CHECK-NEXT:        FunctionDeclaration : %function.13
// CHECK-NEXT:            Id 'convertWrite' [D:E:%d.5 'convertWrite']
// CHECK-NEXT:            Id 'value' [D:E:%d.12 'value']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    Id 'value' [D:E:%d.12 'value'] : %object.6
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'FnHolderA'
// CHECK-NEXT:            ObjectTypeAnnotation
// CHECK-NEXT:                ObjectTypeProperty
// CHECK-NEXT:                    Id 'next'
// CHECK-NEXT:                    UnionTypeAnnotation
// CHECK-NEXT:                        GenericTypeAnnotation
// CHECK-NEXT:                            Id 'FnHolderA'
// CHECK-NEXT:                        NullLiteralTypeAnnotation
// CHECK-NEXT:                    Variance
// CHECK-NEXT:                ObjectTypeProperty
// CHECK-NEXT:                    Id 'fn'
// CHECK-NEXT:                    FunctionTypeAnnotation
// CHECK-NEXT:                        GenericTypeAnnotation
// CHECK-NEXT:                            Id 'D'
// CHECK-NEXT:                    Variance
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'FnHolderB'
// CHECK-NEXT:            ObjectTypeAnnotation
// CHECK-NEXT:                ObjectTypeProperty
// CHECK-NEXT:                    Id 'next'
// CHECK-NEXT:                    UnionTypeAnnotation
// CHECK-NEXT:                        GenericTypeAnnotation
// CHECK-NEXT:                            Id 'FnHolderB'
// CHECK-NEXT:                        NullLiteralTypeAnnotation
// CHECK-NEXT:                    Variance
// CHECK-NEXT:                ObjectTypeProperty
// CHECK-NEXT:                    Id 'fn'
// CHECK-NEXT:                    FunctionTypeAnnotation
// CHECK-NEXT:                        GenericTypeAnnotation
// CHECK-NEXT:                            Id 'C'
// CHECK-NEXT:                    Variance
// CHECK-NEXT:        FunctionDeclaration : %function.14
// CHECK-NEXT:            Id 'convertFunction' [D:E:%d.6 'convertFunction']
// CHECK-NEXT:            Id 'value' [D:E:%d.14 'value']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    Id 'value' [D:E:%d.14 'value'] : %object.7
// CHECK-NEXT:        ClassDeclaration Scope %s.4
// CHECK-NEXT:            Id 'Base' [D:E:%d.7 'Base']
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:                MethodDefinition : %function.10
// CHECK-NEXT:                    Id 'method'
// CHECK-NEXT:                    FunctionExpression : %function.10
// CHECK-NEXT:                        Id 'value' [D:E:%d.16 'value']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            ReturnStatement
// CHECK-NEXT:                                Id 'value' [D:E:%d.16 'value'] : %object.4
// CHECK-NEXT:        ClassDeclaration Scope %s.5
// CHECK-NEXT:            Id 'Derived' [D:E:%d.8 'Derived']
// CHECK-NEXT:            Id 'Base' [D:E:%d.7 'Base'] : %class_constructor.8
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:                MethodDefinition : %function.11
// CHECK-NEXT:                    Id 'method'
// CHECK-NEXT:                    FunctionExpression : %function.11
// CHECK-NEXT:                        Id 'value' [D:E:%d.18 'value']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            ReturnStatement
// CHECK-NEXT:                                Id 'value' [D:E:%d.18 'value'] : %object.5
