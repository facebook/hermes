/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -hermes-parser -dump-ir %s -O

function logical_and(y, x) { return x && y }

function logical_or(y, x) { return x || y }

function logical_and_and(y, x, z) { return x && y && z }

function logical_and_or(y, x, z) { return x && (y || z) }

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "logical_and" : string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "logical_or" : string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "logical_and_and" : string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "logical_and_or" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %logical_and()
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4 : closure, globalObject : object, "logical_and" : string
// CHECK-NEXT:  %6 = CreateFunctionInst %logical_or()
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6 : closure, globalObject : object, "logical_or" : string
// CHECK-NEXT:  %8 = CreateFunctionInst %logical_and_and()
// CHECK-NEXT:  %9 = StorePropertyLooseInst %8 : closure, globalObject : object, "logical_and_and" : string
// CHECK-NEXT:  %10 = CreateFunctionInst %logical_and_or()
// CHECK-NEXT:  %11 = StorePropertyLooseInst %10 : closure, globalObject : object, "logical_and_or" : string
// CHECK-NEXT:  %12 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %13 = StoreStackInst undefined : undefined, %12
// CHECK-NEXT:  %14 = LoadStackInst %12
// CHECK-NEXT:  %15 = ReturnInst %14
// CHECK-NEXT:function_end

// CHECK:function logical_and(y, x)
// CHECK-NEXT:frame = [y, x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %y
// CHECK-NEXT:  %1 = StoreFrameInst %0, [y]
// CHECK-NEXT:  %2 = LoadParamInst %x
// CHECK-NEXT:  %3 = StoreFrameInst %2, [x]
// CHECK-NEXT:  %4 = AllocStackInst $?anon_0_logical
// CHECK-NEXT:  %5 = LoadFrameInst [x]
// CHECK-NEXT:  %6 = StoreStackInst %5, %4
// CHECK-NEXT:  %7 = CondBranchInst %5, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = LoadFrameInst [y]
// CHECK-NEXT:  %9 = StoreStackInst %8, %4
// CHECK-NEXT:  %10 = BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %11 = LoadStackInst %4
// CHECK-NEXT:  %12 = ReturnInst %11
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %13 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function logical_or(y, x)
// CHECK-NEXT:frame = [y, x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %y
// CHECK-NEXT:  %1 = StoreFrameInst %0, [y]
// CHECK-NEXT:  %2 = LoadParamInst %x
// CHECK-NEXT:  %3 = StoreFrameInst %2, [x]
// CHECK-NEXT:  %4 = AllocStackInst $?anon_0_logical
// CHECK-NEXT:  %5 = LoadFrameInst [x]
// CHECK-NEXT:  %6 = StoreStackInst %5, %4
// CHECK-NEXT:  %7 = CondBranchInst %5, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %8 = LoadFrameInst [y]
// CHECK-NEXT:  %9 = StoreStackInst %8, %4
// CHECK-NEXT:  %10 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %11 = LoadStackInst %4
// CHECK-NEXT:  %12 = ReturnInst %11
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %13 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function logical_and_and(y, x, z)
// CHECK-NEXT:frame = [y, x, z]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %y
// CHECK-NEXT:  %1 = StoreFrameInst %0, [y]
// CHECK-NEXT:  %2 = LoadParamInst %x
// CHECK-NEXT:  %3 = StoreFrameInst %2, [x]
// CHECK-NEXT:  %4 = LoadParamInst %z
// CHECK-NEXT:  %5 = StoreFrameInst %4, [z]
// CHECK-NEXT:  %6 = AllocStackInst $?anon_0_logical
// CHECK-NEXT:  %7 = AllocStackInst $?anon_1_logical
// CHECK-NEXT:  %8 = LoadFrameInst [x]
// CHECK-NEXT:  %9 = StoreStackInst %8, %7
// CHECK-NEXT:  %10 = CondBranchInst %8, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %11 = LoadFrameInst [z]
// CHECK-NEXT:  %12 = StoreStackInst %11, %6
// CHECK-NEXT:  %13 = BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %14 = LoadStackInst %6
// CHECK-NEXT:  %15 = ReturnInst %14
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %16 = LoadFrameInst [y]
// CHECK-NEXT:  %17 = StoreStackInst %16, %7
// CHECK-NEXT:  %18 = BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %19 = LoadStackInst %7
// CHECK-NEXT:  %20 = StoreStackInst %19, %6
// CHECK-NEXT:  %21 = CondBranchInst %19, %BB3, %BB4
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %22 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function logical_and_or(y, x, z)
// CHECK-NEXT:frame = [y, x, z]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %y
// CHECK-NEXT:  %1 = StoreFrameInst %0, [y]
// CHECK-NEXT:  %2 = LoadParamInst %x
// CHECK-NEXT:  %3 = StoreFrameInst %2, [x]
// CHECK-NEXT:  %4 = LoadParamInst %z
// CHECK-NEXT:  %5 = StoreFrameInst %4, [z]
// CHECK-NEXT:  %6 = AllocStackInst $?anon_0_logical
// CHECK-NEXT:  %7 = LoadFrameInst [x]
// CHECK-NEXT:  %8 = StoreStackInst %7, %6
// CHECK-NEXT:  %9 = CondBranchInst %7, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = AllocStackInst $?anon_1_logical
// CHECK-NEXT:  %11 = LoadFrameInst [y]
// CHECK-NEXT:  %12 = StoreStackInst %11, %10
// CHECK-NEXT:  %13 = CondBranchInst %11, %BB3, %BB4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %14 = LoadStackInst %6
// CHECK-NEXT:  %15 = ReturnInst %14
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %16 = LoadFrameInst [z]
// CHECK-NEXT:  %17 = StoreStackInst %16, %10
// CHECK-NEXT:  %18 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %19 = LoadStackInst %10
// CHECK-NEXT:  %20 = StoreStackInst %19, %6
// CHECK-NEXT:  %21 = BranchInst %BB2
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %22 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
