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

// CHECK:function global()
// CHECK-NEXT:frame = [], globals = [logical_and, logical_or, logical_and_and, logical_and_or]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %logical_and()
// CHECK-NEXT:  %1 = StorePropertyInst %0 : closure, globalObject : object, "logical_and" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %logical_or()
// CHECK-NEXT:  %3 = StorePropertyInst %2 : closure, globalObject : object, "logical_or" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %logical_and_and()
// CHECK-NEXT:  %5 = StorePropertyInst %4 : closure, globalObject : object, "logical_and_and" : string
// CHECK-NEXT:  %6 = CreateFunctionInst %logical_and_or()
// CHECK-NEXT:  %7 = StorePropertyInst %6 : closure, globalObject : object, "logical_and_or" : string
// CHECK-NEXT:  %8 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %9 = StoreStackInst undefined : undefined, %8
// CHECK-NEXT:  %10 = LoadStackInst %8
// CHECK-NEXT:  %11 = ReturnInst %10
// CHECK-NEXT:function_end

// CHECK:function logical_and(y, x)
// CHECK-NEXT:frame = [y, x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %y, [y]
// CHECK-NEXT:  %1 = StoreFrameInst %x, [x]
// CHECK-NEXT:  %2 = AllocStackInst $?anon_0_logical
// CHECK-NEXT:  %3 = LoadFrameInst [x]
// CHECK-NEXT:  %4 = StoreStackInst %3, %2
// CHECK-NEXT:  %5 = CondBranchInst %3, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = LoadFrameInst [y]
// CHECK-NEXT:  %7 = StoreStackInst %6, %2
// CHECK-NEXT:  %8 = BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %9 = LoadStackInst %2
// CHECK-NEXT:  %10 = ReturnInst %9
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %11 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function logical_or(y, x)
// CHECK-NEXT:frame = [y, x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %y, [y]
// CHECK-NEXT:  %1 = StoreFrameInst %x, [x]
// CHECK-NEXT:  %2 = AllocStackInst $?anon_0_logical
// CHECK-NEXT:  %3 = LoadFrameInst [x]
// CHECK-NEXT:  %4 = StoreStackInst %3, %2
// CHECK-NEXT:  %5 = CondBranchInst %3, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = LoadFrameInst [y]
// CHECK-NEXT:  %7 = StoreStackInst %6, %2
// CHECK-NEXT:  %8 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = LoadStackInst %2
// CHECK-NEXT:  %10 = ReturnInst %9
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %11 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function logical_and_and(y, x, z)
// CHECK-NEXT:frame = [y, x, z]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %y, [y]
// CHECK-NEXT:  %1 = StoreFrameInst %x, [x]
// CHECK-NEXT:  %2 = StoreFrameInst %z, [z]
// CHECK-NEXT:  %3 = AllocStackInst $?anon_0_logical
// CHECK-NEXT:  %4 = AllocStackInst $?anon_1_logical
// CHECK-NEXT:  %5 = LoadFrameInst [x]
// CHECK-NEXT:  %6 = StoreStackInst %5, %4
// CHECK-NEXT:  %7 = CondBranchInst %5, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = LoadFrameInst [z]
// CHECK-NEXT:  %9 = StoreStackInst %8, %3
// CHECK-NEXT:  %10 = BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %11 = LoadStackInst %3
// CHECK-NEXT:  %12 = ReturnInst %11
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %13 = LoadFrameInst [y]
// CHECK-NEXT:  %14 = StoreStackInst %13, %4
// CHECK-NEXT:  %15 = BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %16 = LoadStackInst %4
// CHECK-NEXT:  %17 = StoreStackInst %16, %3
// CHECK-NEXT:  %18 = CondBranchInst %16, %BB3, %BB4
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %19 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function logical_and_or(y, x, z)
// CHECK-NEXT:frame = [y, x, z]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %y, [y]
// CHECK-NEXT:  %1 = StoreFrameInst %x, [x]
// CHECK-NEXT:  %2 = StoreFrameInst %z, [z]
// CHECK-NEXT:  %3 = AllocStackInst $?anon_0_logical
// CHECK-NEXT:  %4 = LoadFrameInst [x]
// CHECK-NEXT:  %5 = StoreStackInst %4, %3
// CHECK-NEXT:  %6 = CondBranchInst %4, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = AllocStackInst $?anon_1_logical
// CHECK-NEXT:  %8 = LoadFrameInst [y]
// CHECK-NEXT:  %9 = StoreStackInst %8, %7
// CHECK-NEXT:  %10 = CondBranchInst %8, %BB3, %BB4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %11 = LoadStackInst %3
// CHECK-NEXT:  %12 = ReturnInst %11
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %13 = LoadFrameInst [z]
// CHECK-NEXT:  %14 = StoreStackInst %13, %7
// CHECK-NEXT:  %15 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %16 = LoadStackInst %7
// CHECK-NEXT:  %17 = StoreStackInst %16, %3
// CHECK-NEXT:  %18 = BranchInst %BB2
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %19 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
