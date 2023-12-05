/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -custom-opt=tdzdedup -dump-ir --test262 %s | %FileCheckOrRegen --check-prefix=CHKIR --match-full-lines %s
// RUN: %shermes -custom-opt=tdzdedup -dump-lir --test262 %s | %FileCheckOrRegen --check-prefix=CHKLIR --match-full-lines %s

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

// CHKIR:function global(): any
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "f1": string
// CHKIR-NEXT:       DeclareGlobalVarInst "f2": string
// CHKIR-NEXT:  %2 = CreateFunctionInst (:object) %f1(): any
// CHKIR-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "f1": string
// CHKIR-NEXT:  %4 = CreateFunctionInst (:object) %f2(): any
// CHKIR-NEXT:       StorePropertyLooseInst %4: object, globalObject: object, "f2": string
// CHKIR-NEXT:  %6 = AllocStackInst (:any) $?anon_0_ret: any
// CHKIR-NEXT:       StoreStackInst undefined: undefined, %6: any
// CHKIR-NEXT:  %8 = LoadStackInst (:any) %6: any
// CHKIR-NEXT:       ReturnInst %8: any
// CHKIR-NEXT:function_end

// CHKIR:function f1(): any
// CHKIR-NEXT:frame = [x: any|empty]
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       StoreFrameInst empty: empty, [x]: any|empty
// CHKIR-NEXT:  %1 = ThrowIfEmptyInst (:any) empty: empty
// CHKIR-NEXT:  %2 = UnionNarrowTrustedInst (:any) empty: empty
// CHKIR-NEXT:  %3 = BinaryAddInst (:any) %2: any, 1: number
// CHKIR-NEXT:       ReturnInst %3: any
// CHKIR-NEXT:function_end

// CHKIR:function f2(): any
// CHKIR-NEXT:frame = [x: any|empty, inner: any]
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       StoreFrameInst empty: empty, [x]: any|empty
// CHKIR-NEXT:       StoreFrameInst undefined: undefined, [inner]: any
// CHKIR-NEXT:  %2 = CreateFunctionInst (:object) %inner(): any
// CHKIR-NEXT:       StoreFrameInst %2: object, [inner]: any
// CHKIR-NEXT:       StoreFrameInst undefined: undefined, [x]: any|empty
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function inner(): any
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = LoadFrameInst (:any|empty) [x@f2]: any|empty
// CHKIR-NEXT:  %1 = ThrowIfEmptyInst (:any) %0: any|empty
// CHKIR-NEXT:       StoreFrameInst 10: number, [x@f2]: any|empty
// CHKIR-NEXT:  %3 = LoadFrameInst (:any|empty) [x@f2]: any|empty
// CHKIR-NEXT:  %4 = UnionNarrowTrustedInst (:any) %3: any|empty
// CHKIR-NEXT:       ReturnInst %4: any
// CHKIR-NEXT:function_end

// CHKLIR:function global(): undefined
// CHKLIR-NEXT:frame = []
// CHKLIR-NEXT:%BB0:
// CHKLIR-NEXT:  %0 = HBCCreateEnvironmentInst (:environment)
// CHKLIR-NEXT:       DeclareGlobalVarInst "f1": string
// CHKLIR-NEXT:       DeclareGlobalVarInst "f2": string
// CHKLIR-NEXT:  %3 = HBCCreateFunctionInst (:object) %f1(): string|number, %0: environment
// CHKLIR-NEXT:  %4 = HBCGetGlobalObjectInst (:object)
// CHKLIR-NEXT:       StorePropertyLooseInst %3: object, %4: object, "f1": string
// CHKLIR-NEXT:  %6 = HBCCreateFunctionInst (:object) %f2(): undefined, %0: environment
// CHKLIR-NEXT:       StorePropertyLooseInst %6: object, %4: object, "f2": string
// CHKLIR-NEXT:  %8 = AllocStackInst (:undefined) $?anon_0_ret: any
// CHKLIR-NEXT:  %9 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKLIR-NEXT:        StoreStackInst %9: undefined, %8: undefined
// CHKLIR-NEXT:  %11 = LoadStackInst (:undefined) %8: undefined
// CHKLIR-NEXT:        ReturnInst %11: undefined
// CHKLIR-NEXT:function_end

// CHKLIR:function f1(): string|number
// CHKLIR-NEXT:frame = [x: empty]
// CHKLIR-NEXT:%BB0:
// CHKLIR-NEXT:  %0 = HBCCreateEnvironmentInst (:environment)
// CHKLIR-NEXT:  %1 = HBCLoadConstInst (:empty) empty: empty
// CHKLIR-NEXT:       HBCStoreToEnvironmentInst %0: environment, %1: empty, [x]: empty
// CHKLIR-NEXT:  %3 = ThrowIfEmptyInst (:any) %1: empty
// CHKLIR-NEXT:  %4 = UnionNarrowTrustedInst (:any) %1: empty
// CHKLIR-NEXT:  %5 = HBCLoadConstInst (:number) 1: number
// CHKLIR-NEXT:  %6 = BinaryAddInst (:string|number) %4: any, %5: number
// CHKLIR-NEXT:       ReturnInst %6: string|number
// CHKLIR-NEXT:function_end

// CHKLIR:function f2(): undefined
// CHKLIR-NEXT:frame = [x: empty|undefined|number, inner: undefined|object]
// CHKLIR-NEXT:%BB0:
// CHKLIR-NEXT:  %0 = HBCCreateEnvironmentInst (:environment)
// CHKLIR-NEXT:  %1 = HBCLoadConstInst (:empty) empty: empty
// CHKLIR-NEXT:       HBCStoreToEnvironmentInst %0: environment, %1: empty, [x]: empty|undefined|number
// CHKLIR-NEXT:  %3 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKLIR-NEXT:       HBCStoreToEnvironmentInst %0: environment, %3: undefined, [inner]: undefined|object
// CHKLIR-NEXT:  %5 = HBCCreateFunctionInst (:object) %inner(): undefined|number, %0: environment
// CHKLIR-NEXT:       HBCStoreToEnvironmentInst %0: environment, %5: object, [inner]: undefined|object
// CHKLIR-NEXT:       HBCStoreToEnvironmentInst %0: environment, %3: undefined, [x]: empty|undefined|number
// CHKLIR-NEXT:       ReturnInst %3: undefined
// CHKLIR-NEXT:function_end

// CHKLIR:function inner(): undefined|number
// CHKLIR-NEXT:frame = []
// CHKLIR-NEXT:%BB0:
// CHKLIR-NEXT:  %0 = HBCResolveEnvironment (:environment) %f2(): any
// CHKLIR-NEXT:  %1 = HBCLoadFromEnvironmentInst (:empty|undefined|number) %0: environment, [x@f2]: empty|undefined|number
// CHKLIR-NEXT:  %2 = ThrowIfEmptyInst (:undefined|number) %1: empty|undefined|number
// CHKLIR-NEXT:  %3 = HBCLoadConstInst (:number) 10: number
// CHKLIR-NEXT:       HBCStoreToEnvironmentInst %0: environment, %3: number, [x@f2]: empty|undefined|number
// CHKLIR-NEXT:  %5 = HBCLoadFromEnvironmentInst (:empty|undefined|number) %0: environment, [x@f2]: empty|undefined|number
// CHKLIR-NEXT:  %6 = UnionNarrowTrustedInst (:undefined|number) %5: empty|undefined|number
// CHKLIR-NEXT:       ReturnInst %6: undefined|number
// CHKLIR-NEXT:function_end
