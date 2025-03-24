/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

function func1() {
    return new.target;
}

function func2(a) {
    if (a)
        return new.target;
    print(new.target !== undefined);
}

function func3() {
    print(new.target !== undefined);
    function innerFunction() {
        return new.target;
    }
    var innerArrow1 = () => {
        print(new.target !== undefined);
        var innerArrow2 = () => { return new.target;}
        return innerArrow2();
    }

    return [innerFunction, innerArrow1];
}

function func4() {
    return new.target.prototype;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "func1": string
// CHECK-NEXT:       DeclareGlobalVarInst "func2": string
// CHECK-NEXT:       DeclareGlobalVarInst "func3": string
// CHECK-NEXT:       DeclareGlobalVarInst "func4": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %0: environment, %VS0: any, %func1(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %5: object, globalObject: object, "func1": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %0: environment, %VS0: any, %func2(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %7: object, globalObject: object, "func2": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %0: environment, %VS0: any, %func3(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %9: object, globalObject: object, "func3": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %0: environment, %VS0: any, %func4(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %11: object, globalObject: object, "func4": string
// CHECK-NEXT:  %13 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %13: any
// CHECK-NEXT:  %15 = LoadStackInst (:any) %13: any
// CHECK-NEXT:        ReturnInst %15: any
// CHECK-NEXT:function_end

// CHECK:scope %VS1 []

// CHECK:function func1(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:       ReturnInst %2: undefined|object
// CHECK-NEXT:function_end

// CHECK:scope %VS2 [a: any]

// CHECK:function func2(a: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS2: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %a: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS2.a]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [%VS2.a]: any
// CHECK-NEXT:       CondBranchInst %4: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:       ReturnInst %6: undefined|object
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %9 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %10 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:  %11 = BinaryStrictlyNotEqualInst (:boolean) %10: undefined|object, undefined: undefined
// CHECK-NEXT:  %12 = CallInst (:any) %9: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %11: boolean
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS3 [?anon_0_this: any, ?anon_1_new.target: undefined|object, innerFunction: any, innerArrow1: any]

// CHECK:function func3(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = CoerceThisNSInst (:object) %0: any
// CHECK-NEXT:  %2 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %3 = CreateScopeInst (:environment) %VS3: any, %2: environment
// CHECK-NEXT:       StoreFrameInst %3: environment, %1: object, [%VS3.?anon_0_this]: any
// CHECK-NEXT:  %5 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:       StoreFrameInst %3: environment, %5: undefined|object, [%VS3.?anon_1_new.target]: undefined|object
// CHECK-NEXT:       StoreFrameInst %3: environment, undefined: undefined, [%VS3.innerArrow1]: any
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %3: environment, %VS3: any, %innerFunction(): functionCode
// CHECK-NEXT:       StoreFrameInst %3: environment, %8: object, [%VS3.innerFunction]: any
// CHECK-NEXT:  %10 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %11 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:  %12 = BinaryStrictlyNotEqualInst (:boolean) %11: undefined|object, undefined: undefined
// CHECK-NEXT:  %13 = CallInst (:any) %10: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %12: boolean
// CHECK-NEXT:  %14 = CreateFunctionInst (:object) %3: environment, %VS3: any, %innerArrow1(): functionCode
// CHECK-NEXT:        StoreFrameInst %3: environment, %14: object, [%VS3.innerArrow1]: any
// CHECK-NEXT:  %16 = LoadFrameInst (:any) %3: environment, [%VS3.innerFunction]: any
// CHECK-NEXT:  %17 = AllocArrayInst (:object) 2: number
// CHECK-NEXT:        DefineOwnPropertyInst %16: any, %17: object, 0: number, true: boolean
// CHECK-NEXT:  %19 = LoadFrameInst (:any) %3: environment, [%VS3.innerArrow1]: any
// CHECK-NEXT:        DefineOwnPropertyInst %19: any, %17: object, 1: number, true: boolean
// CHECK-NEXT:        ReturnInst %17: object
// CHECK-NEXT:function_end

// CHECK:scope %VS4 []

// CHECK:function func4(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS4: any, %0: environment
// CHECK-NEXT:  %2 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) %2: undefined|object, "prototype": string
// CHECK-NEXT:       ReturnInst %3: any
// CHECK-NEXT:function_end

// CHECK:scope %VS5 []

// CHECK:function innerFunction(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS3: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS5: any, %0: environment
// CHECK-NEXT:  %2 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:       ReturnInst %2: undefined|object
// CHECK-NEXT:function_end

// CHECK:scope %VS6 [innerArrow2: any]

// CHECK:arrow innerArrow1(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS3: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS6: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS6.innerArrow2]: any
// CHECK-NEXT:  %3 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %4 = LoadFrameInst (:undefined|object) %0: environment, [%VS3.?anon_1_new.target]: undefined|object
// CHECK-NEXT:  %5 = BinaryStrictlyNotEqualInst (:boolean) %4: undefined|object, undefined: undefined
// CHECK-NEXT:  %6 = CallInst (:any) %3: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %5: boolean
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %1: environment, %VS6: any, %innerArrow2(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %7: object, [%VS6.innerArrow2]: any
// CHECK-NEXT:  %9 = LoadFrameInst (:any) %1: environment, [%VS6.innerArrow2]: any
// CHECK-NEXT:  %10 = CallInst (:any) %9: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:        ReturnInst %10: any
// CHECK-NEXT:function_end

// CHECK:scope %VS7 []

// CHECK:arrow innerArrow2(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS6: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS7: any, %0: environment
// CHECK-NEXT:  %2 = ResolveScopeInst (:environment) %VS3: any, %VS6: any, %0: environment
// CHECK-NEXT:  %3 = LoadFrameInst (:undefined|object) %2: environment, [%VS3.?anon_1_new.target]: undefined|object
// CHECK-NEXT:       ReturnInst %3: undefined|object
// CHECK-NEXT:function_end
