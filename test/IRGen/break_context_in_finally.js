/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheck %s --match-full-lines
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

//CHECK-LABEL:function foo()
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = BranchInst %BB1
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %1 = ReturnInst undefined : undefined
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %2 = TryStartInst %BB4, %BB5
//CHECK-NEXT:%BB6:
//CHECK-NEXT:  %3 = BranchInst %BB2
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %4 = BranchInst %BB3
//CHECK-NEXT:%BB7:
//CHECK-NEXT:  %5 = BranchInst %BB3
//CHECK-NEXT:%BB8:
//CHECK-NEXT:  %6 = BranchInst %BB7
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %7 = CatchInst
//CHECK-NEXT:  %8 = TryLoadGlobalPropertyInst globalObject : object, "finally1" : string
//CHECK-NEXT:  %9 = CallInst %8, undefined : undefined
//CHECK-NEXT:  %10 = BranchInst %BB6
//CHECK-NEXT:%BB9:
//CHECK-NEXT:  %11 = BranchInst %BB8
//CHECK-NEXT:%BB5:
//CHECK-NEXT:  %12 = BranchInst %BB10
//CHECK-NEXT:%BB11:
//CHECK-NEXT:  %13 = TryLoadGlobalPropertyInst globalObject : object, "bar2" : string
//CHECK-NEXT:  %14 = CallInst %13, undefined : undefined
//CHECK-NEXT:  %15 = BranchInst %BB12
//CHECK-NEXT:%BB13:
//CHECK-NEXT:  %16 = TryLoadGlobalPropertyInst globalObject : object, "bar3" : string
//CHECK-NEXT:  %17 = CallInst %16, undefined : undefined
//CHECK-NEXT:  %18 = BranchInst %BB14
//CHECK-NEXT:%BB10:
//CHECK-NEXT:  %19 = BranchInst %BB11
//CHECK-NEXT:%BB15:
//CHECK-NEXT:  %20 = BranchInst %BB11
//CHECK-NEXT:%BB16:
//CHECK-NEXT:  %21 = BranchInst %BB15
//CHECK-NEXT:%BB12:
//CHECK-NEXT:  %22 = TryEndInst
//CHECK-NEXT:  %23 = TryLoadGlobalPropertyInst globalObject : object, "finally1" : string
//CHECK-NEXT:  %24 = CallInst %23, undefined : undefined
//CHECK-NEXT:  %25 = BranchInst %BB6
//CHECK-NEXT:%BB17:
//CHECK-NEXT:  %26 = BranchInst %BB6
//CHECK-NEXT:%BB18:
//CHECK-NEXT:  %27 = BranchInst %BB16
//CHECK-NEXT:%BB14:
//CHECK-NEXT:  %28 = TryEndInst
//CHECK-NEXT:  %29 = TryLoadGlobalPropertyInst globalObject : object, "finally1" : string
//CHECK-NEXT:  %30 = CallInst %29, undefined : undefined
//CHECK-NEXT:  %31 = BranchInst %BB6
//CHECK-NEXT:%BB19:
//CHECK-NEXT:  %32 = BranchInst %BB9
//CHECK-NEXT:%BB20:
//CHECK-NEXT:  %33 = ThrowInst %7
//CHECK-NEXT:function_end

