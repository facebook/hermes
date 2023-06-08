/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

// Test that introducing a try statement does not prevent promoting stack
// locations used outside that try statement to SSA values.
function foo(sink){
    var x = 0;
    try {} catch {}
    sink(x);
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %foo(): undefined
// CHECK-NEXT:  %2 = StorePropertyLooseInst %1: object, globalObject: object, "foo": string
// CHECK-NEXT:  %3 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function foo(sink: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %sink: any
// CHECK-NEXT:  %1 = TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = CatchInst (:any)
// CHECK-NEXT:  %3 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %4 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %5 = ReturnInst undefined: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %7 = TryEndInst
// CHECK-NEXT:  %8 = BranchInst %BB3
// CHECK-NEXT:function_end
