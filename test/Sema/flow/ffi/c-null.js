/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals --typed --dump-sema %s | %FileCheckOrRegen %s --match-full-lines

const c_null = $SHBuiltin.c_null();

let x = c_null;

// Auto-generated content below. Please do not modify manually.

// CHECK:untyped function %t.1 = untyped function ()

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 '$SHBuiltin' UndeclaredGlobalProperty
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.2 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.3 'c_null' Const : c_ptr
// CHECK-NEXT:            Decl %d.4 'x' Let : c_ptr
// CHECK-NEXT:            Decl %d.5 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : untyped function %t.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.2 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            CallExpression : c_ptr
// CHECK-NEXT:                                MemberExpression : any
// CHECK-NEXT:                                    SHBuiltin
// CHECK-NEXT:                                    Id 'c_null'
// CHECK-NEXT:                            Id 'c_null' [D:E:%d.3 'c_null']
// CHECK-NEXT:                    VariableDeclaration
// CHECK-NEXT:                        VariableDeclarator
// CHECK-NEXT:                            Id 'c_null' [D:E:%d.3 'c_null'] : c_ptr
// CHECK-NEXT:                            Id 'x' [D:E:%d.4 'x']
// CHECK-NEXT:            ObjectExpression
