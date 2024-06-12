/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Xdump-functions=f1,f2,inner -Xcustom-opt=tdzdedup -dump-ir --test262 %s | %FileCheckOrRegen --check-prefix=CHKIR --match-full-lines %s
// RUN: %shermes -Xdump-functions=f1,f2,inner -Xcustom-opt=tdzdedup -dump-lir --test262 %s | %FileCheckOrRegen --check-prefix=CHKLIR --match-full-lines %s

// Verify that LIRDeadValueInst is emitted after TDZDedup has eliminated a ThrowIfEmpty
function f1() {
    x;
    return x + 1;
    let x;
}

// Verify that the result type of UnionArrowTrusted is copied to the input.
function f2() {
    let x;
    function inner() {
        x = 10;
        return x;
    }
}

// Auto-generated content below. Please do not modify manually.

// CHKIR:scope %VS0 []

// CHKIR:scope %VS1 [x: any|empty]

// CHKIR:function f1(): any
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHKIR-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHKIR-NEXT:       StoreFrameInst %1: environment, empty: empty, [%VS1.x]: any|empty
// CHKIR-NEXT:  %3 = ThrowIfInst (:any) empty: empty, type(empty)
// CHKIR-NEXT:  %4 = UnionNarrowTrustedInst (:any) empty: empty
// CHKIR-NEXT:  %5 = BinaryAddInst (:any) %4: any, 1: number
// CHKIR-NEXT:       ReturnInst %5: any
// CHKIR-NEXT:function_end

// CHKIR:scope %VS2 [x: any|empty, inner: any]

// CHKIR:function f2(): any
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHKIR-NEXT:  %1 = CreateScopeInst (:environment) %VS2: any, %0: environment
// CHKIR-NEXT:       StoreFrameInst %1: environment, empty: empty, [%VS2.x]: any|empty
// CHKIR-NEXT:  %3 = CreateFunctionInst (:object) %1: environment, %inner(): functionCode
// CHKIR-NEXT:       StoreFrameInst %1: environment, %3: object, [%VS2.inner]: any
// CHKIR-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS2.x]: any|empty
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:scope %VS3 []

// CHKIR:function inner(): any
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = GetParentScopeInst (:environment) %VS2: any, %parentScope: environment
// CHKIR-NEXT:  %1 = CreateScopeInst (:environment) %VS3: any, %0: environment
// CHKIR-NEXT:  %2 = LoadFrameInst (:any|empty) %0: environment, [%VS2.x]: any|empty
// CHKIR-NEXT:  %3 = ThrowIfInst (:any) %2: any|empty, type(empty)
// CHKIR-NEXT:       StoreFrameInst %0: environment, 10: number, [%VS2.x]: any|empty
// CHKIR-NEXT:  %5 = LoadFrameInst (:any|empty) %0: environment, [%VS2.x]: any|empty
// CHKIR-NEXT:  %6 = UnionNarrowTrustedInst (:any) %5: any|empty
// CHKIR-NEXT:       ReturnInst %6: any
// CHKIR-NEXT:function_end

// CHKLIR:scope %VS0 []

// CHKLIR:scope %VS1 [x: empty]

// CHKLIR:function f1(): string|number
// CHKLIR-NEXT:%BB0:
// CHKLIR-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHKLIR-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHKLIR-NEXT:  %2 = HBCLoadConstInst (:empty) empty: empty
// CHKLIR-NEXT:       StoreFrameInst %1: environment, %2: empty, [%VS1.x]: empty
// CHKLIR-NEXT:  %4 = ThrowIfInst (:any) %2: empty, type(empty)
// CHKLIR-NEXT:  %5 = UnionNarrowTrustedInst (:any) %2: empty
// CHKLIR-NEXT:  %6 = HBCLoadConstInst (:number) 1: number
// CHKLIR-NEXT:  %7 = BinaryAddInst (:string|number) %5: any, %6: number
// CHKLIR-NEXT:       ReturnInst %7: string|number
// CHKLIR-NEXT:function_end

// CHKLIR:scope %VS2 [x: empty|undefined|number, inner: object]

// CHKLIR:function f2(): undefined
// CHKLIR-NEXT:%BB0:
// CHKLIR-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHKLIR-NEXT:  %1 = CreateScopeInst (:environment) %VS2: any, %0: environment
// CHKLIR-NEXT:  %2 = HBCLoadConstInst (:empty) empty: empty
// CHKLIR-NEXT:       StoreFrameInst %1: environment, %2: empty, [%VS2.x]: empty|undefined|number
// CHKLIR-NEXT:  %4 = CreateFunctionInst (:object) %1: environment, %inner(): functionCode
// CHKLIR-NEXT:       StoreFrameInst %1: environment, %4: object, [%VS2.inner]: object
// CHKLIR-NEXT:  %6 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKLIR-NEXT:       StoreFrameInst %1: environment, %6: undefined, [%VS2.x]: empty|undefined|number
// CHKLIR-NEXT:       ReturnInst %6: undefined
// CHKLIR-NEXT:function_end

// CHKLIR:function inner(): undefined|number
// CHKLIR-NEXT:%BB0:
// CHKLIR-NEXT:  %0 = GetParentScopeInst (:environment) %VS2: any, %parentScope: environment
// CHKLIR-NEXT:  %1 = LoadFrameInst (:empty|undefined|number) %0: environment, [%VS2.x]: empty|undefined|number
// CHKLIR-NEXT:  %2 = ThrowIfInst (:undefined|number) %1: empty|undefined|number, type(empty)
// CHKLIR-NEXT:  %3 = HBCLoadConstInst (:number) 10: number
// CHKLIR-NEXT:       StoreFrameInst %0: environment, %3: number, [%VS2.x]: empty|undefined|number
// CHKLIR-NEXT:  %5 = LoadFrameInst (:empty|undefined|number) %0: environment, [%VS2.x]: empty|undefined|number
// CHKLIR-NEXT:  %6 = UnionNarrowTrustedInst (:undefined|number) %5: empty|undefined|number
// CHKLIR-NEXT:       ReturnInst %6: undefined|number
// CHKLIR-NEXT:function_end
