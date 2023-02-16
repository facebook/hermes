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
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "func1": string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "func2": string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "func3": string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "func4": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:closure) %func1(): any
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4: closure, globalObject: object, "func1": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:closure) %func2(): any
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6: closure, globalObject: object, "func2": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:closure) %func3(): any
// CHECK-NEXT:  %9 = StorePropertyLooseInst %8: closure, globalObject: object, "func3": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:closure) %func4(): any
// CHECK-NEXT:  %11 = StorePropertyLooseInst %10: closure, globalObject: object, "func4": string
// CHECK-NEXT:  %12 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %13 = StoreStackInst undefined: undefined, %12: any
// CHECK-NEXT:  %14 = LoadStackInst (:any) %12: any
// CHECK-NEXT:  %15 = ReturnInst (:any) %14: any
// CHECK-NEXT:function_end

// CHECK:function func1(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetNewTargetInst (:any) %new.target: any
// CHECK-NEXT:  %1 = ReturnInst (:any) %0: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function func2(a: any): any
// CHECK-NEXT:frame = [a: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [a]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [a]: any
// CHECK-NEXT:  %3 = CondBranchInst %2: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = GetNewTargetInst (:any) %new.target: any
// CHECK-NEXT:  %5 = ReturnInst (:any) %4: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %8 = GetNewTargetInst (:any) %new.target: any
// CHECK-NEXT:  %9 = BinaryStrictlyNotEqualInst (:any) %8: any, undefined: undefined
// CHECK-NEXT:  %10 = CallInst (:any) %7: any, empty: any, empty: any, undefined: undefined, %9: any
// CHECK-NEXT:  %11 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %12 = BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function func3(): any
// CHECK-NEXT:frame = [?anon_0_this: any, ?anon_1_new.target: any, innerFunction: any, innerArrow1: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %this: any
// CHECK-NEXT:  %1 = CoerceThisNSInst (:object) %0: any
// CHECK-NEXT:  %2 = StoreFrameInst %1: object, [?anon_0_this]: any
// CHECK-NEXT:  %3 = GetNewTargetInst (:any) %new.target: any
// CHECK-NEXT:  %4 = StoreFrameInst %3: any, [?anon_1_new.target]: any
// CHECK-NEXT:  %5 = StoreFrameInst undefined: undefined, [innerArrow1]: any
// CHECK-NEXT:  %6 = CreateFunctionInst (:closure) %innerFunction(): any
// CHECK-NEXT:  %7 = StoreFrameInst %6: closure, [innerFunction]: any
// CHECK-NEXT:  %8 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %9 = GetNewTargetInst (:any) %new.target: any
// CHECK-NEXT:  %10 = BinaryStrictlyNotEqualInst (:any) %9: any, undefined: undefined
// CHECK-NEXT:  %11 = CallInst (:any) %8: any, empty: any, empty: any, undefined: undefined, %10: any
// CHECK-NEXT:  %12 = CreateFunctionInst (:closure) %innerArrow1(): any
// CHECK-NEXT:  %13 = StoreFrameInst %12: closure, [innerArrow1]: any
// CHECK-NEXT:  %14 = LoadFrameInst (:any) [innerFunction]: any
// CHECK-NEXT:  %15 = AllocArrayInst (:object) 2: number
// CHECK-NEXT:  %16 = StoreOwnPropertyInst %14: any, %15: object, 0: number, true: boolean
// CHECK-NEXT:  %17 = LoadFrameInst (:any) [innerArrow1]: any
// CHECK-NEXT:  %18 = StoreOwnPropertyInst %17: any, %15: object, 1: number, true: boolean
// CHECK-NEXT:  %19 = ReturnInst (:any) %15: object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %20 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function func4(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetNewTargetInst (:any) %new.target: any
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) %0: any, "prototype": string
// CHECK-NEXT:  %2 = ReturnInst (:any) %1: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function innerFunction(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetNewTargetInst (:any) %new.target: any
// CHECK-NEXT:  %1 = ReturnInst (:any) %0: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:arrow innerArrow1(): any
// CHECK-NEXT:frame = [innerArrow2: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined: undefined, [innerArrow2]: any
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [?anon_1_new.target@func3]: any
// CHECK-NEXT:  %3 = BinaryStrictlyNotEqualInst (:any) %2: any, undefined: undefined
// CHECK-NEXT:  %4 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, %3: any
// CHECK-NEXT:  %5 = CreateFunctionInst (:closure) %innerArrow2(): any
// CHECK-NEXT:  %6 = StoreFrameInst %5: closure, [innerArrow2]: any
// CHECK-NEXT:  %7 = LoadFrameInst (:any) [innerArrow2]: any
// CHECK-NEXT:  %8 = CallInst (:any) %7: any, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %9 = ReturnInst (:any) %8: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:arrow innerArrow2(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst (:any) [?anon_1_new.target@func3]: any
// CHECK-NEXT:  %1 = ReturnInst (:any) %0: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:function_end
