/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals -typed -dump-sema %s | %FileCheckOrRegen %s --match-full-lines

// Generic type parameter bounds that are satisfied by their type arguments.

class Base {}
class Derived extends Base {
  x: number = 1;
}

// Generic function with a bound, satisfied by a subclass.
function f<T: Base>(x: T): T {
  return x;
}
f<Derived>(new Derived());

// Generic class with a bound.
class Box<T: Base> {
  v: T;
  constructor(v: T) {
    this.v = v;
  }
}
new Box<Derived>(new Derived());

// Generic type alias with a bound (exercises the DeclareScopeTypes resolver).
type Alias<T: Base> = T;
let a: Alias<Derived> = new Derived();

// Bound referencing an earlier type parameter (sibling reference).
function g<T: Base, U: T>(x: T, y: U): void {}
g<Base, Derived>(new Base(), new Derived());

// Generic method with a bound.
class C {
  @Hermes.final
  m<T: Base>(x: T): T {
    return x;
  }
}
new C().m<Derived>(new Derived());

// Bound that is itself a generic type alias instantiation. The bound resolves
// through the alias to Base, so Derived satisfies it.
type Id<X> = X;
function h<T: Id<Base>>(x: T): T {
  return x;
}
h<Derived>(new Derived());

// Bound that is a multi-level generic type alias chain (Wrap -> Id -> Base).
// Each link must resolve down to Base.
type Wrap<Y> = Id<Y>;
type Chained<T: Wrap<Base>> = T;
let chained: Chained<Derived> = new Derived();

// Bounds containing nested generic class specializations are validated only
// after those specializations have initialized their superclass chains.
class NestedBox<T: Base> extends Base {}

type NestedAlias<T: NestedBox<NestedBox<Base>>> = T;

let nested:
  NestedAlias<NestedBox<NestedBox<Base>>>;

// A structural bound satisfied by a distinct but mutually recursive type.
// Checking RecB against the bound recurses back into RecA -> RecB, so this
// relies on the cycle check in canAFlowIntoB.
type RecA = {readonly next: RecA | null};
type RecB = {readonly next: RecB | null};

function bounded<T: RecA>(value: T): T {
  return value;
}
bounded<RecB>(({next: null}: RecB));

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%class.2 = class(Base {
// CHECK-NEXT:  %homeObject: %class.20
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.3 = class_constructor(%class.2)
// CHECK-NEXT:%class.4 = class(Derived extends %class.2 {
// CHECK-NEXT:  %homeObject: %class.21
// CHECK-NEXT:  x: number
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.5 = class_constructor(%class.4)
// CHECK-NEXT:%class.6 = class(C {
// CHECK-NEXT:  %homeObject: %class.22
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.7 = class_constructor(%class.6)
// CHECK-NEXT:%object.8 = object({
// CHECK-NEXT:  +next: %union.23
// CHECK-NEXT:})
// CHECK-NEXT:%class.9 = class(NestedBox extends %class.2 {
// CHECK-NEXT:  %homeObject: %class.24
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.10 = class_constructor(%class.25)
// CHECK-NEXT:%class_constructor.11 = class_constructor(%class.9)
// CHECK-NEXT:%function.12 = function(x: %class.4): %class.4
// CHECK-NEXT:%class.13 = class(Box {
// CHECK-NEXT:  %constructor: %function.15
// CHECK-NEXT:  %homeObject: %class.26
// CHECK-NEXT:  v: %class.4
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.14 = class_constructor(%class.13)
// CHECK-NEXT:%function.15 = function(this: %class.13, v: %class.4): void
// CHECK-NEXT:%function.16 = function(x: %class.2, y: %class.4): void
// CHECK-NEXT:%function.17 = function(this: %class.6, x: %class.4): %class.4
// CHECK-NEXT:%function.18 = function(value: %object.8): %object.8
// CHECK-NEXT:%object.19 = object({
// CHECK-NEXT:  next: %union.23
// CHECK-NEXT:})
// CHECK-NEXT:%class.20 = class( {
// CHECK-NEXT:})
// CHECK-NEXT:%class.21 = class( extends %class.20 {
// CHECK-NEXT:})
// CHECK-NEXT:%class.22 = class( {
// CHECK-NEXT:  m [final]: generic
// CHECK-NEXT:})
// CHECK-NEXT:%union.23 = union(null | %object.8)
// CHECK-NEXT:%class.24 = class( extends %class.20 {
// CHECK-NEXT:})
// CHECK-NEXT:%class.25 = class(NestedBox extends %class.2 {
// CHECK-NEXT:  %homeObject: %class.27
// CHECK-NEXT:})
// CHECK-NEXT:%class.26 = class( {
// CHECK-NEXT:})
// CHECK-NEXT:%class.27 = class( extends %class.20 {
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict mayReachImplicitReturn
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'Base' Class : %class_constructor.3
// CHECK-NEXT:        Decl %d.3 'Derived' Class : %class_constructor.5
// CHECK-NEXT:        Decl %d.4 'f' Var
// CHECK-NEXT:        Decl %d.5 'Box' Class
// CHECK-NEXT:        Decl %d.6 'a' Let : %class.4
// CHECK-NEXT:        Decl %d.7 'g' Var
// CHECK-NEXT:        Decl %d.8 'C' Class : %class_constructor.7
// CHECK-NEXT:        Decl %d.9 'h' Var
// CHECK-NEXT:        Decl %d.10 'chained' Let : %class.4
// CHECK-NEXT:        Decl %d.11 'NestedBox' Class
// CHECK-NEXT:        Decl %d.12 'nested' Let : %class.9
// CHECK-NEXT:        Decl %d.13 'bounded' Var
// CHECK-NEXT:        Decl %d.14 'arguments' Var Arguments
// CHECK-NEXT:        Decl %d.15 'NestedBox' Class : %class_constructor.10
// CHECK-NEXT:        Decl %d.16 'NestedBox' Class : %class_constructor.11
// CHECK-NEXT:        Decl %d.17 'f' Var : %function.12
// CHECK-NEXT:        Decl %d.18 'Box' Class : %class_constructor.14
// CHECK-NEXT:        Decl %d.19 'g' Var : %function.16
// CHECK-NEXT:        Decl %d.20 'h' Var : %function.12
// CHECK-NEXT:        Decl %d.21 'bounded' Var : %function.18
// CHECK-NEXT:        hoistedFunction f
// CHECK-NEXT:        hoistedFunction g
// CHECK-NEXT:        hoistedFunction h
// CHECK-NEXT:        hoistedFunction bounded
// CHECK-NEXT:        hoistedFunction f
// CHECK-NEXT:        hoistedFunction g
// CHECK-NEXT:        hoistedFunction h
// CHECK-NEXT:        hoistedFunction bounded
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:        Scope %s.3
// CHECK-NEXT:        Scope %s.4
// CHECK-NEXT:        Scope %s.5
// CHECK-NEXT:            Decl %d.22 'm' Const
// CHECK-NEXT:        Scope %s.6
// CHECK-NEXT:        Scope %s.7
// CHECK-NEXT:        Scope %s.8
// CHECK-NEXT:        Scope %s.9
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.10
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.11
// CHECK-NEXT:            Decl %d.23 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.12
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.13
// CHECK-NEXT:            Decl %d.24 'x' Parameter
// CHECK-NEXT:            Decl %d.25 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.14
// CHECK-NEXT:            Decl %d.26 'v' Parameter
// CHECK-NEXT:            Decl %d.27 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.15
// CHECK-NEXT:            Decl %d.28 'x' Parameter
// CHECK-NEXT:            Decl %d.29 'y' Parameter
// CHECK-NEXT:            Decl %d.30 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.16
// CHECK-NEXT:            Decl %d.31 'x' Parameter
// CHECK-NEXT:            Decl %d.32 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.17
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.18
// CHECK-NEXT:            Decl %d.33 'x' Parameter
// CHECK-NEXT:            Decl %d.34 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.19
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.20
// CHECK-NEXT:            Decl %d.35 'value' Parameter
// CHECK-NEXT:            Decl %d.36 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.21
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.22
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.23
// CHECK-NEXT:            Decl %d.37 'x' Parameter : %class.4
// CHECK-NEXT:            Decl %d.38 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.24
// CHECK-NEXT:            Decl %d.39 'v' Parameter : %class.4
// CHECK-NEXT:            Decl %d.40 'arguments' Var Arguments
// CHECK-NEXT:    Func strict mayReachImplicitReturn
// CHECK-NEXT:        Scope %s.25
// CHECK-NEXT:            Decl %d.41 'x' Parameter : %class.2
// CHECK-NEXT:            Decl %d.42 'y' Parameter : %class.4
// CHECK-NEXT:            Decl %d.43 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.26
// CHECK-NEXT:            Decl %d.44 'x' Parameter : %class.4
// CHECK-NEXT:            Decl %d.45 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.27
// CHECK-NEXT:            Decl %d.46 'x' Parameter : %class.4
// CHECK-NEXT:            Decl %d.47 'arguments' Var Arguments
// CHECK-NEXT:    Func strict noImplicitReturn
// CHECK-NEXT:        Scope %s.28
// CHECK-NEXT:            Decl %d.48 'value' Parameter : %object.8
// CHECK-NEXT:            Decl %d.49 'arguments' Var Arguments

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        ClassDeclaration Scope %s.2
// CHECK-NEXT:            Id 'Base' [D:E:%d.2 'Base']
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:        ClassDeclaration Scope %s.3
// CHECK-NEXT:            Id 'Derived' [D:E:%d.3 'Derived']
// CHECK-NEXT:            Id 'Base' [D:E:%d.2 'Base'] : %class_constructor.3
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:                ClassProperty : number
// CHECK-NEXT:                    Id 'x'
// CHECK-NEXT:                    NumericLiteral : 1
// CHECK-NEXT:        FunctionDeclaration : %function.12
// CHECK-NEXT:            Id 'f' [D:E:%d.17 'f']
// CHECK-NEXT:            Id 'x' [D:E:%d.37 'x']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    Id 'x' [D:E:%d.37 'x'] : %class.4
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:        FunctionDeclaration
// CHECK-NEXT:            Id 'f' [D:E:%d.4 'f']
// CHECK-NEXT:            Id 'x' [D:E:%d.24 'x']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    Id 'x' [D:E:%d.24 'x']
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            CallExpression : %class.4
// CHECK-NEXT:                Id 'f' [D:E:%d.17 'f'] : %function.12
// CHECK-NEXT:                TypeParameterInstantiation
// CHECK-NEXT:                    GenericTypeAnnotation
// CHECK-NEXT:                        Id 'Derived'
// CHECK-NEXT:                NewExpression : %class.4
// CHECK-NEXT:                    Id 'Derived' [D:E:%d.3 'Derived'] : %class_constructor.5
// CHECK-NEXT:        ClassDeclaration Scope %s.9
// CHECK-NEXT:            Id 'Box' [D:E:%d.18 'Box']
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:                ClassProperty : %class.4
// CHECK-NEXT:                    Id 'v'
// CHECK-NEXT:                MethodDefinition : %function.15
// CHECK-NEXT:                    Id 'constructor'
// CHECK-NEXT:                    FunctionExpression : %function.15
// CHECK-NEXT:                        Id 'v' [D:E:%d.39 'v']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            ExpressionStatement
// CHECK-NEXT:                                AssignmentExpression : %class.4
// CHECK-NEXT:                                    MemberExpression : %class.4
// CHECK-NEXT:                                        ThisExpression : %class.13
// CHECK-NEXT:                                        Id 'v'
// CHECK-NEXT:                                    Id 'v' [D:E:%d.39 'v'] : %class.4
// CHECK-NEXT:        ClassDeclaration Scope %s.4
// CHECK-NEXT:            Id 'Box' [D:E:%d.5 'Box']
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:                ClassProperty
// CHECK-NEXT:                    Id 'v'
// CHECK-NEXT:                MethodDefinition
// CHECK-NEXT:                    Id 'constructor'
// CHECK-NEXT:                    FunctionExpression
// CHECK-NEXT:                        Id 'v' [D:E:%d.26 'v']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            ExpressionStatement
// CHECK-NEXT:                                AssignmentExpression
// CHECK-NEXT:                                    MemberExpression
// CHECK-NEXT:                                        ThisExpression
// CHECK-NEXT:                                        Id 'v'
// CHECK-NEXT:                                    Id 'v' [D:E:%d.26 'v']
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            NewExpression : %class.13
// CHECK-NEXT:                Id 'Box' [D:E:%d.18 'Box'] : %class_constructor.14
// CHECK-NEXT:                TypeParameterInstantiation
// CHECK-NEXT:                    GenericTypeAnnotation
// CHECK-NEXT:                        Id 'Derived'
// CHECK-NEXT:                NewExpression : %class.4
// CHECK-NEXT:                    Id 'Derived' [D:E:%d.3 'Derived'] : %class_constructor.5
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'Alias'
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:            GenericTypeAnnotation
// CHECK-NEXT:                Id 'T'
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                NewExpression : %class.4
// CHECK-NEXT:                    Id 'Derived' [D:E:%d.3 'Derived'] : %class_constructor.5
// CHECK-NEXT:                Id 'a' [D:E:%d.6 'a']
// CHECK-NEXT:        FunctionDeclaration : %function.16
// CHECK-NEXT:            Id 'g' [D:E:%d.19 'g']
// CHECK-NEXT:            Id 'x' [D:E:%d.41 'x']
// CHECK-NEXT:            Id 'y' [D:E:%d.42 'y']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:        FunctionDeclaration
// CHECK-NEXT:            Id 'g' [D:E:%d.7 'g']
// CHECK-NEXT:            Id 'x' [D:E:%d.28 'x']
// CHECK-NEXT:            Id 'y' [D:E:%d.29 'y']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            CallExpression : void
// CHECK-NEXT:                Id 'g' [D:E:%d.19 'g'] : %function.16
// CHECK-NEXT:                TypeParameterInstantiation
// CHECK-NEXT:                    GenericTypeAnnotation
// CHECK-NEXT:                        Id 'Base'
// CHECK-NEXT:                    GenericTypeAnnotation
// CHECK-NEXT:                        Id 'Derived'
// CHECK-NEXT:                NewExpression : %class.2
// CHECK-NEXT:                    Id 'Base' [D:E:%d.2 'Base'] : %class_constructor.3
// CHECK-NEXT:                NewExpression : %class.4
// CHECK-NEXT:                    Id 'Derived' [D:E:%d.3 'Derived'] : %class_constructor.5
// CHECK-NEXT:        ClassDeclaration Scope %s.5
// CHECK-NEXT:            Id 'C' [D:E:%d.8 'C']
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:                MethodDefinition : %function.17
// CHECK-NEXT:                    Id 'm'
// CHECK-NEXT:                    FunctionExpression : %function.17
// CHECK-NEXT:                        Id 'x' [D:E:%d.44 'x']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            ReturnStatement
// CHECK-NEXT:                                Id 'x' [D:E:%d.44 'x'] : %class.4
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                    Decorator
// CHECK-NEXT:                        MemberExpression
// CHECK-NEXT:                            Id 'Hermes'
// CHECK-NEXT:                            Id 'final'
// CHECK-NEXT:                MethodDefinition : generic
// CHECK-NEXT:                    Id 'm'
// CHECK-NEXT:                    FunctionExpression : generic
// CHECK-NEXT:                        Id 'x' [D:E:%d.31 'x']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            ReturnStatement
// CHECK-NEXT:                                Id 'x' [D:E:%d.31 'x']
// CHECK-NEXT:                        TypeParameterDeclaration
// CHECK-NEXT:                            TypeParameter
// CHECK-NEXT:                    Decorator
// CHECK-NEXT:                        MemberExpression
// CHECK-NEXT:                            Id 'Hermes'
// CHECK-NEXT:                            Id 'final'
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            CallExpression : %class.4
// CHECK-NEXT:                MemberExpression : %function.17
// CHECK-NEXT:                    NewExpression : %class.6
// CHECK-NEXT:                        Id 'C' [D:E:%d.8 'C'] : %class_constructor.7
// CHECK-NEXT:                    Id 'm' [D:E:%d.22 'm']
// CHECK-NEXT:                TypeParameterInstantiation
// CHECK-NEXT:                    GenericTypeAnnotation
// CHECK-NEXT:                        Id 'Derived'
// CHECK-NEXT:                NewExpression : %class.4
// CHECK-NEXT:                    Id 'Derived' [D:E:%d.3 'Derived'] : %class_constructor.5
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'Id'
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:            GenericTypeAnnotation
// CHECK-NEXT:                Id 'X'
// CHECK-NEXT:        FunctionDeclaration : %function.12
// CHECK-NEXT:            Id 'h' [D:E:%d.20 'h']
// CHECK-NEXT:            Id 'x' [D:E:%d.46 'x']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    Id 'x' [D:E:%d.46 'x'] : %class.4
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:        FunctionDeclaration
// CHECK-NEXT:            Id 'h' [D:E:%d.9 'h']
// CHECK-NEXT:            Id 'x' [D:E:%d.33 'x']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    Id 'x' [D:E:%d.33 'x']
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            CallExpression : %class.4
// CHECK-NEXT:                Id 'h' [D:E:%d.20 'h'] : %function.12
// CHECK-NEXT:                TypeParameterInstantiation
// CHECK-NEXT:                    GenericTypeAnnotation
// CHECK-NEXT:                        Id 'Derived'
// CHECK-NEXT:                NewExpression : %class.4
// CHECK-NEXT:                    Id 'Derived' [D:E:%d.3 'Derived'] : %class_constructor.5
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'Wrap'
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:            GenericTypeAnnotation
// CHECK-NEXT:                Id 'Id'
// CHECK-NEXT:                TypeParameterInstantiation
// CHECK-NEXT:                    GenericTypeAnnotation
// CHECK-NEXT:                        Id 'Y'
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'Chained'
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:            GenericTypeAnnotation
// CHECK-NEXT:                Id 'T'
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                NewExpression : %class.4
// CHECK-NEXT:                    Id 'Derived' [D:E:%d.3 'Derived'] : %class_constructor.5
// CHECK-NEXT:                Id 'chained' [D:E:%d.10 'chained']
// CHECK-NEXT:        ClassDeclaration Scope %s.7
// CHECK-NEXT:            Id 'NestedBox' [D:E:%d.15 'NestedBox']
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:            Id 'Base' [D:E:%d.2 'Base'] : %class_constructor.3
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:        ClassDeclaration Scope %s.8
// CHECK-NEXT:            Id 'NestedBox' [D:E:%d.16 'NestedBox']
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:            Id 'Base' [D:E:%d.2 'Base'] : %class_constructor.3
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:        ClassDeclaration Scope %s.6
// CHECK-NEXT:            Id 'NestedBox' [D:E:%d.11 'NestedBox']
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:            Id 'Base' [D:E:%d.2 'Base']
// CHECK-NEXT:            ClassBody
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'NestedAlias'
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:            GenericTypeAnnotation
// CHECK-NEXT:                Id 'T'
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                Id 'nested' [D:E:%d.12 'nested']
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'RecA'
// CHECK-NEXT:            ObjectTypeAnnotation
// CHECK-NEXT:                ObjectTypeProperty
// CHECK-NEXT:                    Id 'next'
// CHECK-NEXT:                    UnionTypeAnnotation
// CHECK-NEXT:                        GenericTypeAnnotation
// CHECK-NEXT:                            Id 'RecA'
// CHECK-NEXT:                        NullLiteralTypeAnnotation
// CHECK-NEXT:                    Variance
// CHECK-NEXT:        TypeAlias
// CHECK-NEXT:            Id 'RecB'
// CHECK-NEXT:            ObjectTypeAnnotation
// CHECK-NEXT:                ObjectTypeProperty
// CHECK-NEXT:                    Id 'next'
// CHECK-NEXT:                    UnionTypeAnnotation
// CHECK-NEXT:                        GenericTypeAnnotation
// CHECK-NEXT:                            Id 'RecB'
// CHECK-NEXT:                        NullLiteralTypeAnnotation
// CHECK-NEXT:                    Variance
// CHECK-NEXT:        FunctionDeclaration : %function.18
// CHECK-NEXT:            Id 'bounded' [D:E:%d.21 'bounded']
// CHECK-NEXT:            Id 'value' [D:E:%d.48 'value']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    Id 'value' [D:E:%d.48 'value'] : %object.8
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:        FunctionDeclaration
// CHECK-NEXT:            Id 'bounded' [D:E:%d.13 'bounded']
// CHECK-NEXT:            Id 'value' [D:E:%d.35 'value']
// CHECK-NEXT:            BlockStatement
// CHECK-NEXT:                ReturnStatement
// CHECK-NEXT:                    Id 'value' [D:E:%d.35 'value']
// CHECK-NEXT:            TypeParameterDeclaration
// CHECK-NEXT:                TypeParameter
// CHECK-NEXT:        ExpressionStatement
// CHECK-NEXT:            CallExpression : %object.8
// CHECK-NEXT:                Id 'bounded' [D:E:%d.21 'bounded'] : %function.18
// CHECK-NEXT:                TypeParameterInstantiation
// CHECK-NEXT:                    GenericTypeAnnotation
// CHECK-NEXT:                        Id 'RecB'
// CHECK-NEXT:                TypeCastExpression : %object.8
// CHECK-NEXT:                    ObjectExpression : %object.19
// CHECK-NEXT:                        Property
// CHECK-NEXT:                            Id 'next'
// CHECK-NEXT:                            NullLiteral : null
