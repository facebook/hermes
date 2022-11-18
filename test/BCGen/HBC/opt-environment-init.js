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

// CHECK:function global() : undefined
// CHECK-NEXT:frame = [], globals = [foo]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %foo() : closure
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function foo(o) : closure
// CHECK-NEXT:frame = [cnt : number|bigint, flag, flag1, flag2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [flag]
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [flag1]
// CHECK-NEXT:  %2 = StoreFrameInst 0 : number, [cnt] : number|bigint
// CHECK-NEXT:  %3 = CallInst %o, undefined : undefined
// CHECK-NEXT:  %4 = StoreFrameInst undefined : undefined, [flag2]
// CHECK-NEXT:  %5 = CreateFunctionInst %""() : number|bigint
// CHECK-NEXT:  %6 = ReturnInst %5 : closure
// CHECK-NEXT:function_end

// CHECK:function ""() : number|bigint
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst [flag@foo]
// CHECK-NEXT:  %1 = CondBranchInst %0, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %2 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = PhiInst %0, %BB0, true : boolean, %BB2
// CHECK-NEXT:  %4 = StoreFrameInst %3, [flag@foo]
// CHECK-NEXT:  %5 = LoadFrameInst [flag1@foo]
// CHECK-NEXT:  %6 = CondBranchInst %5, %BB3, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %7 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = PhiInst %5, %BB1, 1 : number, %BB4
// CHECK-NEXT:  %9 = StoreFrameInst %8, [flag1@foo]
// CHECK-NEXT:  %10 = LoadFrameInst [flag2@foo]
// CHECK-NEXT:  %11 = CondBranchInst %10, %BB5, %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %12 = BranchInst %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %13 = PhiInst %10, %BB3, 2 : number, %BB6
// CHECK-NEXT:  %14 = StoreFrameInst %13, [flag2@foo]
// CHECK-NEXT:  %15 = LoadFrameInst [cnt@foo] : number|bigint
// CHECK-NEXT:  %16 = UnaryOperatorInst '++', %15 : number|bigint
// CHECK-NEXT:  %17 = StoreFrameInst %16 : number|bigint, [cnt@foo] : number|bigint
// CHECK-NEXT:  %18 = ReturnInst %16 : number|bigint
// CHECK-NEXT:function_end

// CHKLIR:function global() : undefined
// CHKLIR-NEXT:frame = [], globals = [foo]
// CHKLIR-NEXT:%BB0:
// CHKLIR-NEXT:  %0 = HBCCreateEnvironmentInst
// CHKLIR-NEXT:  %1 = HBCCreateFunctionInst %foo() : closure, %0
// CHKLIR-NEXT:  %2 = HBCGetGlobalObjectInst
// CHKLIR-NEXT:  %3 = StorePropertyLooseInst %1 : closure, %2 : object, "foo" : string
// CHKLIR-NEXT:  %4 = HBCLoadConstInst undefined : undefined
// CHKLIR-NEXT:  %5 = ReturnInst %4 : undefined
// CHKLIR-NEXT:function_end

// CHKLIR:function foo(o) : closure
// CHKLIR-NEXT:frame = [cnt : number|bigint, flag, flag1, flag2]
// CHKLIR-NEXT:%BB0:
// CHKLIR-NEXT:  %0 = HBCCreateEnvironmentInst
// CHKLIR-NEXT:  %1 = HBCLoadConstInst 0 : number
// CHKLIR-NEXT:  %2 = HBCStoreToEnvironmentInst %0, %1 : number, [cnt] : number|bigint
// CHKLIR-NEXT:  %3 = HBCLoadParamInst 1 : number
// CHKLIR-NEXT:  %4 = HBCLoadConstInst undefined : undefined
// CHKLIR-NEXT:  %5 = HBCCallNInst %3, %4 : undefined
// CHKLIR-NEXT:  %6 = HBCStoreToEnvironmentInst %0, %4 : undefined, [flag2]
// CHKLIR-NEXT:  %7 = HBCCreateFunctionInst %""() : number|bigint, %0
// CHKLIR-NEXT:  %8 = ReturnInst %7 : closure
// CHKLIR-NEXT:function_end

// CHKLIR:function ""() : number|bigint
// CHKLIR-NEXT:frame = []
// CHKLIR-NEXT:%BB0:
// CHKLIR-NEXT:  %0 = HBCLoadConstInst true : boolean
// CHKLIR-NEXT:  %1 = HBCLoadConstInst 1 : number
// CHKLIR-NEXT:  %2 = HBCLoadConstInst 2 : number
// CHKLIR-NEXT:  %3 = HBCResolveEnvironment %foo()
// CHKLIR-NEXT:  %4 = HBCLoadFromEnvironmentInst %3, [flag@foo]
// CHKLIR-NEXT:  %5 = CondBranchInst %4, %BB1, %BB2
// CHKLIR-NEXT:%BB2:
// CHKLIR-NEXT:  %6 = BranchInst %BB1
// CHKLIR-NEXT:%BB1:
// CHKLIR-NEXT:  %7 = PhiInst %4, %BB0, %0 : boolean, %BB2
// CHKLIR-NEXT:  %8 = HBCStoreToEnvironmentInst %3, %7, [flag@foo]
// CHKLIR-NEXT:  %9 = HBCLoadFromEnvironmentInst %3, [flag1@foo]
// CHKLIR-NEXT:  %10 = CondBranchInst %9, %BB3, %BB4
// CHKLIR-NEXT:%BB4:
// CHKLIR-NEXT:  %11 = BranchInst %BB3
// CHKLIR-NEXT:%BB3:
// CHKLIR-NEXT:  %12 = PhiInst %9, %BB1, %1 : number, %BB4
// CHKLIR-NEXT:  %13 = HBCStoreToEnvironmentInst %3, %12, [flag1@foo]
// CHKLIR-NEXT:  %14 = HBCLoadFromEnvironmentInst %3, [flag2@foo]
// CHKLIR-NEXT:  %15 = CondBranchInst %14, %BB5, %BB6
// CHKLIR-NEXT:%BB6:
// CHKLIR-NEXT:  %16 = BranchInst %BB5
// CHKLIR-NEXT:%BB5:
// CHKLIR-NEXT:  %17 = PhiInst %14, %BB3, %2 : number, %BB6
// CHKLIR-NEXT:  %18 = HBCStoreToEnvironmentInst %3, %17, [flag2@foo]
// CHKLIR-NEXT:  %19 = HBCLoadFromEnvironmentInst %3, [cnt@foo] : number|bigint
// CHKLIR-NEXT:  %20 = UnaryOperatorInst '++', %19
// CHKLIR-NEXT:  %21 = HBCStoreToEnvironmentInst %3, %20 : number|bigint, [cnt@foo] : number|bigint
// CHKLIR-NEXT:  %22 = ReturnInst %20 : number|bigint
// CHKLIR-NEXT:function_end
