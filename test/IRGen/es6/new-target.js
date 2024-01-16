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
// CHECK-NEXT:       DeclareGlobalVarInst "func1": string
// CHECK-NEXT:       DeclareGlobalVarInst "func2": string
// CHECK-NEXT:       DeclareGlobalVarInst "func3": string
// CHECK-NEXT:       DeclareGlobalVarInst "func4": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %func1(): any
// CHECK-NEXT:       StorePropertyLooseInst %4: object, globalObject: object, "func1": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %func2(): any
// CHECK-NEXT:       StorePropertyLooseInst %6: object, globalObject: object, "func2": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %func3(): any
// CHECK-NEXT:       StorePropertyLooseInst %8: object, globalObject: object, "func3": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %func4(): any
// CHECK-NEXT:        StorePropertyLooseInst %10: object, globalObject: object, "func4": string
// CHECK-NEXT:  %12 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %12: any
// CHECK-NEXT:  %14 = LoadStackInst (:any) %12: any
// CHECK-NEXT:        ReturnInst %14: any
// CHECK-NEXT:function_end

// CHECK:function func1(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:       ReturnInst %0: undefined|object
// CHECK-NEXT:function_end

// CHECK:function func2(a: any): any
// CHECK-NEXT:frame = [a: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:       StoreFrameInst %0: any, [a]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [a]: any
// CHECK-NEXT:       CondBranchInst %2: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:       ReturnInst %4: undefined|object
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %8 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:  %9 = BinaryStrictlyNotEqualInst (:boolean) %8: undefined|object, undefined: undefined
// CHECK-NEXT:  %10 = CallInst (:any) %7: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %9: boolean
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function func3(): any
// CHECK-NEXT:frame = [?anon_0_this: any, ?anon_1_new.target: undefined|object, innerFunction: any, innerArrow1: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = CoerceThisNSInst (:object) %0: any
// CHECK-NEXT:       StoreFrameInst %1: object, [?anon_0_this]: any
// CHECK-NEXT:  %3 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:       StoreFrameInst %3: undefined|object, [?anon_1_new.target]: undefined|object
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [innerFunction]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [innerArrow1]: any
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %innerFunction(): any
// CHECK-NEXT:       StoreFrameInst %7: object, [innerFunction]: any
// CHECK-NEXT:  %9 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %10 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:  %11 = BinaryStrictlyNotEqualInst (:boolean) %10: undefined|object, undefined: undefined
// CHECK-NEXT:  %12 = CallInst (:any) %9: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %11: boolean
// CHECK-NEXT:  %13 = CreateFunctionInst (:object) %innerArrow1(): any
// CHECK-NEXT:        StoreFrameInst %13: object, [innerArrow1]: any
// CHECK-NEXT:  %15 = LoadFrameInst (:any) [innerFunction]: any
// CHECK-NEXT:  %16 = AllocArrayInst (:object) 2: number
// CHECK-NEXT:        StoreOwnPropertyInst %15: any, %16: object, 0: number, true: boolean
// CHECK-NEXT:  %18 = LoadFrameInst (:any) [innerArrow1]: any
// CHECK-NEXT:        StoreOwnPropertyInst %18: any, %16: object, 1: number, true: boolean
// CHECK-NEXT:        ReturnInst %16: object
// CHECK-NEXT:function_end

// CHECK:function func4(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) %0: undefined|object, "prototype": string
// CHECK-NEXT:       ReturnInst %1: any
// CHECK-NEXT:function_end

// CHECK:function innerFunction(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:       ReturnInst %0: undefined|object
// CHECK-NEXT:function_end

// CHECK:arrow innerArrow1(): any
// CHECK-NEXT:frame = [innerArrow2: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [innerArrow2]: any
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %2 = LoadFrameInst (:undefined|object) [?anon_1_new.target@func3]: undefined|object
// CHECK-NEXT:  %3 = BinaryStrictlyNotEqualInst (:boolean) %2: undefined|object, undefined: undefined
// CHECK-NEXT:  %4 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %3: boolean
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %innerArrow2(): any
// CHECK-NEXT:       StoreFrameInst %5: object, [innerArrow2]: any
// CHECK-NEXT:  %7 = LoadFrameInst (:any) [innerArrow2]: any
// CHECK-NEXT:  %8 = CallInst (:any) %7: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       ReturnInst %8: any
// CHECK-NEXT:function_end

// CHECK:arrow innerArrow2(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst (:undefined|object) [?anon_1_new.target@func3]: undefined|object
// CHECK-NEXT:       ReturnInst %0: undefined|object
// CHECK-NEXT:function_end
