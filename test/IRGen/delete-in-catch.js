/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines

// Ensure that catch variables are not treated as global.
var result;
try {
    foo();
} catch(e) {
    result = delete e;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [?anon_1_e#1], globals = [result]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %2 = StoreStackInst undefined : undefined, %1
// CHECK-NEXT:  %3 = TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = CatchInst
// CHECK-NEXT:  %5 = StoreFrameInst %4, [?anon_1_e#1], %0
// CHECK-NEXT:  %6 = StorePropertyInst false : boolean, globalObject : object, "result" : string
// CHECK-NEXT:  %7 = StoreStackInst false : boolean, %1
// CHECK-NEXT:  %8 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %9 = LoadStackInst %1
// CHECK-NEXT:  %10 = ReturnInst %9
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %11 = TryLoadGlobalPropertyInst globalObject : object, "foo" : string
// CHECK-NEXT:  %12 = CallInst %11, undefined : undefined
// CHECK-NEXT:  %13 = StoreStackInst %12, %1
// CHECK-NEXT:  %14 = BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %15 = TryEndInst
// CHECK-NEXT:  %16 = BranchInst %BB3
// CHECK-NEXT:function_end
