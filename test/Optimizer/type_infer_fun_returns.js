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

// CHECK:function global#0()#1 : undefined
// CHECK-NEXT:frame = [], globals = [g14]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %g14#0#1()#2 : undefined|object, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "g14" : string
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function g14#0#1(z)#2 : undefined|object
// CHECK-NEXT:frame = [w#2 : closure]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{g14#0#1()#2}
// CHECK-NEXT:  %1 = CreateFunctionInst %w#1#2()#3 : number, %0
// CHECK-NEXT:  %2 = CallInst %1 : closure, undefined : undefined
// CHECK-NEXT:  %3 = BinaryOperatorInst '>', %z, %2 : number
// CHECK-NEXT:  %4 = CondBranchInst %3 : boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = StoreFrameInst %1 : closure, [w#2] : closure, %0
// CHECK-NEXT:  %6 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %7 = LoadFrameInst [w#2] : closure, %0
// CHECK-NEXT:  %8 = CallInst %7 : closure, undefined : undefined
// CHECK-NEXT:  %9 = BinaryOperatorInst '+', %8 : boolean|number, 1 : number
// CHECK-NEXT:  %10 = CallInst %6, undefined : undefined, %9 : number
// CHECK-NEXT:  %11 = CreateFunctionInst %m#1#2()#4 : undefined, %0
// CHECK-NEXT:  %12 = AllocObjectLiteralInst "m" : string, %11 : closure
// CHECK-NEXT:  %13 = ReturnInst %12 : object
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %14 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function w#1#2()#3 : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{w#1#2()#3}
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst globalObject : object, "k" : string
// CHECK-NEXT:  %2 = BinaryOperatorInst '*', %1, 1 : number
// CHECK-NEXT:  %3 = ReturnInst %2 : number
// CHECK-NEXT:function_end

// CHECK:function m#1#2()#4 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{m#1#2()#4}
// CHECK-NEXT:  %1 = CreateFunctionInst %"w 1#"#2#4()#5 : boolean, %0
// CHECK-NEXT:  %2 = StoreFrameInst %1 : closure, [w#2@g14] : closure, %0
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function "w 1#"#2#4()#5 : boolean
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{"w 1#"#2#4()#5}
// CHECK-NEXT:  %1 = ReturnInst false : boolean
// CHECK-NEXT:function_end
