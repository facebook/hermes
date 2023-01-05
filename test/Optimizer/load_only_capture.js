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
// CHECK-NEXT:  %2 = StoreFrameInst 0 : number, [i] : number
// CHECK-NEXT:  %3 = BinaryOperatorInst '<', 0 : number, %1
// CHECK-NEXT:  %4 = CondBranchInst %3 : boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = PhiInst 0 : number, %BB0, %8 : number, %BB1
// CHECK-NEXT:  %6 = CreateFunctionInst %""() : number
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6 : closure, %0, "k" : string
// CHECK-NEXT:  %8 = UnaryOperatorInst '++', %5 : number
// CHECK-NEXT:  %9 = StoreFrameInst %8 : number, [i] : number
// CHECK-NEXT:  %10 = BinaryOperatorInst '<', %8 : number, %1
// CHECK-NEXT:  %11 = CondBranchInst %10 : boolean, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %12 = PhiInst 0 : number, %BB0, %8 : number, %BB1
// CHECK-NEXT:  %13 = ReturnInst %12 : number
// CHECK-NEXT:function_end

// CHECK:arrow ""() : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst [i@load_only_capture] : number
// CHECK-NEXT:  %1 = ReturnInst %0 : number
// CHECK-NEXT:function_end
