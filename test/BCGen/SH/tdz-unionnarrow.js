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
// CHKIR-NEXT:  %0 = DeclareGlobalVarInst "f1": string
// CHKIR-NEXT:  %1 = DeclareGlobalVarInst "f2": string
// CHKIR-NEXT:  %2 = CreateFunctionInst (:object) %f1(): any
// CHKIR-NEXT:  %3 = StorePropertyLooseInst %2: object, globalObject: object, "f1": string
// CHKIR-NEXT:  %4 = CreateFunctionInst (:object) %f2(): any
// CHKIR-NEXT:  %5 = StorePropertyLooseInst %4: object, globalObject: object, "f2": string
// CHKIR-NEXT:  %6 = AllocStackInst (:any) $?anon_0_ret: any
// CHKIR-NEXT:  %7 = StoreStackInst undefined: undefined, %6: any
// CHKIR-NEXT:  %8 = LoadStackInst (:any) %6: any
// CHKIR-NEXT:  %9 = ReturnInst %8: any
// CHKIR-NEXT:function_end

// CHKIR:function f1(): any
// CHKIR-NEXT:frame = [x: any|empty]
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = StoreFrameInst empty: empty, [x]: any|empty
// CHKIR-NEXT:  %1 = ThrowIfEmptyInst (:any) empty: empty
// CHKIR-NEXT:  %2 = UnionNarrowTrustedInst (:any) empty: empty
// CHKIR-NEXT:  %3 = BinaryAddInst (:any) %2: any, 1: number
// CHKIR-NEXT:  %4 = ReturnInst %3: any
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %5 = StoreFrameInst undefined: undefined, [x]: any|empty
// CHKIR-NEXT:  %6 = ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function f2(): any
// CHKIR-NEXT:frame = [x: any|empty, inner: any]
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = StoreFrameInst empty: empty, [x]: any|empty
// CHKIR-NEXT:  %1 = StoreFrameInst undefined: undefined, [inner]: any
// CHKIR-NEXT:  %2 = CreateFunctionInst (:object) %inner(): any
// CHKIR-NEXT:  %3 = StoreFrameInst %2: object, [inner]: any
// CHKIR-NEXT:  %4 = StoreFrameInst undefined: undefined, [x]: any|empty
// CHKIR-NEXT:  %5 = ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function inner(): any
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = LoadFrameInst (:any|empty) [x@f2]: any|empty
// CHKIR-NEXT:  %1 = ThrowIfEmptyInst (:any) %0: any|empty
// CHKIR-NEXT:  %2 = StoreFrameInst 10: number, [x@f2]: any|empty
// CHKIR-NEXT:  %3 = LoadFrameInst (:any|empty) [x@f2]: any|empty
// CHKIR-NEXT:  %4 = UnionNarrowTrustedInst (:any) %3: any|empty
// CHKIR-NEXT:  %5 = ReturnInst %4: any
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %6 = ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKLIR:function global(): undefined
// CHKLIR-NEXT:frame = []
// CHKLIR-NEXT:%BB0:
// CHKLIR-NEXT:  %0 = HBCCreateEnvironmentInst (:environment)
// CHKLIR-NEXT:  %1 = DeclareGlobalVarInst "f1": string
// CHKLIR-NEXT:  %2 = DeclareGlobalVarInst "f2": string
// CHKLIR-NEXT:  %3 = HBCCreateFunctionInst (:object) %f1(): undefined|string|number, %0: environment
// CHKLIR-NEXT:  %4 = HBCGetGlobalObjectInst (:object)
// CHKLIR-NEXT:  %5 = StorePropertyLooseInst %3: object, %4: object, "f1": string
// CHKLIR-NEXT:  %6 = HBCCreateFunctionInst (:object) %f2(): undefined, %0: environment
// CHKLIR-NEXT:  %7 = StorePropertyLooseInst %6: object, %4: object, "f2": string
// CHKLIR-NEXT:  %8 = AllocStackInst (:undefined) $?anon_0_ret: any
// CHKLIR-NEXT:  %9 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKLIR-NEXT:  %10 = StoreStackInst %9: undefined, %8: undefined
// CHKLIR-NEXT:  %11 = LoadStackInst (:undefined) %8: undefined
// CHKLIR-NEXT:  %12 = ReturnInst %11: undefined
// CHKLIR-NEXT:function_end

// CHKLIR:function f1(): undefined|string|number
// CHKLIR-NEXT:frame = [x: empty|undefined]
// CHKLIR-NEXT:%BB0:
// CHKLIR-NEXT:  %0 = HBCCreateEnvironmentInst (:environment)
// CHKLIR-NEXT:  %1 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKLIR-NEXT:  %2 = HBCLoadConstInst (:empty) empty: empty
// CHKLIR-NEXT:  %3 = HBCStoreToEnvironmentInst %0: environment, %2: empty, [x]: empty|undefined
// CHKLIR-NEXT:  %4 = ThrowIfEmptyInst (:any) %2: empty
// CHKLIR-NEXT:  %5 = LIRDeadValueInst (:any)
// CHKLIR-NEXT:  %6 = HBCLoadConstInst (:number) 1: number
// CHKLIR-NEXT:  %7 = BinaryAddInst (:string|number) %5: any, %6: number
// CHKLIR-NEXT:  %8 = ReturnInst %7: string|number
// CHKLIR-NEXT:%BB1:
// CHKLIR-NEXT:  %9 = HBCStoreToEnvironmentInst %0: environment, %1: undefined, [x]: empty|undefined
// CHKLIR-NEXT:  %10 = ReturnInst %1: undefined
// CHKLIR-NEXT:function_end

// CHKLIR:function f2(): undefined
// CHKLIR-NEXT:frame = [x: empty|undefined|number, inner: undefined|object]
// CHKLIR-NEXT:%BB0:
// CHKLIR-NEXT:  %0 = HBCCreateEnvironmentInst (:environment)
// CHKLIR-NEXT:  %1 = HBCLoadConstInst (:empty) empty: empty
// CHKLIR-NEXT:  %2 = HBCStoreToEnvironmentInst %0: environment, %1: empty, [x]: empty|undefined|number
// CHKLIR-NEXT:  %3 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKLIR-NEXT:  %4 = HBCStoreToEnvironmentInst %0: environment, %3: undefined, [inner]: undefined|object
// CHKLIR-NEXT:  %5 = HBCCreateFunctionInst (:object) %inner(): undefined|number, %0: environment
// CHKLIR-NEXT:  %6 = HBCStoreToEnvironmentInst %0: environment, %5: object, [inner]: undefined|object
// CHKLIR-NEXT:  %7 = HBCStoreToEnvironmentInst %0: environment, %3: undefined, [x]: empty|undefined|number
// CHKLIR-NEXT:  %8 = ReturnInst %3: undefined
// CHKLIR-NEXT:function_end

// CHKLIR:function inner(): undefined|number
// CHKLIR-NEXT:frame = []
// CHKLIR-NEXT:%BB0:
// CHKLIR-NEXT:  %0 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKLIR-NEXT:  %1 = HBCResolveEnvironment (:environment) %f2(): any
// CHKLIR-NEXT:  %2 = HBCLoadFromEnvironmentInst (:empty|undefined|number) %1: environment, [x@f2]: empty|undefined|number
// CHKLIR-NEXT:  %3 = ThrowIfEmptyInst (:undefined|number) %2: empty|undefined|number
// CHKLIR-NEXT:  %4 = HBCLoadConstInst (:number) 10: number
// CHKLIR-NEXT:  %5 = HBCStoreToEnvironmentInst %1: environment, %4: number, [x@f2]: empty|undefined|number
// CHKLIR-NEXT:  %6 = HBCLoadFromEnvironmentInst (:undefined|number) %1: environment, [x@f2]: empty|undefined|number
// CHKLIR-NEXT:  %7 = ReturnInst %6: undefined|number
// CHKLIR-NEXT:%BB1:
// CHKLIR-NEXT:  %8 = ReturnInst %0: undefined
// CHKLIR-NEXT:function_end
