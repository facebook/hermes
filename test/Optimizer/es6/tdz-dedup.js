/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -Xenable-tdz -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s
// RUN: %hermesc -Xenable-tdz -custom-opt=typeinference -custom-opt=tdzdedup -dump-ir %s | %FileCheckOrRegen --match-full-lines --check-prefix=CHKOPT %s

function check_after_store(p) {
    let x = 10;
    if (p)
        return x;
    return 0;
}

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

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "check_after_store" : string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "check_after_check" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %check_after_store()
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2 : closure, globalObject : object, "check_after_store" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %check_after_check()
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4 : closure, globalObject : object, "check_after_check" : string
// CHECK-NEXT:  %6 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %7 = StoreStackInst undefined : undefined, %6
// CHECK-NEXT:  %8 = LoadStackInst %6
// CHECK-NEXT:  %9 = ReturnInst %8
// CHECK-NEXT:function_end

// CHECK:function check_after_store(p)
// CHECK-NEXT:frame = [p, x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %p
// CHECK-NEXT:  %1 = StoreFrameInst %0, [p]
// CHECK-NEXT:  %2 = StoreFrameInst empty : empty, [x]
// CHECK-NEXT:  %3 = StoreFrameInst 10 : number, [x]
// CHECK-NEXT:  %4 = LoadFrameInst [p]
// CHECK-NEXT:  %5 = CondBranchInst %4, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = LoadFrameInst [x]
// CHECK-NEXT:  %7 = ThrowIfEmptyInst %6
// CHECK-NEXT:  %8 = ReturnInst %7
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %9 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %10 = ReturnInst 0 : number
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %11 = BranchInst %BB3
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %12 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function check_after_check()
// CHECK-NEXT:frame = [inner, x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst empty : empty, [x]
// CHECK-NEXT:  %1 = CreateFunctionInst %inner()
// CHECK-NEXT:  %2 = StoreFrameInst %1 : closure, [inner]
// CHECK-NEXT:  %3 = StoreFrameInst 0 : number, [x]
// CHECK-NEXT:  %4 = LoadFrameInst [inner]
// CHECK-NEXT:  %5 = ReturnInst %4
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function inner(p)
// CHECK-NEXT:frame = [p]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %p
// CHECK-NEXT:  %1 = StoreFrameInst %0, [p]
// CHECK-NEXT:  %2 = LoadFrameInst [x@check_after_check]
// CHECK-NEXT:  %3 = ThrowIfEmptyInst %2
// CHECK-NEXT:  %4 = UnaryOperatorInst '++', %3
// CHECK-NEXT:  %5 = LoadFrameInst [x@check_after_check]
// CHECK-NEXT:  %6 = ThrowIfEmptyInst %5
// CHECK-NEXT:  %7 = StoreFrameInst %4, [x@check_after_check]
// CHECK-NEXT:  %8 = LoadFrameInst [p]
// CHECK-NEXT:  %9 = CondBranchInst %8, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = LoadFrameInst [x@check_after_check]
// CHECK-NEXT:  %11 = ThrowIfEmptyInst %10
// CHECK-NEXT:  %12 = UnaryOperatorInst '++', %11
// CHECK-NEXT:  %13 = LoadFrameInst [x@check_after_check]
// CHECK-NEXT:  %14 = ThrowIfEmptyInst %13
// CHECK-NEXT:  %15 = StoreFrameInst %12, [x@check_after_check]
// CHECK-NEXT:  %16 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %17 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %18 = LoadFrameInst [x@check_after_check]
// CHECK-NEXT:  %19 = ThrowIfEmptyInst %18
// CHECK-NEXT:  %20 = ReturnInst %19
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %21 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHKOPT:function global() : undefined
// CHKOPT-NEXT:frame = []
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  %0 = DeclareGlobalVarInst "check_after_store" : string
// CHKOPT-NEXT:  %1 = DeclareGlobalVarInst "check_after_check" : string
// CHKOPT-NEXT:  %2 = CreateFunctionInst %check_after_store() : empty|undefined|number
// CHKOPT-NEXT:  %3 = StorePropertyLooseInst %2 : closure, globalObject : object, "check_after_store" : string
// CHKOPT-NEXT:  %4 = CreateFunctionInst %check_after_check() : undefined|closure
// CHKOPT-NEXT:  %5 = StorePropertyLooseInst %4 : closure, globalObject : object, "check_after_check" : string
// CHKOPT-NEXT:  %6 = AllocStackInst $?anon_0_ret
// CHKOPT-NEXT:  %7 = StoreStackInst undefined : undefined, %6 : undefined
// CHKOPT-NEXT:  %8 = LoadStackInst %6 : undefined
// CHKOPT-NEXT:  %9 = ReturnInst %8 : undefined
// CHKOPT-NEXT:function_end

// CHKOPT:function check_after_store(p) : empty|undefined|number
// CHKOPT-NEXT:frame = [p, x : empty|number]
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  %0 = LoadParamInst %p
// CHKOPT-NEXT:  %1 = StoreFrameInst %0, [p]
// CHKOPT-NEXT:  %2 = StoreFrameInst empty : empty, [x] : empty|number
// CHKOPT-NEXT:  %3 = StoreFrameInst 10 : number, [x] : empty|number
// CHKOPT-NEXT:  %4 = LoadFrameInst [p]
// CHKOPT-NEXT:  %5 = CondBranchInst %4, %BB1, %BB2
// CHKOPT-NEXT:%BB1:
// CHKOPT-NEXT:  %6 = LoadFrameInst [x] : empty|number
// CHKOPT-NEXT:  %7 = ReturnInst %6 : empty|number
// CHKOPT-NEXT:%BB2:
// CHKOPT-NEXT:  %8 = BranchInst %BB3
// CHKOPT-NEXT:%BB3:
// CHKOPT-NEXT:  %9 = ReturnInst 0 : number
// CHKOPT-NEXT:%BB4:
// CHKOPT-NEXT:  %10 = BranchInst %BB3
// CHKOPT-NEXT:%BB5:
// CHKOPT-NEXT:  %11 = ReturnInst undefined : undefined
// CHKOPT-NEXT:function_end

// CHKOPT:function check_after_check() : undefined|closure
// CHKOPT-NEXT:frame = [inner : closure, x]
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  %0 = StoreFrameInst empty : empty, [x]
// CHKOPT-NEXT:  %1 = CreateFunctionInst %inner()
// CHKOPT-NEXT:  %2 = StoreFrameInst %1 : closure, [inner] : closure
// CHKOPT-NEXT:  %3 = StoreFrameInst 0 : number, [x]
// CHKOPT-NEXT:  %4 = LoadFrameInst [inner] : closure
// CHKOPT-NEXT:  %5 = ReturnInst %4 : closure
// CHKOPT-NEXT:%BB1:
// CHKOPT-NEXT:  %6 = ReturnInst undefined : undefined
// CHKOPT-NEXT:function_end

// CHKOPT:function inner(p)
// CHKOPT-NEXT:frame = [p]
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  %0 = LoadParamInst %p
// CHKOPT-NEXT:  %1 = StoreFrameInst %0, [p]
// CHKOPT-NEXT:  %2 = LoadFrameInst [x@check_after_check]
// CHKOPT-NEXT:  %3 = ThrowIfEmptyInst %2
// CHKOPT-NEXT:  %4 = UnaryOperatorInst '++', %3
// CHKOPT-NEXT:  %5 = StoreFrameInst %4 : number|bigint, [x@check_after_check]
// CHKOPT-NEXT:  %6 = LoadFrameInst [p]
// CHKOPT-NEXT:  %7 = CondBranchInst %6, %BB1, %BB2
// CHKOPT-NEXT:%BB1:
// CHKOPT-NEXT:  %8 = LoadFrameInst [x@check_after_check]
// CHKOPT-NEXT:  %9 = UnaryOperatorInst '++', %8
// CHKOPT-NEXT:  %10 = StoreFrameInst %9 : number|bigint, [x@check_after_check]
// CHKOPT-NEXT:  %11 = BranchInst %BB3
// CHKOPT-NEXT:%BB2:
// CHKOPT-NEXT:  %12 = BranchInst %BB3
// CHKOPT-NEXT:%BB3:
// CHKOPT-NEXT:  %13 = LoadFrameInst [x@check_after_check]
// CHKOPT-NEXT:  %14 = ReturnInst %13
// CHKOPT-NEXT:%BB4:
// CHKOPT-NEXT:  %15 = ReturnInst undefined : undefined
// CHKOPT-NEXT:function_end
