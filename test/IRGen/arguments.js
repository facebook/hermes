/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir -non-strict %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir -non-strict %s -O

// Code that doesn't require creation of the expensive object.
function cheap() {
    return arguments.length + arguments[0];
}

// Code that requires creation of the expensive object.
function expensive() {
    return arguments.length + cheap(arguments) + arguments[0];
}

// Something with more than one BB
function cond() {
    if (arguments.length)
        return arguments[1];
}

function shadow(arguments) {
    return arguments.length;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [cheap, expensive, cond, shadow]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %cheap#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "cheap" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %expensive#0#1()#3, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "expensive" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %cond#0#1()#4, %0
// CHECK-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "cond" : string
// CHECK-NEXT:  %7 = CreateFunctionInst %shadow#0#1()#5, %0
// CHECK-NEXT:  %8 = StorePropertyInst %7 : closure, globalObject : object, "shadow" : string
// CHECK-NEXT:  %9 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %10 = StoreStackInst undefined : undefined, %9
// CHECK-NEXT:  %11 = LoadStackInst %9
// CHECK-NEXT:  %12 = ReturnInst %11
// CHECK-NEXT:function_end

// CHECK:function cheap#0#1()#2
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{cheap#0#1()#2}
// CHECK-NEXT:  %1 = CreateArgumentsInst
// CHECK-NEXT:  %2 = LoadPropertyInst %1 : object, "length" : string
// CHECK-NEXT:  %3 = LoadPropertyInst %1 : object, 0 : number
// CHECK-NEXT:  %4 = BinaryOperatorInst '+', %2, %3
// CHECK-NEXT:  %5 = ReturnInst %4
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function expensive#0#1()#3
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{expensive#0#1()#3}
// CHECK-NEXT:  %1 = CreateArgumentsInst
// CHECK-NEXT:  %2 = LoadPropertyInst %1 : object, "length" : string
// CHECK-NEXT:  %3 = LoadPropertyInst globalObject : object, "cheap" : string
// CHECK-NEXT:  %4 = CallInst %3, undefined : undefined, %1 : object
// CHECK-NEXT:  %5 = BinaryOperatorInst '+', %2, %4
// CHECK-NEXT:  %6 = LoadPropertyInst %1 : object, 0 : number
// CHECK-NEXT:  %7 = BinaryOperatorInst '+', %5, %6
// CHECK-NEXT:  %8 = ReturnInst %7
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function cond#0#1()#4
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{cond#0#1()#4}
// CHECK-NEXT:  %1 = CreateArgumentsInst
// CHECK-NEXT:  %2 = LoadPropertyInst %1 : object, "length" : string
// CHECK-NEXT:  %3 = CondBranchInst %2, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = LoadPropertyInst %1 : object, 1 : number
// CHECK-NEXT:  %5 = ReturnInst %4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %8 = BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function shadow#0#1(arguments)#5
// CHECK-NEXT:frame = [arguments#5]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{shadow#0#1()#5}
// CHECK-NEXT:  %1 = StoreFrameInst %arguments, [arguments#5], %0
// CHECK-NEXT:  %2 = LoadFrameInst [arguments#5], %0
// CHECK-NEXT:  %3 = LoadPropertyInst %2, "length" : string
// CHECK-NEXT:  %4 = ReturnInst %3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
