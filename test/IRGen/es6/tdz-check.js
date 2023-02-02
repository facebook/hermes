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

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "check1" : string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "check2" : string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "check3" : string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "check4" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %check1()
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4 : closure, globalObject : object, "check1" : string
// CHECK-NEXT:  %6 = CreateFunctionInst %check2()
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6 : closure, globalObject : object, "check2" : string
// CHECK-NEXT:  %8 = CreateFunctionInst %check3()
// CHECK-NEXT:  %9 = StorePropertyLooseInst %8 : closure, globalObject : object, "check3" : string
// CHECK-NEXT:  %10 = CreateFunctionInst %check4()
// CHECK-NEXT:  %11 = StorePropertyLooseInst %10 : closure, globalObject : object, "check4" : string
// CHECK-NEXT:  %12 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %13 = StoreStackInst undefined : undefined, %12
// CHECK-NEXT:  %14 = LoadStackInst %12
// CHECK-NEXT:  %15 = ReturnInst %14
// CHECK-NEXT:function_end

// CHECK:function check1()
// CHECK-NEXT:frame = [x, y]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst empty : empty, [x]
// CHECK-NEXT:  %1 = StoreFrameInst empty : empty, [y]
// CHECK-NEXT:  %2 = LoadFrameInst [x]
// CHECK-NEXT:  %3 = ThrowIfEmptyInst %2
// CHECK-NEXT:  %4 = LoadFrameInst [y]
// CHECK-NEXT:  %5 = ThrowIfEmptyInst %4
// CHECK-NEXT:  %6 = BinaryOperatorInst '+', %3, %5
// CHECK-NEXT:  %7 = ReturnInst %6
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = StoreFrameInst 10 : number, [x]
// CHECK-NEXT:  %9 = StoreFrameInst 1 : number, [y]
// CHECK-NEXT:  %10 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function check2(p)
// CHECK-NEXT:frame = [p, b, a]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %p
// CHECK-NEXT:  %1 = StoreFrameInst %0, [p]
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [b]
// CHECK-NEXT:  %3 = StoreFrameInst empty : empty, [a]
// CHECK-NEXT:  %4 = LoadFrameInst [a]
// CHECK-NEXT:  %5 = ThrowIfEmptyInst %4
// CHECK-NEXT:  %6 = StoreFrameInst %5, [b]
// CHECK-NEXT:  %7 = StoreFrameInst undefined : undefined, [a]
// CHECK-NEXT:  %8 = LoadFrameInst [a]
// CHECK-NEXT:  %9 = ThrowIfEmptyInst %8
// CHECK-NEXT:  %10 = LoadFrameInst [b]
// CHECK-NEXT:  %11 = BinaryOperatorInst '+', %9, %10
// CHECK-NEXT:  %12 = ReturnInst %11
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %13 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function check3()
// CHECK-NEXT:frame = [x, check3_inner]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst empty : empty, [x]
// CHECK-NEXT:  %1 = CreateFunctionInst %check3_inner()
// CHECK-NEXT:  %2 = StoreFrameInst %1 : closure, [check3_inner]
// CHECK-NEXT:  %3 = LoadFrameInst [check3_inner]
// CHECK-NEXT:  %4 = CallInst %3, undefined : undefined
// CHECK-NEXT:  %5 = StoreFrameInst %4, [x]
// CHECK-NEXT:  %6 = LoadFrameInst [x]
// CHECK-NEXT:  %7 = ThrowIfEmptyInst %6
// CHECK-NEXT:  %8 = ReturnInst %7
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function check4()
// CHECK-NEXT:frame = [x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst empty : empty, [x]
// CHECK-NEXT:  %1 = LoadFrameInst [x]
// CHECK-NEXT:  %2 = ThrowIfEmptyInst %1
// CHECK-NEXT:  %3 = StoreFrameInst 10 : number, [x]
// CHECK-NEXT:  %4 = StoreFrameInst undefined : undefined, [x]
// CHECK-NEXT:  %5 = LoadFrameInst [x]
// CHECK-NEXT:  %6 = ThrowIfEmptyInst %5
// CHECK-NEXT:  %7 = ReturnInst %6
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function check3_inner()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst [x@check3]
// CHECK-NEXT:  %1 = ThrowIfEmptyInst %0
// CHECK-NEXT:  %2 = BinaryOperatorInst '+', %1, 1 : number
// CHECK-NEXT:  %3 = ReturnInst %2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
