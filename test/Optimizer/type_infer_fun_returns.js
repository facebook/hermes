/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ir %s -O | %FileCheckOrRegen %s --match-full-lines

function g14(z) {
    var w = function () { return k * 1; }
    if (z > w()) {
        print (w() + 1);
        return {m : function () { w = function() { return false; }; }};
    }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global() : undefined
// CHECK-NEXT:frame = [], globals = [g14]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %g14() : undefined|object
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "g14" : string
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function g14(z) : undefined|object
// CHECK-NEXT:frame = [w : closure]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %w() : number
// CHECK-NEXT:  %1 = CallInst %0 : closure, undefined : undefined
// CHECK-NEXT:  %2 = BinaryOperatorInst '>', %z, %1 : number
// CHECK-NEXT:  %3 = CondBranchInst %2 : boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = StoreFrameInst %0 : closure, [w] : closure
// CHECK-NEXT:  %5 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %6 = LoadFrameInst [w] : closure
// CHECK-NEXT:  %7 = CallInst %6 : closure, undefined : undefined
// CHECK-NEXT:  %8 = BinaryOperatorInst '+', %7 : boolean|number, 1 : number
// CHECK-NEXT:  %9 = CallInst %5, undefined : undefined, %8 : number
// CHECK-NEXT:  %10 = CreateFunctionInst %m() : undefined
// CHECK-NEXT:  %11 = AllocObjectLiteralInst "m" : string, %10 : closure
// CHECK-NEXT:  %12 = ReturnInst %11 : object
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %13 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function w() : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "k" : string
// CHECK-NEXT:  %1 = BinaryOperatorInst '*', %0, 1 : number
// CHECK-NEXT:  %2 = ReturnInst %1 : number
// CHECK-NEXT:function_end

// CHECK:function m() : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %"w 1#"() : boolean
// CHECK-NEXT:  %1 = StoreFrameInst %0 : closure, [w@g14] : closure
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function "w 1#"() : boolean
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst false : boolean
// CHECK-NEXT:function_end
