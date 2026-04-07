/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals --typed --dump-sema %s | %FileCheckOrRegen %s --match-full-lines

let fopen = $SHBuiltin.extern_c({},function fopen(path: c_ptr, mode: c_ptr): c_ptr {throw 0});
let fopen1 = $SHBuiltin.extern_c({},function fopen(path: c_ptr, mode: c_ptr): c_ptr {throw 0});
let load  = $SHBuiltin.extern_c({}, function load(addr: c_ptr): c_u8 {throw 0});

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%object.2 = object({
// CHECK-NEXT:})
// CHECK-NEXT:%function.3 = function(path: c_ptr, mode: c_ptr): c_ptr
// CHECK-NEXT:%native_function.4 = native_function(path: c_ptr, mode: c_ptr): c_ptr [void * (*)(void *, void *)]
// CHECK-NEXT:%function.5 = function(addr: c_ptr): number
// CHECK-NEXT:%native_function.6 = native_function(addr: c_ptr): number [uint8_t (*)(void *)]

// CHECK:extern "C" void * fopen(void *, void *);
// CHECK-NEXT:extern "C" uint8_t load(void *);

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'fopen' Let : %native_function.4
// CHECK-NEXT:        Decl %d.3 'fopen1' Let : %native_function.4
// CHECK-NEXT:        Decl %d.4 'load' Let : %native_function.6
// CHECK-NEXT:        Decl %d.5 'arguments' Var Arguments
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.6 'fopen' FunctionExprName : %function.3
// CHECK-NEXT:        Scope %s.3
// CHECK-NEXT:            Decl %d.7 'fopen' FunctionExprName : %function.3
// CHECK-NEXT:        Scope %s.4
// CHECK-NEXT:            Decl %d.8 'load' FunctionExprName : %function.5
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.5
// CHECK-NEXT:            Decl %d.9 'path' Parameter : c_ptr
// CHECK-NEXT:            Decl %d.10 'mode' Parameter : c_ptr
// CHECK-NEXT:            Decl %d.11 'arguments' Var Arguments
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.6
// CHECK-NEXT:            Decl %d.12 'path' Parameter : c_ptr
// CHECK-NEXT:            Decl %d.13 'mode' Parameter : c_ptr
// CHECK-NEXT:            Decl %d.14 'arguments' Var Arguments
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.7
// CHECK-NEXT:            Decl %d.15 'addr' Parameter : c_ptr
// CHECK-NEXT:            Decl %d.16 'arguments' Var Arguments

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                CallExpression : %native_function.4
// CHECK-NEXT:                    MemberExpression : any
// CHECK-NEXT:                        SHBuiltin
// CHECK-NEXT:                        Id 'extern_c'
// CHECK-NEXT:                    ObjectExpression : %object.2
// CHECK-NEXT:                    FunctionExpression : %function.3 Scope %s.2
// CHECK-NEXT:                        Id 'fopen' [D:E:%d.6 'fopen']
// CHECK-NEXT:                        Id 'path' [D:E:%d.9 'path']
// CHECK-NEXT:                        Id 'mode' [D:E:%d.10 'mode']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            ThrowStatement
// CHECK-NEXT:                                NumericLiteral
// CHECK-NEXT:                Id 'fopen' [D:E:%d.2 'fopen']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                CallExpression : %native_function.4
// CHECK-NEXT:                    MemberExpression : any
// CHECK-NEXT:                        SHBuiltin
// CHECK-NEXT:                        Id 'extern_c'
// CHECK-NEXT:                    ObjectExpression : %object.2
// CHECK-NEXT:                    FunctionExpression : %function.3 Scope %s.3
// CHECK-NEXT:                        Id 'fopen' [D:E:%d.7 'fopen']
// CHECK-NEXT:                        Id 'path' [D:E:%d.12 'path']
// CHECK-NEXT:                        Id 'mode' [D:E:%d.13 'mode']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            ThrowStatement
// CHECK-NEXT:                                NumericLiteral
// CHECK-NEXT:                Id 'fopen1' [D:E:%d.3 'fopen1']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                CallExpression : %native_function.6
// CHECK-NEXT:                    MemberExpression : any
// CHECK-NEXT:                        SHBuiltin
// CHECK-NEXT:                        Id 'extern_c'
// CHECK-NEXT:                    ObjectExpression : %object.2
// CHECK-NEXT:                    FunctionExpression : %function.5 Scope %s.4
// CHECK-NEXT:                        Id 'load' [D:E:%d.8 'load']
// CHECK-NEXT:                        Id 'addr' [D:E:%d.15 'addr']
// CHECK-NEXT:                        BlockStatement
// CHECK-NEXT:                            ThrowStatement
// CHECK-NEXT:                                NumericLiteral
// CHECK-NEXT:                Id 'load' [D:E:%d.4 'load']
