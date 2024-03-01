/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -O0 --test262 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s
// RUN: %shermes -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines --check-prefix=CHKDIS %s

function check1() {
    glob = function inner() {
        ++x;
        return y;
    }
    return x + y;
    let x = 10;
    const y = 1;
    return x + y;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "check1": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %check1(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "check1": string
// CHECK-NEXT:  %4 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %4: any
// CHECK-NEXT:  %6 = LoadStackInst (:any) %4: any
// CHECK-NEXT:       ReturnInst %6: any
// CHECK-NEXT:function_end

// CHECK:function check1(): any
// CHECK-NEXT:frame = [x: any|empty, y: any|empty, inner: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %check1(): any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, empty: empty, [x]: any|empty
// CHECK-NEXT:       StoreFrameInst %1: environment, empty: empty, [y]: any|empty
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %1: environment, %inner(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: object, [inner]: any
// CHECK-NEXT:       StorePropertyLooseInst %4: object, globalObject: object, "glob": string
// CHECK-NEXT:  %7 = ThrowIfInst (:any) empty: empty, type(empty)
// CHECK-NEXT:  %8 = ThrowIfInst (:any) empty: empty, type(empty)
// CHECK-NEXT:  %9 = BinaryAddInst (:any) %7: any, %8: any
// CHECK-NEXT:        ReturnInst %9: any
// CHECK-NEXT:function_end

// CHECK:function inner(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %check1(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %inner(): any, %0: environment
// CHECK-NEXT:  %2 = ResolveScopeInst (:environment) %check1(): any, %1: environment
// CHECK-NEXT:  %3 = LoadFrameInst (:any|empty) %2: environment, [x@check1]: any|empty
// CHECK-NEXT:  %4 = ThrowIfInst (:any) %3: any|empty, type(empty)
// CHECK-NEXT:  %5 = UnaryIncInst (:number|bigint) %4: any
// CHECK-NEXT:  %6 = ResolveScopeInst (:environment) %check1(): any, %1: environment
// CHECK-NEXT:  %7 = LoadFrameInst (:any|empty) %6: environment, [x@check1]: any|empty
// CHECK-NEXT:  %8 = ThrowIfInst (:any) %7: any|empty, type(empty)
// CHECK-NEXT:       StoreFrameInst %6: environment, %5: number|bigint, [x@check1]: any|empty
// CHECK-NEXT:  %10 = ResolveScopeInst (:environment) %check1(): any, %1: environment
// CHECK-NEXT:  %11 = LoadFrameInst (:any|empty) %10: environment, [y@check1]: any|empty
// CHECK-NEXT:  %12 = ThrowIfInst (:any) %11: any|empty, type(empty)
// CHECK-NEXT:        ReturnInst %12: any
// CHECK-NEXT:function_end

// CHKDIS:function global(): any
// CHKDIS-NEXT:frame = []
// CHKDIS-NEXT:%BB0:
// CHKDIS-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHKDIS-NEXT:       DeclareGlobalVarInst "check1": string
// CHKDIS-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %check1(): functionCode
// CHKDIS-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "check1": string
// CHKDIS-NEXT:  %4 = AllocStackInst (:any) $?anon_0_ret: any
// CHKDIS-NEXT:       StoreStackInst undefined: undefined, %4: any
// CHKDIS-NEXT:  %6 = LoadStackInst (:any) %4: any
// CHKDIS-NEXT:       ReturnInst %6: any
// CHKDIS-NEXT:function_end

// CHKDIS:function check1(): any
// CHKDIS-NEXT:frame = [x: any, y: any, inner: any]
// CHKDIS-NEXT:%BB0:
// CHKDIS-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHKDIS-NEXT:  %1 = CreateScopeInst (:environment) %check1(): any, %0: environment
// CHKDIS-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [x]: any
// CHKDIS-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [y]: any
// CHKDIS-NEXT:  %4 = CreateFunctionInst (:object) %1: environment, %inner(): functionCode
// CHKDIS-NEXT:       StoreFrameInst %1: environment, %4: object, [inner]: any
// CHKDIS-NEXT:       StorePropertyLooseInst %4: object, globalObject: object, "glob": string
// CHKDIS-NEXT:  %7 = LoadFrameInst (:any) %1: environment, [x]: any
// CHKDIS-NEXT:  %8 = LoadFrameInst (:any) %1: environment, [y]: any
// CHKDIS-NEXT:  %9 = BinaryAddInst (:any) %7: any, %8: any
// CHKDIS-NEXT:        ReturnInst %9: any
// CHKDIS-NEXT:function_end

// CHKDIS:function inner(): any
// CHKDIS-NEXT:frame = []
// CHKDIS-NEXT:%BB0:
// CHKDIS-NEXT:  %0 = GetParentScopeInst (:environment) %check1(): any, %parentScope: environment
// CHKDIS-NEXT:  %1 = CreateScopeInst (:environment) %inner(): any, %0: environment
// CHKDIS-NEXT:  %2 = ResolveScopeInst (:environment) %check1(): any, %1: environment
// CHKDIS-NEXT:  %3 = LoadFrameInst (:any) %2: environment, [x@check1]: any
// CHKDIS-NEXT:  %4 = UnaryIncInst (:number|bigint) %3: any
// CHKDIS-NEXT:  %5 = ResolveScopeInst (:environment) %check1(): any, %1: environment
// CHKDIS-NEXT:       StoreFrameInst %5: environment, %4: number|bigint, [x@check1]: any
// CHKDIS-NEXT:  %7 = ResolveScopeInst (:environment) %check1(): any, %1: environment
// CHKDIS-NEXT:  %8 = LoadFrameInst (:any) %7: environment, [y@check1]: any
// CHKDIS-NEXT:       ReturnInst %8: any
// CHKDIS-NEXT:function_end
