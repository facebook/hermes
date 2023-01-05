/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

function load_only_capture(leak, foreach, n) {
    for(var i = 0; i < n; i++){
      leak.k = () => i;
    }
    return i;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global() : undefined
// CHECK-NEXT:frame = [], globals = [load_only_capture]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %load_only_capture() : number
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "load_only_capture" : string
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function load_only_capture(leak, foreach, n) : number
// CHECK-NEXT:frame = [i : number]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %leak
// CHECK-NEXT:  %1 = LoadParamInst %n
// CHECK-NEXT:  %2 = BinaryOperatorInst '<', 0 : number, %1
// CHECK-NEXT:  %3 = CondBranchInst %2 : boolean, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %4 = CreateFunctionInst %""() : number
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4 : closure, %0, "k" : string
// CHECK-NEXT:  %6 = LoadFrameInst [i] : number
// CHECK-NEXT:  %7 = UnaryOperatorInst '++', %6 : number
// CHECK-NEXT:  %8 = StoreFrameInst %7 : number, [i] : number
// CHECK-NEXT:  %9 = BinaryOperatorInst '<', %7 : number, %1
// CHECK-NEXT:  %10 = CondBranchInst %9 : boolean, %BB3, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %11 = LoadFrameInst [i] : number
// CHECK-NEXT:  %12 = ReturnInst %11 : number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %13 = StoreFrameInst 0 : number, [i] : number
// CHECK-NEXT:  %14 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %15 = StoreFrameInst 0 : number, [i] : number
// CHECK-NEXT:  %16 = BranchInst %BB4
// CHECK-NEXT:function_end

// CHECK:arrow ""() : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst [i@load_only_capture] : number
// CHECK-NEXT:  %1 = ReturnInst %0 : number
// CHECK-NEXT:function_end
