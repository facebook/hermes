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

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "a" : string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "b" : string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "foo" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %foo()
// CHECK-NEXT:  %4 = StorePropertyLooseInst %3 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %5 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %6 = StoreStackInst undefined : undefined, %5
// CHECK-NEXT:  %7 = LoadStackInst %5
// CHECK-NEXT:  %8 = ReturnInst %7
// CHECK-NEXT:function_end

// CHECK:function foo(param)
// CHECK-NEXT:frame = [param]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [param]
// CHECK-NEXT:  %1 = LoadParamInst %param
// CHECK-NEXT:  %2 = BinaryOperatorInst '!==', %1, undefined : undefined
// CHECK-NEXT:  %3 = CondBranchInst %2, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %4 = AllocStackInst $?anon_0_logical
// CHECK-NEXT:  %5 = LoadPropertyInst globalObject : object, "a" : string
// CHECK-NEXT:  %6 = StoreStackInst %5, %4
// CHECK-NEXT:  %7 = CondBranchInst %5, %BB3, %BB4
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = PhiInst %1, %BB0, %14, %BB3
// CHECK-NEXT:  %9 = StoreFrameInst %8, [param]
// CHECK-NEXT:  %10 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %11 = LoadPropertyInst globalObject : object, "b" : string
// CHECK-NEXT:  %12 = StoreStackInst %11, %4
// CHECK-NEXT:  %13 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %14 = LoadStackInst %4
// CHECK-NEXT:  %15 = BranchInst %BB1
// CHECK-NEXT:function_end
