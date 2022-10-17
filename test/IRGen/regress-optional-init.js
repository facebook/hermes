/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s
//
// Make sure that the optional initialization generates a correct Phi node, when the initialization
// is multi-block.

var a, b;
function foo(param = a || b) {}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [a, b, foo]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %foo#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %3 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %4 = StoreStackInst undefined : undefined, %3
// CHECK-NEXT:  %5 = LoadStackInst %3
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:function_end

// CHECK:function foo#0#1(param)#2
// CHECK-NEXT:frame = [param#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{foo#0#1()#2}
// CHECK-NEXT:  %1 = BinaryOperatorInst '!==', %param, undefined : undefined
// CHECK-NEXT:  %2 = CondBranchInst %1, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %3 = AllocStackInst $?anon_0_logical
// CHECK-NEXT:  %4 = LoadPropertyInst globalObject : object, "a" : string
// CHECK-NEXT:  %5 = StoreStackInst %4, %3
// CHECK-NEXT:  %6 = CondBranchInst %4, %BB3, %BB4
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = PhiInst %param, %BB0, %13, %BB3
// CHECK-NEXT:  %8 = StoreFrameInst %7, [param#2], %0
// CHECK-NEXT:  %9 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %10 = LoadPropertyInst globalObject : object, "b" : string
// CHECK-NEXT:  %11 = StoreStackInst %10, %3
// CHECK-NEXT:  %12 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %13 = LoadStackInst %3
// CHECK-NEXT:  %14 = BranchInst %BB1
// CHECK-NEXT:function_end
