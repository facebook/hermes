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

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "outer1": string
// CHECK-NEXT:       DeclareGlobalVarInst "outer2": string
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %0: environment, %outer1(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %3: object, globalObject: object, "outer1": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %0: environment, %outer2(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %5: object, globalObject: object, "outer2": string
// CHECK-NEXT:  %7 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %7: any
// CHECK-NEXT:  %9 = LoadStackInst (:any) %7: any
// CHECK-NEXT:        ReturnInst %9: any
// CHECK-NEXT:function_end

// CHECK:function outer1(): any
// CHECK-NEXT:frame = [?anon_0_this: any, ?anon_1_new.target: undefined|object, innerArrow1: any, innerArrow2: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = CoerceThisNSInst (:object) %0: any
// CHECK-NEXT:  %2 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %3 = CreateScopeInst (:environment) %outer1(): any, %2: environment
// CHECK-NEXT:       StoreFrameInst %3: environment, %1: object, [?anon_0_this]: any
// CHECK-NEXT:  %5 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:       StoreFrameInst %3: environment, %5: undefined|object, [?anon_1_new.target]: undefined|object
// CHECK-NEXT:       StoreFrameInst %3: environment, undefined: undefined, [innerArrow1]: any
// CHECK-NEXT:       StoreFrameInst %3: environment, undefined: undefined, [innerArrow2]: any
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %3: environment, %innerArrow1(): functionCode
// CHECK-NEXT:        StoreFrameInst %3: environment, %9: object, [innerArrow1]: any
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %3: environment, %innerArrow2(): functionCode
// CHECK-NEXT:        StoreFrameInst %3: environment, %11: object, [innerArrow2]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function outer2(): any
// CHECK-NEXT:frame = [?anon_0_this: any, ?anon_1_new.target: undefined|object, inner3: any, innerArrow4: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = CoerceThisNSInst (:object) %0: any
// CHECK-NEXT:  %2 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %3 = CreateScopeInst (:environment) %outer2(): any, %2: environment
// CHECK-NEXT:       StoreFrameInst %3: environment, %1: object, [?anon_0_this]: any
// CHECK-NEXT:  %5 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:       StoreFrameInst %3: environment, %5: undefined|object, [?anon_1_new.target]: undefined|object
// CHECK-NEXT:       StoreFrameInst %3: environment, undefined: undefined, [inner3]: any
// CHECK-NEXT:       StoreFrameInst %3: environment, undefined: undefined, [innerArrow4]: any
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %3: environment, %inner3(): functionCode
// CHECK-NEXT:        StoreFrameInst %3: environment, %9: object, [inner3]: any
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %3: environment, %innerArrow4(): functionCode
// CHECK-NEXT:        StoreFrameInst %3: environment, %11: object, [innerArrow4]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:arrow innerArrow1(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %outer1(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %innerArrow1(): any, %0: environment
// CHECK-NEXT:  %2 = ResolveScopeInst (:environment) %outer1(): any, %1: environment
// CHECK-NEXT:  %3 = LoadFrameInst (:any) %2: environment, [?anon_0_this@outer1]: any
// CHECK-NEXT:  %4 = LoadPropertyInst (:any) %3: any, "x": string
// CHECK-NEXT:       ReturnInst %4: any
// CHECK-NEXT:function_end

// CHECK:arrow innerArrow2(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %outer1(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %innerArrow2(): any, %0: environment
// CHECK-NEXT:  %2 = ResolveScopeInst (:environment) %outer1(): any, %1: environment
// CHECK-NEXT:  %3 = LoadFrameInst (:any) %2: environment, [?anon_0_this@outer1]: any
// CHECK-NEXT:  %4 = LoadPropertyInst (:any) %3: any, "y": string
// CHECK-NEXT:       ReturnInst %4: any
// CHECK-NEXT:function_end

// CHECK:function inner3(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = CoerceThisNSInst (:object) %0: any
// CHECK-NEXT:  %2 = GetParentScopeInst (:environment) %outer2(): any, %parentScope: environment
// CHECK-NEXT:  %3 = CreateScopeInst (:environment) %inner3(): any, %2: environment
// CHECK-NEXT:  %4 = LoadPropertyInst (:any) %1: object, "a": string
// CHECK-NEXT:       ReturnInst %4: any
// CHECK-NEXT:function_end

// CHECK:arrow innerArrow4(): any
// CHECK-NEXT:frame = [nestedInnerArrow5: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %outer2(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %innerArrow4(): any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [nestedInnerArrow5]: any
// CHECK-NEXT:  %3 = ResolveScopeInst (:environment) %outer2(): any, %1: environment
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %3: environment, [?anon_0_this@outer2]: any
// CHECK-NEXT:       StorePropertyLooseInst 10: number, %4: any, "b": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %1: environment, %nestedInnerArrow5(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: object, [nestedInnerArrow5]: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:arrow nestedInnerArrow5(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %innerArrow4(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %nestedInnerArrow5(): any, %0: environment
// CHECK-NEXT:  %2 = ResolveScopeInst (:environment) %outer2(): any, %1: environment
// CHECK-NEXT:  %3 = LoadFrameInst (:any) %2: environment, [?anon_0_this@outer2]: any
// CHECK-NEXT:  %4 = LoadPropertyInst (:any) %3: any, "b": string
// CHECK-NEXT:       ReturnInst %4: any
// CHECK-NEXT:function_end
