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

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "func1": string
// CHECK-NEXT:       DeclareGlobalVarInst "func2": string
// CHECK-NEXT:       DeclareGlobalVarInst "func3": string
// CHECK-NEXT:       DeclareGlobalVarInst "func4": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %0: environment, %func1(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %5: object, globalObject: object, "func1": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %0: environment, %func2(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %7: object, globalObject: object, "func2": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %0: environment, %func3(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %9: object, globalObject: object, "func3": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %0: environment, %func4(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %11: object, globalObject: object, "func4": string
// CHECK-NEXT:  %13 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %13: any
// CHECK-NEXT:  %15 = LoadStackInst (:any) %13: any
// CHECK-NEXT:        ReturnInst %15: any
// CHECK-NEXT:function_end

// CHECK:function func1(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %func1(): any, %0: environment
// CHECK-NEXT:  %2 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:       ReturnInst %2: undefined|object
// CHECK-NEXT:function_end

// CHECK:function func2(a: any): any
// CHECK-NEXT:frame = [a: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %func2(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %a: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [a]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [a]: any
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
// CHECK-NEXT:  %12 = CallInst (:any) %9: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %11: boolean
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function func3(): any
// CHECK-NEXT:frame = [?anon_0_this: any, ?anon_1_new.target: undefined|object, innerFunction: any, innerArrow1: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = CoerceThisNSInst (:object) %0: any
// CHECK-NEXT:  %2 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %3 = CreateScopeInst (:environment) %func3(): any, %2: environment
// CHECK-NEXT:       StoreFrameInst %3: environment, %1: object, [?anon_0_this]: any
// CHECK-NEXT:  %5 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:       StoreFrameInst %3: environment, %5: undefined|object, [?anon_1_new.target]: undefined|object
// CHECK-NEXT:       StoreFrameInst %3: environment, undefined: undefined, [innerFunction]: any
// CHECK-NEXT:       StoreFrameInst %3: environment, undefined: undefined, [innerArrow1]: any
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %3: environment, %innerFunction(): functionCode
// CHECK-NEXT:        StoreFrameInst %3: environment, %9: object, [innerFunction]: any
// CHECK-NEXT:  %11 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %12 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:  %13 = BinaryStrictlyNotEqualInst (:boolean) %12: undefined|object, undefined: undefined
// CHECK-NEXT:  %14 = CallInst (:any) %11: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %13: boolean
// CHECK-NEXT:  %15 = CreateFunctionInst (:object) %3: environment, %innerArrow1(): functionCode
// CHECK-NEXT:        StoreFrameInst %3: environment, %15: object, [innerArrow1]: any
// CHECK-NEXT:  %17 = LoadFrameInst (:any) %3: environment, [innerFunction]: any
// CHECK-NEXT:  %18 = AllocArrayInst (:object) 2: number
// CHECK-NEXT:        StoreOwnPropertyInst %17: any, %18: object, 0: number, true: boolean
// CHECK-NEXT:  %20 = LoadFrameInst (:any) %3: environment, [innerArrow1]: any
// CHECK-NEXT:        StoreOwnPropertyInst %20: any, %18: object, 1: number, true: boolean
// CHECK-NEXT:        ReturnInst %18: object
// CHECK-NEXT:function_end

// CHECK:function func4(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %func4(): any, %0: environment
// CHECK-NEXT:  %2 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) %2: undefined|object, "prototype": string
// CHECK-NEXT:       ReturnInst %3: any
// CHECK-NEXT:function_end

// CHECK:function innerFunction(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %func3(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %innerFunction(): any, %0: environment
// CHECK-NEXT:  %2 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:       ReturnInst %2: undefined|object
// CHECK-NEXT:function_end

// CHECK:arrow innerArrow1(): any
// CHECK-NEXT:frame = [innerArrow2: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %func3(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %innerArrow1(): any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [innerArrow2]: any
// CHECK-NEXT:  %3 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %4 = ResolveScopeInst (:environment) %func3(): any, %1: environment
// CHECK-NEXT:  %5 = LoadFrameInst (:undefined|object) %4: environment, [?anon_1_new.target@func3]: undefined|object
// CHECK-NEXT:  %6 = BinaryStrictlyNotEqualInst (:boolean) %5: undefined|object, undefined: undefined
// CHECK-NEXT:  %7 = CallInst (:any) %3: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %6: boolean
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %1: environment, %innerArrow2(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %8: object, [innerArrow2]: any
// CHECK-NEXT:  %10 = LoadFrameInst (:any) %1: environment, [innerArrow2]: any
// CHECK-NEXT:  %11 = CallInst (:any) %10: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:        ReturnInst %11: any
// CHECK-NEXT:function_end

// CHECK:arrow innerArrow2(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %innerArrow1(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %innerArrow2(): any, %0: environment
// CHECK-NEXT:  %2 = ResolveScopeInst (:environment) %func3(): any, %1: environment
// CHECK-NEXT:  %3 = LoadFrameInst (:undefined|object) %2: environment, [?anon_1_new.target@func3]: undefined|object
// CHECK-NEXT:       ReturnInst %3: undefined|object
// CHECK-NEXT:function_end
