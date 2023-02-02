/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

function foo1(x) {
    switch (x) {
        case 10: return 1;
        case 10: return 2;
        case 11: return 3;
    }
}

function foo2(x) {
    switch (x) {
        case 10: return 1;
        case "10": return 2;
        case "10": return 3;
    }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "foo1" : string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "foo2" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %foo1()
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2 : closure, globalObject : object, "foo1" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %foo2()
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4 : closure, globalObject : object, "foo2" : string
// CHECK-NEXT:  %6 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %7 = StoreStackInst undefined : undefined, %6
// CHECK-NEXT:  %8 = LoadStackInst %6
// CHECK-NEXT:  %9 = ReturnInst %8
// CHECK-NEXT:function_end

// CHECK:function foo1(x)
// CHECK-NEXT:frame = [x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %x
// CHECK-NEXT:  %1 = StoreFrameInst %0, [x]
// CHECK-NEXT:  %2 = LoadFrameInst [x]
// CHECK-NEXT:  %3 = SwitchInst %2, %BB1, 10 : number, %BB2, 11 : number, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = ReturnInst 1 : number
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %6 = BranchInst %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %7 = ReturnInst 2 : number
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %8 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %9 = ReturnInst 3 : number
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %10 = BranchInst %BB1
// CHECK-NEXT:function_end

// CHECK:function foo2(x)
// CHECK-NEXT:frame = [x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %x
// CHECK-NEXT:  %1 = StoreFrameInst %0, [x]
// CHECK-NEXT:  %2 = LoadFrameInst [x]
// CHECK-NEXT:  %3 = SwitchInst %2, %BB1, 10 : number, %BB2, "10" : string, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = ReturnInst 1 : number
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %6 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %7 = ReturnInst 2 : number
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %8 = BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %9 = ReturnInst 3 : number
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %10 = BranchInst %BB1
// CHECK-NEXT:function_end
