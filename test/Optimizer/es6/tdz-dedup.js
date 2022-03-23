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
//CHECK-NEXT:  %3 = AsNumberInst %2
//CHECK-NEXT:  %4 = UnaryOperatorInst '++', %3 : number
//CHECK-NEXT:  %5 = LoadFrameInst [x@check_after_check]
//CHECK-NEXT:  %6 = ThrowIfEmptyInst %5
//CHECK-NEXT:  %7 = StoreFrameInst %4, [x@check_after_check]
//CHECK-NEXT:  %8 = LoadFrameInst [p]
//CHECK-NEXT:  %9 = CondBranchInst %8, %BB1, %BB2
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %10 = LoadFrameInst [x@check_after_check]
//CHECK-NEXT:  %11 = ThrowIfEmptyInst %10
//CHECK-NEXT:  %12 = AsNumberInst %11
//CHECK-NEXT:  %13 = UnaryOperatorInst '++', %12 : number
//CHECK-NEXT:  %14 = LoadFrameInst [x@check_after_check]
//CHECK-NEXT:  %15 = ThrowIfEmptyInst %14
//CHECK-NEXT:  %16 = StoreFrameInst %13, [x@check_after_check]
//CHECK-NEXT:  %17 = BranchInst %BB3
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %18 = BranchInst %BB3
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %19 = LoadFrameInst [x@check_after_check]
//CHECK-NEXT:  %20 = ThrowIfEmptyInst %19
//CHECK-NEXT:  %21 = ReturnInst %20
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %22 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end

//CHKOPT-LABEL:function inner(p) : undefined|null|boolean|string|number|object|closure|regexp
//CHKOPT-NEXT:frame = [p]
//CHKOPT-NEXT:%BB0:
//CHKOPT-NEXT:  %0 = StoreFrameInst %p, [p]
//CHKOPT-NEXT:  %1 = LoadFrameInst [x@check_after_check]
//CHKOPT-NEXT:  %2 = ThrowIfEmptyInst %1
//CHKOPT-NEXT:  %3 = AsNumberInst %2 : undefined|null|boolean|string|number|object|closure|regexp
//CHKOPT-NEXT:  %4 = UnaryOperatorInst '++', %3 : number
//CHKOPT-NEXT:  %5 = StoreFrameInst %4 : number, [x@check_after_check]
//CHKOPT-NEXT:  %6 = LoadFrameInst [p]
//CHKOPT-NEXT:  %7 = CondBranchInst %6, %BB1, %BB2
//CHKOPT-NEXT:%BB1:
//CHKOPT-NEXT:  %8 = LoadFrameInst [x@check_after_check]
//CHKOPT-NEXT:  %9 = AsNumberInst %8
//CHKOPT-NEXT:  %10 = UnaryOperatorInst '++', %9 : number
//CHKOPT-NEXT:  %11 = StoreFrameInst %10 : number, [x@check_after_check]
//CHKOPT-NEXT:  %12 = BranchInst %BB3
//CHKOPT-NEXT:%BB2:
//CHKOPT-NEXT:  %13 = BranchInst %BB3
//CHKOPT-NEXT:%BB3:
//CHKOPT-NEXT:  %14 = LoadFrameInst [x@check_after_check]
//CHKOPT-NEXT:  %15 = ReturnInst %14
//CHKOPT-NEXT:%BB4:
//CHKOPT-NEXT:  %16 = ReturnInst undefined : undefined
//CHKOPT-NEXT:function_end
