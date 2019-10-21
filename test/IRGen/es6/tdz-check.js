/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir %s | %FileCheck --match-full-lines %s

function check1() {
    return x + y;
    let x = 10;
    const y = 1;
}
//CHECK-LABEL:function check1()
//CHECK-NEXT:frame = [x, ?anon_0_tdz$x, y, ?anon_1_tdz$y]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [x]
//CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [?anon_0_tdz$x]
//CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [y]
//CHECK-NEXT:  %3 = StoreFrameInst undefined : undefined, [?anon_1_tdz$y]
//CHECK-NEXT:  %4 = LoadFrameInst [?anon_0_tdz$x]
//CHECK-NEXT:  %5 = ThrowIfUndefinedInst %4
//CHECK-NEXT:  %6 = LoadFrameInst [x]
//CHECK-NEXT:  %7 = LoadFrameInst [?anon_1_tdz$y]
//CHECK-NEXT:  %8 = ThrowIfUndefinedInst %7
//CHECK-NEXT:  %9 = LoadFrameInst [y]
//CHECK-NEXT:  %10 = BinaryOperatorInst '+', %6, %9
//CHECK-NEXT:  %11 = ReturnInst %10
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %12 = StoreFrameInst 10 : number, [x]
//CHECK-NEXT:  %13 = StoreFrameInst true : boolean, [?anon_0_tdz$x]
//CHECK-NEXT:  %14 = StoreFrameInst 1 : number, [y]
//CHECK-NEXT:  %15 = StoreFrameInst true : boolean, [?anon_1_tdz$y]
//CHECK-NEXT:  %16 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end

function check2(p) {
    var b = a;
    let a;
    return a + b;
}
//CHECK-LABEL:function check2(p)
//CHECK-NEXT:frame = [b, a, ?anon_0_tdz$a, p]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [b]
//CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [a]
//CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [?anon_0_tdz$a]
//CHECK-NEXT:  %3 = StoreFrameInst %p, [p]
//CHECK-NEXT:  %4 = LoadFrameInst [?anon_0_tdz$a]
//CHECK-NEXT:  %5 = ThrowIfUndefinedInst %4
//CHECK-NEXT:  %6 = LoadFrameInst [a]
//CHECK-NEXT:  %7 = StoreFrameInst %6, [b]
//CHECK-NEXT:  %8 = StoreFrameInst undefined : undefined, [a]
//CHECK-NEXT:  %9 = StoreFrameInst true : boolean, [?anon_0_tdz$a]
//CHECK-NEXT:  %10 = LoadFrameInst [?anon_0_tdz$a]
//CHECK-NEXT:  %11 = ThrowIfUndefinedInst %10
//CHECK-NEXT:  %12 = LoadFrameInst [a]
//CHECK-NEXT:  %13 = LoadFrameInst [b]
//CHECK-NEXT:  %14 = BinaryOperatorInst '+', %12, %13
//CHECK-NEXT:  %15 = ReturnInst %14
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %16 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end

function check3() {
    let x = check3_inner();
    function check3_inner() {
        return x + 1;
    }
    return x;
}
//CHECK-LABEL:function check3()
//CHECK-NEXT:frame = [x, ?anon_0_tdz$x, check3_inner]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [x]
//CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [?anon_0_tdz$x]
//CHECK-NEXT:  %2 = CreateFunctionInst %check3_inner()
//CHECK-NEXT:  %3 = StoreFrameInst %2 : closure, [check3_inner]
//CHECK-NEXT:  %4 = LoadFrameInst [check3_inner]
//CHECK-NEXT:  %5 = CallInst %4, undefined : undefined
//CHECK-NEXT:  %6 = StoreFrameInst %5, [x]
//CHECK-NEXT:  %7 = StoreFrameInst true : boolean, [?anon_0_tdz$x]
//CHECK-NEXT:  %8 = LoadFrameInst [?anon_0_tdz$x]
//CHECK-NEXT:  %9 = ThrowIfUndefinedInst %8
//CHECK-NEXT:  %10 = LoadFrameInst [x]
//CHECK-NEXT:  %11 = ReturnInst %10
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %12 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end

//CHECK-LABEL:function check3_inner()
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = LoadFrameInst [?anon_0_tdz$x@check3]
//CHECK-NEXT:  %1 = ThrowIfUndefinedInst %0
//CHECK-NEXT:  %2 = LoadFrameInst [x@check3]
//CHECK-NEXT:  %3 = BinaryOperatorInst '+', %2, 1 : number
//CHECK-NEXT:  %4 = ReturnInst %3
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %5 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end

function check4() {
    x = 10;
    let x;
    return x;
}
//CHECK-LABEL:function check4()
//CHECK-NEXT:frame = [x, ?anon_0_tdz$x]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [x]
//CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [?anon_0_tdz$x]
//CHECK-NEXT:  %2 = LoadFrameInst [?anon_0_tdz$x]
//CHECK-NEXT:  %3 = ThrowIfUndefinedInst %2
//CHECK-NEXT:  %4 = StoreFrameInst 10 : number, [x]
//CHECK-NEXT:  %5 = StoreFrameInst undefined : undefined, [x]
//CHECK-NEXT:  %6 = StoreFrameInst true : boolean, [?anon_0_tdz$x]
//CHECK-NEXT:  %7 = LoadFrameInst [?anon_0_tdz$x]
//CHECK-NEXT:  %8 = ThrowIfUndefinedInst %7
//CHECK-NEXT:  %9 = LoadFrameInst [x]
//CHECK-NEXT:  %10 = ReturnInst %9
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %11 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
