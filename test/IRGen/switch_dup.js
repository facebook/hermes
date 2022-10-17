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

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [foo1, foo2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %foo1#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "foo1" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %foo2#0#1()#3, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "foo2" : string
// CHECK-NEXT:  %5 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %6 = StoreStackInst undefined : undefined, %5
// CHECK-NEXT:  %7 = LoadStackInst %5
// CHECK-NEXT:  %8 = ReturnInst %7
// CHECK-NEXT:function_end

// CHECK:function foo1#0#1(x)#2
// CHECK-NEXT:frame = [x#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{foo1#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst %x, [x#2], %0
// CHECK-NEXT:  %2 = LoadFrameInst [x#2], %0
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

// CHECK:function foo2#0#1(x)#3
// CHECK-NEXT:frame = [x#3]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{foo2#0#1()#3}
// CHECK-NEXT:  %1 = StoreFrameInst %x, [x#3], %0
// CHECK-NEXT:  %2 = LoadFrameInst [x#3], %0
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
