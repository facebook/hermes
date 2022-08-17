/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheck %s --match-full-lines

// Ensure that catch variables are not treated as global.
var result;
try {
    foo();
} catch(e) {
    result = delete e;
}

//CHECK-LABEL:function global()
//CHECK-NEXT:frame = [?anon_1_e], globals = [result]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = AllocStackInst $?anon_0_ret
//CHECK-NEXT:  %1 = StoreStackInst undefined : undefined, %0
//CHECK-NEXT:  %2 = TryStartInst %BB1, %BB2
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %3 = CatchInst
//CHECK-NEXT:  %4 = StoreFrameInst %3, [?anon_1_e]
//CHECK-NEXT:  %5 = StorePropertyInst false : boolean, globalObject : object, "result" : string
//CHECK-NEXT:  %6 = StoreStackInst false : boolean, %0
//CHECK-NEXT:  %7 = BranchInst %BB3
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %8 = LoadStackInst %0
//CHECK-NEXT:  %9 = ReturnInst %8
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %10 = TryLoadGlobalPropertyInst globalObject : object, "foo" : string
//CHECK-NEXT:  %11 = CallInst %10, undefined : undefined
//CHECK-NEXT:  %12 = StoreStackInst %11, %0
//CHECK-NEXT:  %13 = BranchInst %BB4
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %14 = TryEndInst
//CHECK-NEXT:  %15 = BranchInst %BB3
//CHECK-NEXT:function_end
