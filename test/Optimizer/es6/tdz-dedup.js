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

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [check_after_store, check_after_check]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %check_after_store#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "check_after_store" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %check_after_check#0#1()#3, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "check_after_check" : string
// CHECK-NEXT:  %5 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %6 = StoreStackInst undefined : undefined, %5
// CHECK-NEXT:  %7 = LoadStackInst %5
// CHECK-NEXT:  %8 = ReturnInst %7
// CHECK-NEXT:function_end

// CHECK:function check_after_store#0#1(p)#2
// CHECK-NEXT:frame = [p#2, x#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{check_after_store#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst %p, [p#2], %0
// CHECK-NEXT:  %2 = StoreFrameInst empty : empty, [x#2], %0
// CHECK-NEXT:  %3 = StoreFrameInst 10 : number, [x#2], %0
// CHECK-NEXT:  %4 = LoadFrameInst [p#2], %0
// CHECK-NEXT:  %5 = CondBranchInst %4, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = LoadFrameInst [x#2], %0
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

// CHECK:function check_after_check#0#1()#3
// CHECK-NEXT:frame = [x#3, inner#3]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{check_after_check#0#1()#3}
// CHECK-NEXT:  %1 = StoreFrameInst empty : empty, [x#3], %0
// CHECK-NEXT:  %2 = CreateFunctionInst %inner#1#3()#4, %0
// CHECK-NEXT:  %3 = StoreFrameInst %2 : closure, [inner#3], %0
// CHECK-NEXT:  %4 = StoreFrameInst 0 : number, [x#3], %0
// CHECK-NEXT:  %5 = LoadFrameInst [inner#3], %0
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function inner#1#3(p)#4
// CHECK-NEXT:frame = [p#4]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{inner#1#3()#4}
// CHECK-NEXT:  %1 = StoreFrameInst %p, [p#4], %0
// CHECK-NEXT:  %2 = LoadFrameInst [x#3@check_after_check], %0
// CHECK-NEXT:  %3 = ThrowIfEmptyInst %2
// CHECK-NEXT:  %4 = UnaryOperatorInst '++', %3
// CHECK-NEXT:  %5 = LoadFrameInst [x#3@check_after_check], %0
// CHECK-NEXT:  %6 = ThrowIfEmptyInst %5
// CHECK-NEXT:  %7 = StoreFrameInst %4, [x#3@check_after_check], %0
// CHECK-NEXT:  %8 = LoadFrameInst [p#4], %0
// CHECK-NEXT:  %9 = CondBranchInst %8, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = LoadFrameInst [x#3@check_after_check], %0
// CHECK-NEXT:  %11 = ThrowIfEmptyInst %10
// CHECK-NEXT:  %12 = UnaryOperatorInst '++', %11
// CHECK-NEXT:  %13 = LoadFrameInst [x#3@check_after_check], %0
// CHECK-NEXT:  %14 = ThrowIfEmptyInst %13
// CHECK-NEXT:  %15 = StoreFrameInst %12, [x#3@check_after_check], %0
// CHECK-NEXT:  %16 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %17 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %18 = LoadFrameInst [x#3@check_after_check], %0
// CHECK-NEXT:  %19 = ThrowIfEmptyInst %18
// CHECK-NEXT:  %20 = ReturnInst %19
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %21 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHKOPT:function global#0()#1 : undefined
// CHKOPT-NEXT:frame = [], globals = [check_after_store, check_after_check]
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHKOPT-NEXT:  %1 = CreateFunctionInst %check_after_store#0#1()#2 : undefined|number, %0
// CHKOPT-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "check_after_store" : string
// CHKOPT-NEXT:  %3 = CreateFunctionInst %check_after_check#0#1()#3 : undefined|closure, %0
// CHKOPT-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "check_after_check" : string
// CHKOPT-NEXT:  %5 = AllocStackInst $?anon_0_ret
// CHKOPT-NEXT:  %6 = StoreStackInst undefined : undefined, %5 : undefined
// CHKOPT-NEXT:  %7 = LoadStackInst %5 : undefined
// CHKOPT-NEXT:  %8 = ReturnInst %7 : undefined
// CHKOPT-NEXT:function_end

// CHKOPT:function check_after_store#0#1(p)#2 : undefined|number
// CHKOPT-NEXT:frame = [p#2, x#2 : empty|number]
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  %0 = CreateScopeInst %S{check_after_store#0#1()#2}
// CHKOPT-NEXT:  %1 = StoreFrameInst %p, [p#2], %0
// CHKOPT-NEXT:  %2 = StoreFrameInst empty : empty, [x#2] : empty|number, %0
// CHKOPT-NEXT:  %3 = StoreFrameInst 10 : number, [x#2] : empty|number, %0
// CHKOPT-NEXT:  %4 = LoadFrameInst [p#2], %0
// CHKOPT-NEXT:  %5 = CondBranchInst %4, %BB1, %BB2
// CHKOPT-NEXT:%BB1:
// CHKOPT-NEXT:  %6 = LoadFrameInst [x#2] : empty|number, %0
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

// CHKOPT:function check_after_check#0#1()#3 : undefined|closure
// CHKOPT-NEXT:frame = [x#3, inner#3 : closure]
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  %0 = CreateScopeInst %S{check_after_check#0#1()#3}
// CHKOPT-NEXT:  %1 = StoreFrameInst empty : empty, [x#3], %0
// CHKOPT-NEXT:  %2 = CreateFunctionInst %inner#1#3()#4 : undefined|null|boolean|string|number|bigint|object|closure|regexp, %0
// CHKOPT-NEXT:  %3 = StoreFrameInst %2 : closure, [inner#3] : closure, %0
// CHKOPT-NEXT:  %4 = StoreFrameInst 0 : number, [x#3], %0
// CHKOPT-NEXT:  %5 = LoadFrameInst [inner#3] : closure, %0
// CHKOPT-NEXT:  %6 = ReturnInst %5 : closure
// CHKOPT-NEXT:%BB1:
// CHKOPT-NEXT:  %7 = ReturnInst undefined : undefined
// CHKOPT-NEXT:function_end

// CHKOPT:function inner#1#3(p)#4 : undefined|null|boolean|string|number|bigint|object|closure|regexp
// CHKOPT-NEXT:frame = [p#4]
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  %0 = CreateScopeInst %S{inner#1#3()#4}
// CHKOPT-NEXT:  %1 = StoreFrameInst %p, [p#4], %0
// CHKOPT-NEXT:  %2 = LoadFrameInst [x#3@check_after_check], %0
// CHKOPT-NEXT:  %3 = ThrowIfEmptyInst %2
// CHKOPT-NEXT:  %4 = UnaryOperatorInst '++', %3 : undefined|null|boolean|string|number|bigint|object|closure|regexp
// CHKOPT-NEXT:  %5 = StoreFrameInst %4 : number|bigint, [x#3@check_after_check], %0
// CHKOPT-NEXT:  %6 = LoadFrameInst [p#4], %0
// CHKOPT-NEXT:  %7 = CondBranchInst %6, %BB1, %BB2
// CHKOPT-NEXT:%BB1:
// CHKOPT-NEXT:  %8 = LoadFrameInst [x#3@check_after_check], %0
// CHKOPT-NEXT:  %9 = UnaryOperatorInst '++', %8
// CHKOPT-NEXT:  %10 = StoreFrameInst %9 : number|bigint, [x#3@check_after_check], %0
// CHKOPT-NEXT:  %11 = BranchInst %BB3
// CHKOPT-NEXT:%BB2:
// CHKOPT-NEXT:  %12 = BranchInst %BB3
// CHKOPT-NEXT:%BB3:
// CHKOPT-NEXT:  %13 = LoadFrameInst [x#3@check_after_check], %0
// CHKOPT-NEXT:  %14 = ReturnInst %13
// CHKOPT-NEXT:%BB4:
// CHKOPT-NEXT:  %15 = ReturnInst undefined : undefined
// CHKOPT-NEXT:function_end
