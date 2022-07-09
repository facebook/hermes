/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -dump-ir %s | %FileCheck --match-full-lines %s
// RUN: %hermesc -O -dump-lir %s | %FileCheck --match-full-lines --check-prefix=CHKLIR %s
//
// Test optimizing out of unnecessary store undefined into a freshly created
// environment

function foo(o) {
    var cnt = 0;
    var flag, flag1;
    var flag2;
    o();
    // This assignment shouldn't be optimized out.
    flag2 = undefined;
    return function () {
        flag = flag || true;
        flag1 = flag1 || 1;
        flag2 = flag2 || 2;
        return ++cnt;
    }
}

//CHECK-LABEL:function foo(o) : closure
//CHECK-NEXT:frame = [cnt : number|bigint, flag, flag1, flag2]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [flag]
//CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [flag1]
//CHECK-NEXT:  %2 = StoreFrameInst 0 : number, [cnt] : number|bigint
//CHECK-NEXT:  %3 = CallInst %o, undefined : undefined
//CHECK-NEXT:  %4 = StoreFrameInst undefined : undefined, [flag2]
//CHECK-NEXT:  %5 = CreateFunctionInst %""() : number|bigint
//CHECK-NEXT:  %6 = ReturnInst %5 : closure
//CHECK-NEXT:function_end

//CHKLIR-LABEL:function foo(o) : closure
//CHKLIR-NEXT:frame = [cnt : number|bigint, flag, flag1, flag2]
//CHKLIR-NEXT:%BB0:
//CHKLIR-NEXT:  %0 = HBCCreateEnvironmentInst
//CHKLIR-NEXT:  %1 = HBCLoadConstInst 0 : number
//CHKLIR-NEXT:  %2 = HBCStoreToEnvironmentInst %0, %1 : number, [cnt] : number|bigint
//CHKLIR-NEXT:  %3 = HBCLoadParamInst 1 : number
//CHKLIR-NEXT:  %4 = HBCLoadConstInst undefined : undefined
//CHKLIR-NEXT:  %5 = HBCCallNInst %3, %4 : undefined
//CHKLIR-NEXT:  %6 = HBCStoreToEnvironmentInst %0, %4 : undefined, [flag2]
//CHKLIR-NEXT:  %7 = HBCCreateFunctionInst %""() : number|bigint, %0
//CHKLIR-NEXT:  %8 = ReturnInst %7 : closure
//CHKLIR-NEXT:function_end
