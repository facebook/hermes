/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheck --match-full-lines %s
// RUN: %hermesc -custom-opt=tdzdedup -dump-ir %s | %FileCheck --match-full-lines --check-prefix=CHKOPT %s

function check_after_store(p) {
    let x = 10;
    if (p)
        return x;
    return 0;
}
//CHECK-LABEL:function check_after_store(p)
//CHECK-NEXT:frame = [x, ?anon_0_tdz$x, p]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [x]
//CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [?anon_0_tdz$x]
//CHECK-NEXT:  %2 = StoreFrameInst %p, [p]
//CHECK-NEXT:  %3 = StoreFrameInst 10 : number, [x]
//CHECK-NEXT:  %4 = StoreFrameInst true : boolean, [?anon_0_tdz$x]
//CHECK-NEXT:  %5 = LoadFrameInst [p]
//CHECK-NEXT:  %6 = CondBranchInst %5, %BB1, %BB2
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %7 = LoadFrameInst [?anon_0_tdz$x]
//CHECK-NEXT:  %8 = ThrowIfUndefinedInst %7
//CHECK-NEXT:  %9 = LoadFrameInst [x]
//CHECK-NEXT:  %10 = ReturnInst %9
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %11 = BranchInst %BB3
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %12 = ReturnInst 0 : number
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %13 = BranchInst %BB3
//CHECK-NEXT:%BB5:
//CHECK-NEXT:  %14 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end

//CHKOPT-LABEL:function check_after_store(p)
//CHKOPT-NEXT:frame = [x, ?anon_0_tdz$x, p]
//CHKOPT-NEXT:%BB0:
//CHKOPT-NEXT:  %0 = StoreFrameInst undefined : undefined, [x]
//CHKOPT-NEXT:  %1 = StoreFrameInst undefined : undefined, [?anon_0_tdz$x]
//CHKOPT-NEXT:  %2 = StoreFrameInst %p, [p]
//CHKOPT-NEXT:  %3 = StoreFrameInst 10 : number, [x]
//CHKOPT-NEXT:  %4 = StoreFrameInst true : boolean, [?anon_0_tdz$x]
//CHKOPT-NEXT:  %5 = LoadFrameInst [p]
//CHKOPT-NEXT:  %6 = CondBranchInst %5, %BB1, %BB2
//CHKOPT-NEXT:%BB1:
//CHKOPT-NEXT:  %7 = LoadFrameInst [x]
//CHKOPT-NEXT:  %8 = ReturnInst %7
//CHKOPT-NEXT:%BB2:
//CHKOPT-NEXT:  %9 = BranchInst %BB3
//CHKOPT-NEXT:%BB3:
//CHKOPT-NEXT:  %10 = ReturnInst 0 : number
//CHKOPT-NEXT:%BB4:
//CHKOPT-NEXT:  %11 = BranchInst %BB3
//CHKOPT-NEXT:%BB5:
//CHKOPT-NEXT:  %12 = ReturnInst undefined : undefined
//CHKOPT-NEXT:function_end

function check_after_check() {
    function inner(p) {
        ++x;
        if (p)
            ++x;
        return x;
    }
    let x = 0;
    return inner;
}
//CHECK-LABEL:function inner(p)
//CHECK-NEXT:frame = [p]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StoreFrameInst %p, [p]
//CHECK-NEXT:  %1 = LoadFrameInst [?anon_0_tdz$x@check_after_check]
//CHECK-NEXT:  %2 = ThrowIfUndefinedInst %1
//CHECK-NEXT:  %3 = LoadFrameInst [x@check_after_check]
//CHECK-NEXT:  %4 = AsNumberInst %3
//CHECK-NEXT:  %5 = BinaryOperatorInst '+', %4 : number, 1 : number
//CHECK-NEXT:  %6 = LoadFrameInst [?anon_0_tdz$x@check_after_check]
//CHECK-NEXT:  %7 = ThrowIfUndefinedInst %6
//CHECK-NEXT:  %8 = StoreFrameInst %5, [x@check_after_check]
//CHECK-NEXT:  %9 = LoadFrameInst [p]
//CHECK-NEXT:  %10 = CondBranchInst %9, %BB1, %BB2
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %11 = LoadFrameInst [?anon_0_tdz$x@check_after_check]
//CHECK-NEXT:  %12 = ThrowIfUndefinedInst %11
//CHECK-NEXT:  %13 = LoadFrameInst [x@check_after_check]
//CHECK-NEXT:  %14 = AsNumberInst %13
//CHECK-NEXT:  %15 = BinaryOperatorInst '+', %14 : number, 1 : number
//CHECK-NEXT:  %16 = LoadFrameInst [?anon_0_tdz$x@check_after_check]
//CHECK-NEXT:  %17 = ThrowIfUndefinedInst %16
//CHECK-NEXT:  %18 = StoreFrameInst %15, [x@check_after_check]
//CHECK-NEXT:  %19 = BranchInst %BB3
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %20 = BranchInst %BB3
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %21 = LoadFrameInst [?anon_0_tdz$x@check_after_check]
//CHECK-NEXT:  %22 = ThrowIfUndefinedInst %21
//CHECK-NEXT:  %23 = LoadFrameInst [x@check_after_check]
//CHECK-NEXT:  %24 = ReturnInst %23
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %25 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end

//CHKOPT-LABEL:function inner(p)
//CHKOPT-NEXT:frame = [p]
//CHKOPT-NEXT:%BB0:
//CHKOPT-NEXT:  %0 = StoreFrameInst %p, [p]
//CHKOPT-NEXT:  %1 = LoadFrameInst [?anon_0_tdz$x@check_after_check]
//CHKOPT-NEXT:  %2 = ThrowIfUndefinedInst %1
//CHKOPT-NEXT:  %3 = LoadFrameInst [x@check_after_check]
//CHKOPT-NEXT:  %4 = AsNumberInst %3
//CHKOPT-NEXT:  %5 = BinaryOperatorInst '+', %4 : number, 1 : number
//CHKOPT-NEXT:  %6 = StoreFrameInst %5, [x@check_after_check]
//CHKOPT-NEXT:  %7 = LoadFrameInst [p]
//CHKOPT-NEXT:  %8 = CondBranchInst %7, %BB1, %BB2
//CHKOPT-NEXT:%BB1:
//CHKOPT-NEXT:  %9 = LoadFrameInst [x@check_after_check]
//CHKOPT-NEXT:  %10 = AsNumberInst %9
//CHKOPT-NEXT:  %11 = BinaryOperatorInst '+', %10 : number, 1 : number
//CHKOPT-NEXT:  %12 = StoreFrameInst %11, [x@check_after_check]
//CHKOPT-NEXT:  %13 = BranchInst %BB3
//CHKOPT-NEXT:%BB2:
//CHKOPT-NEXT:  %14 = BranchInst %BB3
//CHKOPT-NEXT:%BB3:
//CHKOPT-NEXT:  %15 = LoadFrameInst [x@check_after_check]
//CHKOPT-NEXT:  %16 = ReturnInst %15
//CHKOPT-NEXT:%BB4:
//CHKOPT-NEXT:  %17 = ReturnInst undefined : undefined
//CHKOPT-NEXT:function_end
