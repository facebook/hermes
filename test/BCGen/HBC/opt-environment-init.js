/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -dump-ir %s | %FileCheckOrRegen --match-full-lines %s
// RUN: %hermesc -O -dump-lir %s | %FileCheckOrRegen --match-full-lines --check-prefix=CHKLIR %s
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

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1 : undefined
// CHECK-NEXT:frame = [], globals = [foo]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %foo#0#1()#2 : closure, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function foo#0#1(o)#2 : closure
// CHECK-NEXT:frame = [cnt#2 : number|bigint, flag#2, flag1#2, flag2#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{foo#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [flag#2], %0
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [flag1#2], %0
// CHECK-NEXT:  %3 = StoreFrameInst 0 : number, [cnt#2] : number|bigint, %0
// CHECK-NEXT:  %4 = CallInst %o, undefined : undefined
// CHECK-NEXT:  %5 = StoreFrameInst undefined : undefined, [flag2#2], %0
// CHECK-NEXT:  %6 = CreateFunctionInst %""#1#2()#3 : number|bigint, %0
// CHECK-NEXT:  %7 = ReturnInst %6 : closure
// CHECK-NEXT:function_end

// CHECK:function ""#1#2()#3 : number|bigint
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{""#1#2()#3}
// CHECK-NEXT:  %1 = LoadFrameInst [flag#2@foo], %0
// CHECK-NEXT:  %2 = CondBranchInst %1, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %3 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = PhiInst %1, %BB0, true : boolean, %BB2
// CHECK-NEXT:  %5 = StoreFrameInst %4, [flag#2@foo], %0
// CHECK-NEXT:  %6 = LoadFrameInst [flag1#2@foo], %0
// CHECK-NEXT:  %7 = CondBranchInst %6, %BB3, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %8 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %9 = PhiInst %6, %BB1, 1 : number, %BB4
// CHECK-NEXT:  %10 = StoreFrameInst %9, [flag1#2@foo], %0
// CHECK-NEXT:  %11 = LoadFrameInst [flag2#2@foo], %0
// CHECK-NEXT:  %12 = CondBranchInst %11, %BB5, %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %13 = BranchInst %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %14 = PhiInst %11, %BB3, 2 : number, %BB6
// CHECK-NEXT:  %15 = StoreFrameInst %14, [flag2#2@foo], %0
// CHECK-NEXT:  %16 = LoadFrameInst [cnt#2@foo] : number|bigint, %0
// CHECK-NEXT:  %17 = UnaryOperatorInst '++', %16 : number|bigint
// CHECK-NEXT:  %18 = StoreFrameInst %17 : number|bigint, [cnt#2@foo] : number|bigint, %0
// CHECK-NEXT:  %19 = ReturnInst %17 : number|bigint
// CHECK-NEXT:function_end

// CHKLIR:function global#0()#1 : undefined
// CHKLIR-NEXT:frame = [], globals = [foo]
// CHKLIR-NEXT:%BB0:
// CHKLIR-NEXT:  %0 = HBCCreateEnvironmentInst %S{global#0()#1}
// CHKLIR-NEXT:  %1 = HBCCreateFunctionInst %foo#0#1()#2 : closure, %0
// CHKLIR-NEXT:  %2 = HBCGetGlobalObjectInst
// CHKLIR-NEXT:  %3 = StorePropertyInst %1 : closure, %2 : object, "foo" : string
// CHKLIR-NEXT:  %4 = HBCLoadConstInst undefined : undefined
// CHKLIR-NEXT:  %5 = ReturnInst %4 : undefined
// CHKLIR-NEXT:function_end

// CHKLIR:function foo#0#1(o)#2 : closure
// CHKLIR-NEXT:frame = [cnt#2 : number|bigint, flag#2, flag1#2, flag2#2]
// CHKLIR-NEXT:%BB0:
// CHKLIR-NEXT:  %0 = HBCCreateEnvironmentInst %S{foo#0#1()#2}
// CHKLIR-NEXT:  %1 = HBCLoadConstInst 0 : number
// CHKLIR-NEXT:  %2 = HBCStoreToEnvironmentInst %0, %1 : number, [cnt#2] : number|bigint
// CHKLIR-NEXT:  %3 = HBCLoadParamInst 1 : number
// CHKLIR-NEXT:  %4 = HBCLoadConstInst undefined : undefined
// CHKLIR-NEXT:  %5 = HBCCallNInst %3, %4 : undefined
// CHKLIR-NEXT:  %6 = HBCStoreToEnvironmentInst %0, %4 : undefined, [flag2#2]
// CHKLIR-NEXT:  %7 = HBCCreateFunctionInst %""#1#2()#3 : number|bigint, %0
// CHKLIR-NEXT:  %8 = ReturnInst %7 : closure
// CHKLIR-NEXT:function_end

// CHKLIR:function ""#1#2()#3 : number|bigint
// CHKLIR-NEXT:frame = []
// CHKLIR-NEXT:%BB0:
// CHKLIR-NEXT:  %0 = HBCLoadConstInst true : boolean
// CHKLIR-NEXT:  %1 = HBCLoadConstInst 1 : number
// CHKLIR-NEXT:  %2 = HBCLoadConstInst 2 : number
// CHKLIR-NEXT:  %3 = HBCResolveEnvironment %S{foo#0#1()#2}, %S{""#1#2()#3}
// CHKLIR-NEXT:  %4 = HBCLoadFromEnvironmentInst %3, [flag#2@foo]
// CHKLIR-NEXT:  %5 = CondBranchInst %4, %BB1, %BB2
// CHKLIR-NEXT:%BB2:
// CHKLIR-NEXT:  %6 = BranchInst %BB1
// CHKLIR-NEXT:%BB1:
// CHKLIR-NEXT:  %7 = PhiInst %4, %BB0, %0 : boolean, %BB2
// CHKLIR-NEXT:  %8 = HBCStoreToEnvironmentInst %3, %7, [flag#2@foo]
// CHKLIR-NEXT:  %9 = HBCLoadFromEnvironmentInst %3, [flag1#2@foo]
// CHKLIR-NEXT:  %10 = CondBranchInst %9, %BB3, %BB4
// CHKLIR-NEXT:%BB4:
// CHKLIR-NEXT:  %11 = BranchInst %BB3
// CHKLIR-NEXT:%BB3:
// CHKLIR-NEXT:  %12 = PhiInst %9, %BB1, %1 : number, %BB4
// CHKLIR-NEXT:  %13 = HBCStoreToEnvironmentInst %3, %12, [flag1#2@foo]
// CHKLIR-NEXT:  %14 = HBCLoadFromEnvironmentInst %3, [flag2#2@foo]
// CHKLIR-NEXT:  %15 = CondBranchInst %14, %BB5, %BB6
// CHKLIR-NEXT:%BB6:
// CHKLIR-NEXT:  %16 = BranchInst %BB5
// CHKLIR-NEXT:%BB5:
// CHKLIR-NEXT:  %17 = PhiInst %14, %BB3, %2 : number, %BB6
// CHKLIR-NEXT:  %18 = HBCStoreToEnvironmentInst %3, %17, [flag2#2@foo]
// CHKLIR-NEXT:  %19 = HBCLoadFromEnvironmentInst %3, [cnt#2@foo] : number|bigint
// CHKLIR-NEXT:  %20 = UnaryOperatorInst '++', %19
// CHKLIR-NEXT:  %21 = HBCStoreToEnvironmentInst %3, %20 : number|bigint, [cnt#2@foo] : number|bigint
// CHKLIR-NEXT:  %22 = ReturnInst %20 : number|bigint
// CHKLIR-NEXT:function_end
