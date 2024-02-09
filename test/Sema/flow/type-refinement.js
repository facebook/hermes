/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror --typed --dump-sema -fno-std-globals %s | %FileCheckOrRegen %s --match-full-lines

class C {
  next: C | null;
}

function loop(cur: C | null): C | null {
  while (cur)
    cur = cur.next;
  return cur;
}

function call(cur: C | null): void {
  function bar(c: C, cv: C | void): void {}
  if (cur) {
    // Observe the ImplicitCheckedCast to C instead of C|void for cv.
    bar(cur, cur);
  }
}

function assign(cur: C | null): void {
  let a: C | null = cur;
  let c: C;
  if (cur) {
    let b: C = cur;
    c = a;
  }
}

function retval(c: C | null): C {
  if (c) return c;
  else return new C();
}

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%class.2 = class(C {
// CHECK-NEXT:  %homeObject: %class.3
// CHECK-NEXT:  next: %union.4
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.5 = class_constructor(%class.2)
// CHECK-NEXT:%union.4 = union(null | %class.2)
// CHECK-NEXT:%class.3 = class( {
// CHECK-NEXT:})
// CHECK-NEXT:%function.6 = function(cur: %union.4): %union.4
// CHECK-NEXT:%function.7 = function(cur: %union.4): void
// CHECK-NEXT:%function.8 = function(c: %union.4): %class.2
// CHECK-NEXT:%union.9 = union(void | %class.2)
// CHECK-NEXT:%function.10 = function(c: %class.2, cv: %union.9): void

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.2 'C' Class : %class_constructor.5
// CHECK-NEXT:            Decl %d.3 'loop' ScopedFunction : %function.6
// CHECK-NEXT:            Decl %d.4 'call' ScopedFunction : %function.7
// CHECK-NEXT:            Decl %d.5 'assign' ScopedFunction : %function.7
// CHECK-NEXT:            Decl %d.6 'retval' ScopedFunction : %function.8
// CHECK-NEXT:            Decl %d.7 'arguments' Var Arguments
// CHECK-NEXT:            hoistedFunction loop
// CHECK-NEXT:            hoistedFunction call
// CHECK-NEXT:            hoistedFunction assign
// CHECK-NEXT:            hoistedFunction retval
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:                Decl %d.8 'cur' Parameter : %union.4
// CHECK-NEXT:                Decl %d.9 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.4
// CHECK-NEXT:                Decl %d.10 'cur' Parameter : %union.4
// CHECK-NEXT:                Decl %d.11 'bar' ScopedFunction : %function.10
// CHECK-NEXT:                Decl %d.12 'arguments' Var Arguments
// CHECK-NEXT:                hoistedFunction bar
// CHECK-NEXT:                Scope %s.5
// CHECK-NEXT:            Func strict
// CHECK-NEXT:                Scope %s.6
// CHECK-NEXT:                    Decl %d.13 'c' Parameter : %class.2
// CHECK-NEXT:                    Decl %d.14 'cv' Parameter : %union.9
// CHECK-NEXT:                    Decl %d.15 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.7
// CHECK-NEXT:                Decl %d.16 'cur' Parameter : %union.4
// CHECK-NEXT:                Decl %d.17 'a' Let : %union.4
// CHECK-NEXT:                Decl %d.18 'c' Let : %class.2
// CHECK-NEXT:                Decl %d.19 'arguments' Var Arguments
// CHECK-NEXT:                Scope %s.8
// CHECK-NEXT:                    Decl %d.20 'b' Let : %class.2
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.9
// CHECK-NEXT:                Decl %d.21 'c' Parameter : %union.4
// CHECK-NEXT:                Decl %d.22 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    ClassDeclaration
// CHECK-NEXT:                        Id 'C' [D:E:%d.2 'C']
// CHECK-NEXT:                        ClassBody
// CHECK-NEXT:                            ClassProperty : %union.4
// CHECK-NEXT:                                Id 'next'
// CHECK-NEXT:                    FunctionDeclaration : %function.6
// CHECK-NEXT:                        Id 'loop' [D:E:%d.3 'loop']
// CHECK-NEXT:                        Id 'cur' [D:E:%d.8 'cur']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            WhileStatement
// CHECK-NEXT:                                ExpressionStatement
// CHECK-NEXT:                                    AssignmentExpression : %union.4
// CHECK-NEXT:                                        Id 'cur' [D:E:%d.8 'cur'] : %union.4
// CHECK-NEXT:                                        MemberExpression : %union.4
// CHECK-NEXT:                                            ImplicitCheckedCast : %class.2
// CHECK-NEXT:                                                Id 'cur' [D:E:%d.8 'cur'] : %union.4
// CHECK-NEXT:                                            Id 'next'
// CHECK-NEXT:                                Id 'cur' [D:E:%d.8 'cur'] : %union.4
// CHECK-NEXT:                            ReturnStatement
// CHECK-NEXT:                                Id 'cur' [D:E:%d.8 'cur'] : %union.4
// CHECK-NEXT:                    FunctionDeclaration : %function.7
// CHECK-NEXT:                        Id 'call' [D:E:%d.4 'call']
// CHECK-NEXT:                        Id 'cur' [D:E:%d.10 'cur']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            FunctionDeclaration : %function.10
// CHECK-NEXT:                                Id 'bar' [D:E:%d.11 'bar']
// CHECK-NEXT:                                Id 'c' [D:E:%d.13 'c']
// CHECK-NEXT:                                Id 'cv' [D:E:%d.14 'cv']
// CHECK-NEXT:                                BlockStatement
// CHECK-NEXT:                            IfStatement
// CHECK-NEXT:                                Id 'cur' [D:E:%d.10 'cur'] : %union.4
// CHECK-NEXT:                                BlockStatement Scope %s.5
// CHECK-NEXT:                                    ExpressionStatement
// CHECK-NEXT:                                        CallExpression : void
// CHECK-NEXT:                                            Id 'bar' [D:E:%d.11 'bar'] : %function.10
// CHECK-NEXT:                                            ImplicitCheckedCast : %class.2
// CHECK-NEXT:                                                Id 'cur' [D:E:%d.10 'cur'] : %union.4
// CHECK-NEXT:                                            ImplicitCheckedCast : %class.2
// CHECK-NEXT:                                                Id 'cur' [D:E:%d.10 'cur'] : %union.4
// CHECK-NEXT:                    FunctionDeclaration : %function.7
// CHECK-NEXT:                        Id 'assign' [D:E:%d.5 'assign']
// CHECK-NEXT:                        Id 'cur' [D:E:%d.16 'cur']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            VariableDeclaration
// CHECK-NEXT:                                VariableDeclarator
// CHECK-NEXT:                                    Id 'cur' [D:E:%d.16 'cur'] : %union.4
// CHECK-NEXT:                                    Id 'a' [D:E:%d.17 'a']
// CHECK-NEXT:                            VariableDeclaration
// CHECK-NEXT:                                VariableDeclarator
// CHECK-NEXT:                                    Id 'c' [D:E:%d.18 'c']
// CHECK-NEXT:                            IfStatement
// CHECK-NEXT:                                Id 'cur' [D:E:%d.16 'cur'] : %union.4
// CHECK-NEXT:                                BlockStatement Scope %s.8
// CHECK-NEXT:                                    VariableDeclaration
// CHECK-NEXT:                                        VariableDeclarator
// CHECK-NEXT:                                            ImplicitCheckedCast : %class.2
// CHECK-NEXT:                                                Id 'cur' [D:E:%d.16 'cur'] : %union.4
// CHECK-NEXT:                                            Id 'b' [D:E:%d.20 'b']
// CHECK-NEXT:                                    ExpressionStatement
// CHECK-NEXT:                                        AssignmentExpression : %class.2
// CHECK-NEXT:                                            Id 'c' [D:E:%d.18 'c'] : %class.2
// CHECK-NEXT:                                            ImplicitCheckedCast : %class.2
// CHECK-NEXT:                                                Id 'a' [D:E:%d.17 'a'] : %union.4
// CHECK-NEXT:                    FunctionDeclaration : %function.8
// CHECK-NEXT:                        Id 'retval' [D:E:%d.6 'retval']
// CHECK-NEXT:                        Id 'c' [D:E:%d.21 'c']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            IfStatement
// CHECK-NEXT:                                Id 'c' [D:E:%d.21 'c'] : %union.4
// CHECK-NEXT:                                ReturnStatement
// CHECK-NEXT:                                    ImplicitCheckedCast : %class.2
// CHECK-NEXT:                                        Id 'c' [D:E:%d.21 'c'] : %union.4
// CHECK-NEXT:                                ReturnStatement
// CHECK-NEXT:                                    NewExpression : %class.2
// CHECK-NEXT:                                        Id 'C' [D:E:%d.2 'C'] : %class_constructor.5
// CHECK-NEXT:            ObjectExpression
