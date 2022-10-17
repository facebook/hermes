/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

function logical_and(y, x) { return x && y }

function logical_or(y, x) { return x || y }

function logical_and_and(y, x, z) { return x && y && z }

function logical_and_or(y, x, z) { return x && (y || z) }

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [logical_and, logical_or, logical_and_and, logical_and_or]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %logical_and#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "logical_and" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %logical_or#0#1()#3, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "logical_or" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %logical_and_and#0#1()#4, %0
// CHECK-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "logical_and_and" : string
// CHECK-NEXT:  %7 = CreateFunctionInst %logical_and_or#0#1()#5, %0
// CHECK-NEXT:  %8 = StorePropertyInst %7 : closure, globalObject : object, "logical_and_or" : string
// CHECK-NEXT:  %9 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %10 = StoreStackInst undefined : undefined, %9
// CHECK-NEXT:  %11 = LoadStackInst %9
// CHECK-NEXT:  %12 = ReturnInst %11
// CHECK-NEXT:function_end

// CHECK:function logical_and#0#1(y, x)#2
// CHECK-NEXT:frame = [y#2, x#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{logical_and#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst %y, [y#2], %0
// CHECK-NEXT:  %2 = StoreFrameInst %x, [x#2], %0
// CHECK-NEXT:  %3 = AllocStackInst $?anon_0_logical
// CHECK-NEXT:  %4 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %5 = StoreStackInst %4, %3
// CHECK-NEXT:  %6 = CondBranchInst %4, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %8 = StoreStackInst %7, %3
// CHECK-NEXT:  %9 = BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %10 = LoadStackInst %3
// CHECK-NEXT:  %11 = ReturnInst %10
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %12 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function logical_or#0#1(y, x)#3
// CHECK-NEXT:frame = [y#3, x#3]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{logical_or#0#1()#3}
// CHECK-NEXT:  %1 = StoreFrameInst %y, [y#3], %0
// CHECK-NEXT:  %2 = StoreFrameInst %x, [x#3], %0
// CHECK-NEXT:  %3 = AllocStackInst $?anon_0_logical
// CHECK-NEXT:  %4 = LoadFrameInst [x#3], %0
// CHECK-NEXT:  %5 = StoreStackInst %4, %3
// CHECK-NEXT:  %6 = CondBranchInst %4, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %7 = LoadFrameInst [y#3], %0
// CHECK-NEXT:  %8 = StoreStackInst %7, %3
// CHECK-NEXT:  %9 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = LoadStackInst %3
// CHECK-NEXT:  %11 = ReturnInst %10
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %12 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function logical_and_and#0#1(y, x, z)#4
// CHECK-NEXT:frame = [y#4, x#4, z#4]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{logical_and_and#0#1()#4}
// CHECK-NEXT:  %1 = StoreFrameInst %y, [y#4], %0
// CHECK-NEXT:  %2 = StoreFrameInst %x, [x#4], %0
// CHECK-NEXT:  %3 = StoreFrameInst %z, [z#4], %0
// CHECK-NEXT:  %4 = AllocStackInst $?anon_0_logical
// CHECK-NEXT:  %5 = AllocStackInst $?anon_1_logical
// CHECK-NEXT:  %6 = LoadFrameInst [x#4], %0
// CHECK-NEXT:  %7 = StoreStackInst %6, %5
// CHECK-NEXT:  %8 = CondBranchInst %6, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %9 = LoadFrameInst [z#4], %0
// CHECK-NEXT:  %10 = StoreStackInst %9, %4
// CHECK-NEXT:  %11 = BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %12 = LoadStackInst %4
// CHECK-NEXT:  %13 = ReturnInst %12
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %14 = LoadFrameInst [y#4], %0
// CHECK-NEXT:  %15 = StoreStackInst %14, %5
// CHECK-NEXT:  %16 = BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %17 = LoadStackInst %5
// CHECK-NEXT:  %18 = StoreStackInst %17, %4
// CHECK-NEXT:  %19 = CondBranchInst %17, %BB3, %BB4
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %20 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function logical_and_or#0#1(y, x, z)#5
// CHECK-NEXT:frame = [y#5, x#5, z#5]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{logical_and_or#0#1()#5}
// CHECK-NEXT:  %1 = StoreFrameInst %y, [y#5], %0
// CHECK-NEXT:  %2 = StoreFrameInst %x, [x#5], %0
// CHECK-NEXT:  %3 = StoreFrameInst %z, [z#5], %0
// CHECK-NEXT:  %4 = AllocStackInst $?anon_0_logical
// CHECK-NEXT:  %5 = LoadFrameInst [x#5], %0
// CHECK-NEXT:  %6 = StoreStackInst %5, %4
// CHECK-NEXT:  %7 = CondBranchInst %5, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = AllocStackInst $?anon_1_logical
// CHECK-NEXT:  %9 = LoadFrameInst [y#5], %0
// CHECK-NEXT:  %10 = StoreStackInst %9, %8
// CHECK-NEXT:  %11 = CondBranchInst %9, %BB3, %BB4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %12 = LoadStackInst %4
// CHECK-NEXT:  %13 = ReturnInst %12
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %14 = LoadFrameInst [z#5], %0
// CHECK-NEXT:  %15 = StoreStackInst %14, %8
// CHECK-NEXT:  %16 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %17 = LoadStackInst %8
// CHECK-NEXT:  %18 = StoreStackInst %17, %4
// CHECK-NEXT:  %19 = BranchInst %BB2
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %20 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
