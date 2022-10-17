/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -O0 -dump-bytecode %s

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

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [foo]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %foo#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %3 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %4 = StoreStackInst undefined : undefined, %3
// CHECK-NEXT:  %5 = LoadStackInst %3
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:function_end

// CHECK:function foo#0#1()#2
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{foo#0#1()#2}
// CHECK-NEXT:  %1 = BranchInst %BB1
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %3 = TryStartInst %BB4, %BB5
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %4 = BranchInst %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = BranchInst %BB3
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %6 = BranchInst %BB3
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %7 = BranchInst %BB7
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %8 = CatchInst
// CHECK-NEXT:  %9 = TryLoadGlobalPropertyInst globalObject : object, "finally1" : string
// CHECK-NEXT:  %10 = CallInst %9, undefined : undefined
// CHECK-NEXT:  %11 = BranchInst %BB6
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %12 = BranchInst %BB8
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %13 = BranchInst %BB10
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %14 = TryLoadGlobalPropertyInst globalObject : object, "bar2" : string
// CHECK-NEXT:  %15 = CallInst %14, undefined : undefined
// CHECK-NEXT:  %16 = BranchInst %BB12
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %17 = TryLoadGlobalPropertyInst globalObject : object, "bar3" : string
// CHECK-NEXT:  %18 = CallInst %17, undefined : undefined
// CHECK-NEXT:  %19 = BranchInst %BB14
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %20 = BranchInst %BB11
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %21 = BranchInst %BB11
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %22 = BranchInst %BB15
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %23 = TryEndInst
// CHECK-NEXT:  %24 = TryLoadGlobalPropertyInst globalObject : object, "finally1" : string
// CHECK-NEXT:  %25 = CallInst %24, undefined : undefined
// CHECK-NEXT:  %26 = BranchInst %BB6
// CHECK-NEXT:%BB17:
// CHECK-NEXT:  %27 = BranchInst %BB6
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %28 = BranchInst %BB16
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %29 = TryEndInst
// CHECK-NEXT:  %30 = TryLoadGlobalPropertyInst globalObject : object, "finally1" : string
// CHECK-NEXT:  %31 = CallInst %30, undefined : undefined
// CHECK-NEXT:  %32 = BranchInst %BB6
// CHECK-NEXT:%BB19:
// CHECK-NEXT:  %33 = BranchInst %BB9
// CHECK-NEXT:%BB20:
// CHECK-NEXT:  %34 = ThrowInst %8
// CHECK-NEXT:function_end
