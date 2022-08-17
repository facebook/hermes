/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -Xenable-tdz -O0 -dump-ir %s | %FileCheck --match-full-lines %s
// RUN: %hermesc -Xenable-tdz -custom-opt=typeinference -custom-opt=tdzdedup -dump-ir %s | %FileCheck --match-full-lines --check-prefix=CHKOPT %s

function check_after_store(p) {
    let x = 10;
    if (p)
        return x;
    return 0;
}
//CHECK-LABEL:function check_after_store(p)
//CHECK-NEXT:frame = [x, p]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StoreFrameInst empty : empty, [x]
//CHECK-NEXT:  %1 = StoreFrameInst %p, [p]
//CHECK-NEXT:  %2 = StoreFrameInst 10 : number, [x]
//CHECK-NEXT:  %3 = LoadFrameInst [p]
//CHECK-NEXT:  %4 = CondBranchInst %3, %BB1, %BB2
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %5 = LoadFrameInst [x]
//CHECK-NEXT:  %6 = ThrowIfEmptyInst %5
//CHECK-NEXT:  %7 = ReturnInst %6
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %8 = BranchInst %BB3
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %9 = ReturnInst 0 : number
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %10 = BranchInst %BB3
//CHECK-NEXT:%BB5:
//CHECK-NEXT:  %11 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end

//CHKOPT-LABEL:function check_after_store(p) : undefined|number
//CHKOPT-NEXT:frame = [x : empty|number, p]
//CHKOPT-NEXT:%BB0:
//CHKOPT-NEXT:  %0 = StoreFrameInst empty : empty, [x] : empty|number
//CHKOPT-NEXT:  %1 = StoreFrameInst %p, [p]
//CHKOPT-NEXT:  %2 = StoreFrameInst 10 : number, [x] : empty|number
//CHKOPT-NEXT:  %3 = LoadFrameInst [p]
//CHKOPT-NEXT:  %4 = CondBranchInst %3, %BB1, %BB2
//CHKOPT-NEXT:%BB1:
//CHKOPT-NEXT:  %5 = LoadFrameInst [x] : empty|number
//CHKOPT-NEXT:  %6 = ReturnInst %5 : empty|number
//CHKOPT-NEXT:%BB2:
//CHKOPT-NEXT:  %7 = BranchInst %BB3
//CHKOPT-NEXT:%BB3:
//CHKOPT-NEXT:  %8 = ReturnInst 0 : number
//CHKOPT-NEXT:%BB4:
//CHKOPT-NEXT:  %9 = BranchInst %BB3
//CHKOPT-NEXT:%BB5:
//CHKOPT-NEXT:  %10 = ReturnInst undefined : undefined
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
//CHECK-NEXT:  %1 = LoadFrameInst [x@check_after_check]
//CHECK-NEXT:  %2 = ThrowIfEmptyInst %1
//CHECK-NEXT:  %3 = UnaryOperatorInst '++', %2
//CHECK-NEXT:  %4 = LoadFrameInst [x@check_after_check]
//CHECK-NEXT:  %5 = ThrowIfEmptyInst %4
//CHECK-NEXT:  %6 = StoreFrameInst %3, [x@check_after_check]
//CHECK-NEXT:  %7 = LoadFrameInst [p]
//CHECK-NEXT:  %8 = CondBranchInst %7, %BB1, %BB2
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %9 = LoadFrameInst [x@check_after_check]
//CHECK-NEXT:  %10 = ThrowIfEmptyInst %9
//CHECK-NEXT:  %11 = UnaryOperatorInst '++', %10
//CHECK-NEXT:  %12 = LoadFrameInst [x@check_after_check]
//CHECK-NEXT:  %13 = ThrowIfEmptyInst %12
//CHECK-NEXT:  %14 = StoreFrameInst %11, [x@check_after_check]
//CHECK-NEXT:  %15 = BranchInst %BB3
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %16 = BranchInst %BB3
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %17 = LoadFrameInst [x@check_after_check]
//CHECK-NEXT:  %18 = ThrowIfEmptyInst %17
//CHECK-NEXT:  %19 = ReturnInst %18
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %20 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end

//CHKOPT-LABEL:function inner(p) : undefined|null|boolean|string|number|bigint|object|closure|regexp
//CHKOPT-NEXT:frame = [p]
//CHKOPT-NEXT:%BB0:
//CHKOPT-NEXT:  %0 = StoreFrameInst %p, [p]
//CHKOPT-NEXT:  %1 = LoadFrameInst [x@check_after_check]
//CHKOPT-NEXT:  %2 = ThrowIfEmptyInst %1
//CHKOPT-NEXT:  %3 = UnaryOperatorInst '++', %2 : undefined|null|boolean|string|number|bigint|object|closure|regexp
//CHKOPT-NEXT:  %4 = StoreFrameInst %3 : number|bigint, [x@check_after_check]
//CHKOPT-NEXT:  %5 = LoadFrameInst [p]
//CHKOPT-NEXT:  %6 = CondBranchInst %5, %BB1, %BB2
//CHKOPT-NEXT:%BB1:
//CHKOPT-NEXT:  %7 = LoadFrameInst [x@check_after_check]
//CHKOPT-NEXT:  %8 = UnaryOperatorInst '++', %7
//CHKOPT-NEXT:  %9 = StoreFrameInst %8 : number|bigint, [x@check_after_check]
//CHKOPT-NEXT:  %10 = BranchInst %BB3
//CHKOPT-NEXT:%BB2:
//CHKOPT-NEXT:  %11 = BranchInst %BB3
//CHKOPT-NEXT:%BB3:
//CHKOPT-NEXT:  %12 = LoadFrameInst [x@check_after_check]
//CHKOPT-NEXT:  %13 = ReturnInst %12
//CHKOPT-NEXT:%BB4:
//CHKOPT-NEXT:  %14 = ReturnInst undefined : undefined
//CHKOPT-NEXT:function_end
