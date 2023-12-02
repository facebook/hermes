/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir %s -O | %FileCheckOrRegen %s --match-full-lines

// This test fails in Hermes because IR dumping of bigints is broken.

// Verify that arithmetic operations with a BigInt operand and non-BigInt
// operand (except adds) are not DCE-d, because that would throw a runtime
// exception.

function f1_throws() { 1n << 1; }
function f2_throws() { 1n + 1; }
function f3_ok() { "a" + 1n; }
function f4_ok() { 1n + "a"; }
function f5_ok() { 1n < "a"; }
function f6_ok() { "a" >= 1n; }
function f7_ok() { 1 >= 1n; }
function f8_ok() { 1 * "a"; }

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1 : undefined
// CHECK-NEXT:globals = [f1_throws, f2_throws, f3_ok, f4_ok, f5_ok, f6_ok, f7_ok, f8_ok]
// CHECK-NEXT:S{global#0()#1} = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %f1_throws#0#1()#2 : undefined, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "f1_throws" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %f2_throws#0#1()#3 : undefined, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "f2_throws" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %f3_ok#0#1()#4 : undefined, %0
// CHECK-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "f3_ok" : string
// CHECK-NEXT:  %7 = CreateFunctionInst %f4_ok#0#1()#5 : undefined, %0
// CHECK-NEXT:  %8 = StorePropertyInst %7 : closure, globalObject : object, "f4_ok" : string
// CHECK-NEXT:  %9 = CreateFunctionInst %f5_ok#0#1()#6 : undefined, %0
// CHECK-NEXT:  %10 = StorePropertyInst %9 : closure, globalObject : object, "f5_ok" : string
// CHECK-NEXT:  %11 = CreateFunctionInst %f6_ok#0#1()#7 : undefined, %0
// CHECK-NEXT:  %12 = StorePropertyInst %11 : closure, globalObject : object, "f6_ok" : string
// CHECK-NEXT:  %13 = CreateFunctionInst %f7_ok#0#1()#8 : undefined, %0
// CHECK-NEXT:  %14 = StorePropertyInst %13 : closure, globalObject : object, "f7_ok" : string
// CHECK-NEXT:  %15 = CreateFunctionInst %f8_ok#0#1()#9 : undefined, %0
// CHECK-NEXT:  %16 = StorePropertyInst %15 : closure, globalObject : object, "f8_ok" : string
// CHECK-NEXT:  %17 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function f1_throws#0#1()#2 : undefined
// CHECK-NEXT:S{f1_throws#0#1()#2} = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{f1_throws#0#1()#2}
// CHECK-NEXT:  %1 = BinaryOperatorInst '<<', 1n : bigint, 1 : number
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function f2_throws#0#1()#3 : undefined
// CHECK-NEXT:S{f2_throws#0#1()#3} = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{f2_throws#0#1()#3}
// CHECK-NEXT:  %1 = BinaryOperatorInst '+', 1n : bigint, 1 : number
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function f3_ok#0#1()#4 : undefined
// CHECK-NEXT:S{f3_ok#0#1()#4} = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{f3_ok#0#1()#4}
// CHECK-NEXT:  %1 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function f4_ok#0#1()#5 : undefined
// CHECK-NEXT:S{f4_ok#0#1()#5} = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{f4_ok#0#1()#5}
// CHECK-NEXT:  %1 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function f5_ok#0#1()#6 : undefined
// CHECK-NEXT:S{f5_ok#0#1()#6} = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{f5_ok#0#1()#6}
// CHECK-NEXT:  %1 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function f6_ok#0#1()#7 : undefined
// CHECK-NEXT:S{f6_ok#0#1()#7} = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{f6_ok#0#1()#7}
// CHECK-NEXT:  %1 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function f7_ok#0#1()#8 : undefined
// CHECK-NEXT:S{f7_ok#0#1()#8} = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{f7_ok#0#1()#8}
// CHECK-NEXT:  %1 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function f8_ok#0#1()#9 : undefined
// CHECK-NEXT:S{f8_ok#0#1()#9} = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{f8_ok#0#1()#9}
// CHECK-NEXT:  %1 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
