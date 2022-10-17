/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -Xenable-tdz -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

function check1() {
    return x + y;
    let x = 10;
    const y = 1;
}

function check2(p) {
    var b = a;
    let a;
    return a + b;
}

function check3() {
    let x = check3_inner();
    function check3_inner() {
        return x + 1;
    }
    return x;
}

function check4() {
    x = 10;
    let x;
    return x;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [check1, check2, check3, check4]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %check1#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "check1" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %check2#0#1()#3, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "check2" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %check3#0#1()#4, %0
// CHECK-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "check3" : string
// CHECK-NEXT:  %7 = CreateFunctionInst %check4#0#1()#6, %0
// CHECK-NEXT:  %8 = StorePropertyInst %7 : closure, globalObject : object, "check4" : string
// CHECK-NEXT:  %9 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %10 = StoreStackInst undefined : undefined, %9
// CHECK-NEXT:  %11 = LoadStackInst %9
// CHECK-NEXT:  %12 = ReturnInst %11
// CHECK-NEXT:function_end

// CHECK:function check1#0#1()#2
// CHECK-NEXT:frame = [x#2, y#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{check1#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst empty : empty, [x#2], %0
// CHECK-NEXT:  %2 = StoreFrameInst empty : empty, [y#2], %0
// CHECK-NEXT:  %3 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %4 = ThrowIfEmptyInst %3
// CHECK-NEXT:  %5 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %6 = ThrowIfEmptyInst %5
// CHECK-NEXT:  %7 = BinaryOperatorInst '+', %4, %6
// CHECK-NEXT:  %8 = ReturnInst %7
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = StoreFrameInst 10 : number, [x#2], %0
// CHECK-NEXT:  %10 = StoreFrameInst 1 : number, [y#2], %0
// CHECK-NEXT:  %11 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function check2#0#1(p)#3
// CHECK-NEXT:frame = [p#3, b#3, a#3]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{check2#0#1()#3}
// CHECK-NEXT:  %1 = StoreFrameInst %p, [p#3], %0
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [b#3], %0
// CHECK-NEXT:  %3 = StoreFrameInst empty : empty, [a#3], %0
// CHECK-NEXT:  %4 = LoadFrameInst [a#3], %0
// CHECK-NEXT:  %5 = ThrowIfEmptyInst %4
// CHECK-NEXT:  %6 = StoreFrameInst %5, [b#3], %0
// CHECK-NEXT:  %7 = StoreFrameInst undefined : undefined, [a#3], %0
// CHECK-NEXT:  %8 = LoadFrameInst [a#3], %0
// CHECK-NEXT:  %9 = ThrowIfEmptyInst %8
// CHECK-NEXT:  %10 = LoadFrameInst [b#3], %0
// CHECK-NEXT:  %11 = BinaryOperatorInst '+', %9, %10
// CHECK-NEXT:  %12 = ReturnInst %11
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %13 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function check3#0#1()#4
// CHECK-NEXT:frame = [x#4, check3_inner#4]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{check3#0#1()#4}
// CHECK-NEXT:  %1 = StoreFrameInst empty : empty, [x#4], %0
// CHECK-NEXT:  %2 = CreateFunctionInst %check3_inner#1#4()#5, %0
// CHECK-NEXT:  %3 = StoreFrameInst %2 : closure, [check3_inner#4], %0
// CHECK-NEXT:  %4 = LoadFrameInst [check3_inner#4], %0
// CHECK-NEXT:  %5 = CallInst %4, undefined : undefined
// CHECK-NEXT:  %6 = StoreFrameInst %5, [x#4], %0
// CHECK-NEXT:  %7 = LoadFrameInst [x#4], %0
// CHECK-NEXT:  %8 = ThrowIfEmptyInst %7
// CHECK-NEXT:  %9 = ReturnInst %8
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function check3_inner#1#4()#5
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{check3_inner#1#4()#5}
// CHECK-NEXT:  %1 = LoadFrameInst [x#4@check3], %0
// CHECK-NEXT:  %2 = ThrowIfEmptyInst %1
// CHECK-NEXT:  %3 = BinaryOperatorInst '+', %2, 1 : number
// CHECK-NEXT:  %4 = ReturnInst %3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function check4#0#1()#6
// CHECK-NEXT:frame = [x#6]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{check4#0#1()#6}
// CHECK-NEXT:  %1 = StoreFrameInst empty : empty, [x#6], %0
// CHECK-NEXT:  %2 = LoadFrameInst [x#6], %0
// CHECK-NEXT:  %3 = ThrowIfEmptyInst %2
// CHECK-NEXT:  %4 = StoreFrameInst 10 : number, [x#6], %0
// CHECK-NEXT:  %5 = StoreFrameInst undefined : undefined, [x#6], %0
// CHECK-NEXT:  %6 = LoadFrameInst [x#6], %0
// CHECK-NEXT:  %7 = ThrowIfEmptyInst %6
// CHECK-NEXT:  %8 = ReturnInst %7
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
