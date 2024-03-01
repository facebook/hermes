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

// CHKIR:function f1(): any
// CHKIR-NEXT:frame = [x: any|empty]
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHKIR-NEXT:  %1 = CreateScopeInst (:environment) %f1(): any, %0: environment
// CHKIR-NEXT:       StoreFrameInst %1: environment, empty: empty, [x]: any|empty
// CHKIR-NEXT:  %3 = ThrowIfInst (:any) empty: empty, type(empty)
// CHKIR-NEXT:  %4 = UnionNarrowTrustedInst (:any) empty: empty
// CHKIR-NEXT:  %5 = BinaryAddInst (:any) %4: any, 1: number
// CHKIR-NEXT:       ReturnInst %5: any
// CHKIR-NEXT:function_end

// CHKIR:function f2(): any
// CHKIR-NEXT:frame = [x: any|empty, inner: any]
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHKIR-NEXT:  %1 = CreateScopeInst (:environment) %f2(): any, %0: environment
// CHKIR-NEXT:       StoreFrameInst %1: environment, empty: empty, [x]: any|empty
// CHKIR-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [inner]: any
// CHKIR-NEXT:  %4 = CreateFunctionInst (:object) %1: environment, %inner(): functionCode
// CHKIR-NEXT:       StoreFrameInst %1: environment, %4: object, [inner]: any
// CHKIR-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [x]: any|empty
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function inner(): any
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = GetParentScopeInst (:environment) %f2(): any, %parentScope: environment
// CHKIR-NEXT:  %1 = CreateScopeInst (:environment) %inner(): any, %0: environment
// CHKIR-NEXT:  %2 = ResolveScopeInst (:environment) %f2(): any, %1: environment
// CHKIR-NEXT:  %3 = LoadFrameInst (:any|empty) %2: environment, [x@f2]: any|empty
// CHKIR-NEXT:  %4 = ThrowIfInst (:any) %3: any|empty, type(empty)
// CHKIR-NEXT:       StoreFrameInst %2: environment, 10: number, [x@f2]: any|empty
// CHKIR-NEXT:  %6 = ResolveScopeInst (:environment) %f2(): any, %1: environment
// CHKIR-NEXT:  %7 = LoadFrameInst (:any|empty) %6: environment, [x@f2]: any|empty
// CHKIR-NEXT:  %8 = UnionNarrowTrustedInst (:any) %7: any|empty
// CHKIR-NEXT:       ReturnInst %8: any
// CHKIR-NEXT:function_end

// CHKLIR:function f1(): string|number
// CHKLIR-NEXT:frame = [x: empty]
// CHKLIR-NEXT:%BB0:
// CHKLIR-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHKLIR-NEXT:  %1 = CreateScopeInst (:environment) %f1(): any, %0: environment
// CHKLIR-NEXT:  %2 = HBCLoadConstInst (:empty) empty: empty
// CHKLIR-NEXT:       StoreFrameInst %1: environment, %2: empty, [x]: empty
// CHKLIR-NEXT:  %4 = ThrowIfInst (:any) %2: empty, type(empty)
// CHKLIR-NEXT:  %5 = UnionNarrowTrustedInst (:any) %2: empty
// CHKLIR-NEXT:  %6 = HBCLoadConstInst (:number) 1: number
// CHKLIR-NEXT:  %7 = BinaryAddInst (:string|number) %5: any, %6: number
// CHKLIR-NEXT:       ReturnInst %7: string|number
// CHKLIR-NEXT:function_end

// CHKLIR:function f2(): undefined
// CHKLIR-NEXT:frame = [x: empty|undefined|number, inner: undefined|object]
// CHKLIR-NEXT:%BB0:
// CHKLIR-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHKLIR-NEXT:  %1 = CreateScopeInst (:environment) %f2(): any, %0: environment
// CHKLIR-NEXT:  %2 = HBCLoadConstInst (:empty) empty: empty
// CHKLIR-NEXT:       StoreFrameInst %1: environment, %2: empty, [x]: empty|undefined|number
// CHKLIR-NEXT:  %4 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKLIR-NEXT:       StoreFrameInst %1: environment, %4: undefined, [inner]: undefined|object
// CHKLIR-NEXT:  %6 = CreateFunctionInst (:object) %1: environment, %inner(): functionCode
// CHKLIR-NEXT:       StoreFrameInst %1: environment, %6: object, [inner]: undefined|object
// CHKLIR-NEXT:       StoreFrameInst %1: environment, %4: undefined, [x]: empty|undefined|number
// CHKLIR-NEXT:       ReturnInst %4: undefined
// CHKLIR-NEXT:function_end

// CHKLIR:function inner(): undefined|number
// CHKLIR-NEXT:frame = []
// CHKLIR-NEXT:%BB0:
// CHKLIR-NEXT:  %0 = GetParentScopeInst (:environment) %f2(): any, %parentScope: environment
// CHKLIR-NEXT:  %1 = CreateScopeInst (:environment) %inner(): any, %0: environment
// CHKLIR-NEXT:  %2 = LIRResolveScopeInst (:environment) %f2(): any, %1: environment, 1: number
// CHKLIR-NEXT:  %3 = LoadFrameInst (:empty|undefined|number) %2: environment, [x@f2]: empty|undefined|number
// CHKLIR-NEXT:  %4 = ThrowIfInst (:undefined|number) %3: empty|undefined|number, type(empty)
// CHKLIR-NEXT:  %5 = HBCLoadConstInst (:number) 10: number
// CHKLIR-NEXT:       StoreFrameInst %2: environment, %5: number, [x@f2]: empty|undefined|number
// CHKLIR-NEXT:  %7 = LoadFrameInst (:empty|undefined|number) %2: environment, [x@f2]: empty|undefined|number
// CHKLIR-NEXT:  %8 = UnionNarrowTrustedInst (:undefined|number) %7: empty|undefined|number
// CHKLIR-NEXT:       ReturnInst %8: undefined|number
// CHKLIR-NEXT:function_end
