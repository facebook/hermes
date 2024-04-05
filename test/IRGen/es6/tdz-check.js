/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes --test262 -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s
// RUN: %shermes --test262 -Xcustom-opt=simplestackpromotion -dump-ir %s > /dev/null

// Note that we are passing --test262 to both enable TDZ and to delay TDZ errors
// until runtime.

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

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "check1": string
// CHECK-NEXT:       DeclareGlobalVarInst "check2": string
// CHECK-NEXT:       DeclareGlobalVarInst "check3": string
// CHECK-NEXT:       DeclareGlobalVarInst "check4": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %0: environment, %check1(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %5: object, globalObject: object, "check1": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %0: environment, %check2(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %7: object, globalObject: object, "check2": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %0: environment, %check3(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %9: object, globalObject: object, "check3": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %0: environment, %check4(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %11: object, globalObject: object, "check4": string
// CHECK-NEXT:  %13 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %13: any
// CHECK-NEXT:  %15 = LoadStackInst (:any) %13: any
// CHECK-NEXT:        ReturnInst %15: any
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [x: any|empty, y: any|empty]

// CHECK:function check1(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, empty: empty, [%VS1.x]: any|empty
// CHECK-NEXT:       StoreFrameInst %1: environment, empty: empty, [%VS1.y]: any|empty
// CHECK-NEXT:  %4 = ThrowIfInst (:any) empty: empty, type(empty)
// CHECK-NEXT:  %5 = ThrowIfInst (:any) empty: empty, type(empty)
// CHECK-NEXT:  %6 = BinaryAddInst (:any) %4: any, %5: any
// CHECK-NEXT:       ReturnInst %6: any
// CHECK-NEXT:function_end

// CHECK:scope %VS2 [p: any, b: any, a: any|empty]

// CHECK:function check2(p: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS2: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %p: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS2.p]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS2.b]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, empty: empty, [%VS2.a]: any|empty
// CHECK-NEXT:  %6 = ThrowIfInst (:any) empty: empty, type(empty)
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: any, [%VS2.b]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS2.a]: any|empty
// CHECK-NEXT:  %9 = LoadFrameInst (:any|empty) %1: environment, [%VS2.a]: any|empty
// CHECK-NEXT:  %10 = UnionNarrowTrustedInst (:any) %9: any|empty
// CHECK-NEXT:  %11 = LoadFrameInst (:any) %1: environment, [%VS2.b]: any
// CHECK-NEXT:  %12 = BinaryAddInst (:any) %10: any, %11: any
// CHECK-NEXT:        ReturnInst %12: any
// CHECK-NEXT:function_end

// CHECK:scope %VS3 [x: any|empty, check3_inner: any]

// CHECK:function check3(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS3: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, empty: empty, [%VS3.x]: any|empty
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS3.check3_inner]: any
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %1: environment, %check3_inner(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: object, [%VS3.check3_inner]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [%VS3.check3_inner]: any
// CHECK-NEXT:  %7 = CallInst (:any) %6: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       StoreFrameInst %1: environment, %7: any, [%VS3.x]: any|empty
// CHECK-NEXT:  %9 = LoadFrameInst (:any|empty) %1: environment, [%VS3.x]: any|empty
// CHECK-NEXT:  %10 = UnionNarrowTrustedInst (:any) %9: any|empty
// CHECK-NEXT:        ReturnInst %10: any
// CHECK-NEXT:function_end

// CHECK:scope %VS4 [x: any|empty]

// CHECK:function check4(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS4: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, empty: empty, [%VS4.x]: any|empty
// CHECK-NEXT:  %3 = ThrowIfInst (:undefined) empty: empty, type(empty)
// CHECK-NEXT:       StoreFrameInst %1: environment, 10: number, [%VS4.x]: any|empty
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS4.x]: any|empty
// CHECK-NEXT:  %6 = LoadFrameInst (:any|empty) %1: environment, [%VS4.x]: any|empty
// CHECK-NEXT:  %7 = UnionNarrowTrustedInst (:any) %6: any|empty
// CHECK-NEXT:       ReturnInst %7: any
// CHECK-NEXT:function_end

// CHECK:scope %VS5 []

// CHECK:function check3_inner(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS3: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS5: any, %0: environment
// CHECK-NEXT:  %2 = ResolveScopeInst (:environment) %VS3: any, %VS5: any, %1: environment
// CHECK-NEXT:  %3 = LoadFrameInst (:any|empty) %2: environment, [%VS3.x]: any|empty
// CHECK-NEXT:  %4 = ThrowIfInst (:any) %3: any|empty, type(empty)
// CHECK-NEXT:  %5 = BinaryAddInst (:any) %4: any, 1: number
// CHECK-NEXT:       ReturnInst %5: any
// CHECK-NEXT:function_end
