/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

function outer1() {
    var innerArrow1 = () => this.x;
    var innerArrow2 = () => this.y;
}

function outer2() {
    function inner3() {
        return this.a;
    }
    var innerArrow4 = () => {
        this.b = 10;
        var nestedInnerArrow5 = () => {
            return this.b;
        }
    }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "outer1": string
// CHECK-NEXT:       DeclareGlobalVarInst "outer2": string
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %0: environment, %VS0: any, %outer1(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %3: object, globalObject: object, "outer1": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %0: environment, %VS0: any, %outer2(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %5: object, globalObject: object, "outer2": string
// CHECK-NEXT:  %7 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %7: any
// CHECK-NEXT:  %9 = LoadStackInst (:any) %7: any
// CHECK-NEXT:        ReturnInst %9: any
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [?anon_0_this: any, ?anon_1_new.target: undefined|object, innerArrow1: any, innerArrow2: any]

// CHECK:function outer1(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = CoerceThisNSInst (:object) %0: any
// CHECK-NEXT:  %2 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %3 = CreateScopeInst (:environment) %VS1: any, %2: environment
// CHECK-NEXT:       StoreFrameInst %3: environment, %1: object, [%VS1.?anon_0_this]: any
// CHECK-NEXT:  %5 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:       StoreFrameInst %3: environment, %5: undefined|object, [%VS1.?anon_1_new.target]: undefined|object
// CHECK-NEXT:       StoreFrameInst %3: environment, undefined: undefined, [%VS1.innerArrow1]: any
// CHECK-NEXT:       StoreFrameInst %3: environment, undefined: undefined, [%VS1.innerArrow2]: any
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %3: environment, %VS1: any, %innerArrow1(): functionCode
// CHECK-NEXT:        StoreFrameInst %3: environment, %9: object, [%VS1.innerArrow1]: any
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %3: environment, %VS1: any, %innerArrow2(): functionCode
// CHECK-NEXT:        StoreFrameInst %3: environment, %11: object, [%VS1.innerArrow2]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS2 [?anon_0_this: any, ?anon_1_new.target: undefined|object, inner3: any, innerArrow4: any]

// CHECK:function outer2(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = CoerceThisNSInst (:object) %0: any
// CHECK-NEXT:  %2 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %3 = CreateScopeInst (:environment) %VS2: any, %2: environment
// CHECK-NEXT:       StoreFrameInst %3: environment, %1: object, [%VS2.?anon_0_this]: any
// CHECK-NEXT:  %5 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:       StoreFrameInst %3: environment, %5: undefined|object, [%VS2.?anon_1_new.target]: undefined|object
// CHECK-NEXT:       StoreFrameInst %3: environment, undefined: undefined, [%VS2.innerArrow4]: any
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %3: environment, %VS2: any, %inner3(): functionCode
// CHECK-NEXT:       StoreFrameInst %3: environment, %8: object, [%VS2.inner3]: any
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %3: environment, %VS2: any, %innerArrow4(): functionCode
// CHECK-NEXT:        StoreFrameInst %3: environment, %10: object, [%VS2.innerArrow4]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS3 []

// CHECK:arrow innerArrow1(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS3: any, %0: environment
// CHECK-NEXT:  %2 = LoadFrameInst (:any) %0: environment, [%VS1.?anon_0_this]: any
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) %2: any, "x": string
// CHECK-NEXT:       ReturnInst %3: any
// CHECK-NEXT:function_end

// CHECK:scope %VS4 []

// CHECK:arrow innerArrow2(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS4: any, %0: environment
// CHECK-NEXT:  %2 = LoadFrameInst (:any) %0: environment, [%VS1.?anon_0_this]: any
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) %2: any, "y": string
// CHECK-NEXT:       ReturnInst %3: any
// CHECK-NEXT:function_end

// CHECK:scope %VS5 []

// CHECK:function inner3(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = CoerceThisNSInst (:object) %0: any
// CHECK-NEXT:  %2 = GetParentScopeInst (:environment) %VS2: any, %parentScope: environment
// CHECK-NEXT:  %3 = CreateScopeInst (:environment) %VS5: any, %2: environment
// CHECK-NEXT:  %4 = LoadPropertyInst (:any) %1: object, "a": string
// CHECK-NEXT:       ReturnInst %4: any
// CHECK-NEXT:function_end

// CHECK:scope %VS6 [nestedInnerArrow5: any]

// CHECK:arrow innerArrow4(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS2: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS6: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS6.nestedInnerArrow5]: any
// CHECK-NEXT:  %3 = LoadFrameInst (:any) %0: environment, [%VS2.?anon_0_this]: any
// CHECK-NEXT:       StorePropertyLooseInst 10: number, %3: any, "b": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %1: environment, %VS6: any, %nestedInnerArrow5(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %5: object, [%VS6.nestedInnerArrow5]: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS7 []

// CHECK:arrow nestedInnerArrow5(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS6: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS7: any, %0: environment
// CHECK-NEXT:  %2 = ResolveScopeInst (:environment) %VS2: any, %VS6: any, %0: environment
// CHECK-NEXT:  %3 = LoadFrameInst (:any) %2: environment, [%VS2.?anon_0_this]: any
// CHECK-NEXT:  %4 = LoadPropertyInst (:any) %3: any, "b": string
// CHECK-NEXT:       ReturnInst %4: any
// CHECK-NEXT:function_end
