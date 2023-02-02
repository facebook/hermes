/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

// Ensure that the "break" in the finally handler exits the correct
// loop.

function foo() {
    outer:
    for (;;) {
        try {
            for (;;) {
                bar2();
                break outer;
            }
            bar3();
        } finally {
            finally1();
            break;
        }
    }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "foo" : string
// CHECK-NEXT:  %1 = CreateFunctionInst %foo()
// CHECK-NEXT:  %2 = StorePropertyLooseInst %1 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %3 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %4 = StoreStackInst undefined : undefined, %3
// CHECK-NEXT:  %5 = LoadStackInst %3
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:function_end

// CHECK:function foo()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = BranchInst %BB1
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %1 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = TryStartInst %BB3, %BB4
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %3 = BranchInst %BB2
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %4 = BranchInst %BB1
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %5 = BranchInst %BB6
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %6 = CatchInst
// CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst globalObject : object, "finally1" : string
// CHECK-NEXT:  %8 = CallInst %7, undefined : undefined
// CHECK-NEXT:  %9 = BranchInst %BB5
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %10 = BranchInst %BB7
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %11 = BranchInst %BB9
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %12 = TryLoadGlobalPropertyInst globalObject : object, "bar2" : string
// CHECK-NEXT:  %13 = CallInst %12, undefined : undefined
// CHECK-NEXT:  %14 = BranchInst %BB10
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %15 = TryLoadGlobalPropertyInst globalObject : object, "bar3" : string
// CHECK-NEXT:  %16 = CallInst %15, undefined : undefined
// CHECK-NEXT:  %17 = BranchInst %BB12
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %18 = BranchInst %BB9
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %19 = BranchInst %BB13
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %20 = TryEndInst
// CHECK-NEXT:  %21 = TryLoadGlobalPropertyInst globalObject : object, "finally1" : string
// CHECK-NEXT:  %22 = CallInst %21, undefined : undefined
// CHECK-NEXT:  %23 = BranchInst %BB5
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %24 = BranchInst %BB5
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %25 = BranchInst %BB14
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %26 = TryEndInst
// CHECK-NEXT:  %27 = TryLoadGlobalPropertyInst globalObject : object, "finally1" : string
// CHECK-NEXT:  %28 = CallInst %27, undefined : undefined
// CHECK-NEXT:  %29 = BranchInst %BB5
// CHECK-NEXT:%BB17:
// CHECK-NEXT:  %30 = BranchInst %BB8
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %31 = ThrowInst %6
// CHECK-NEXT:function_end
