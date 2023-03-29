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
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "outer1": string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "outer2": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:closure) %outer1(): any
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2: closure, globalObject: object, "outer1": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:closure) %outer2(): any
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4: closure, globalObject: object, "outer2": string
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %7 = StoreStackInst undefined: undefined, %6: any
// CHECK-NEXT:  %8 = LoadStackInst (:any) %6: any
// CHECK-NEXT:  %9 = ReturnInst %8: any
// CHECK-NEXT:function_end

// CHECK:function outer1(): any
// CHECK-NEXT:frame = [?anon_0_this: any, ?anon_1_new.target: undefined|closure, innerArrow1: any, innerArrow2: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %this: any
// CHECK-NEXT:  %1 = CoerceThisNSInst (:object) %0: any
// CHECK-NEXT:  %2 = StoreFrameInst %1: object, [?anon_0_this]: any
// CHECK-NEXT:  %3 = GetNewTargetInst (:undefined|closure) %new.target: undefined|closure
// CHECK-NEXT:  %4 = StoreFrameInst %3: undefined|closure, [?anon_1_new.target]: undefined|closure
// CHECK-NEXT:  %5 = StoreFrameInst undefined: undefined, [innerArrow1]: any
// CHECK-NEXT:  %6 = StoreFrameInst undefined: undefined, [innerArrow2]: any
// CHECK-NEXT:  %7 = CreateFunctionInst (:closure) %innerArrow1(): any
// CHECK-NEXT:  %8 = StoreFrameInst %7: closure, [innerArrow1]: any
// CHECK-NEXT:  %9 = CreateFunctionInst (:closure) %innerArrow2(): any
// CHECK-NEXT:  %10 = StoreFrameInst %9: closure, [innerArrow2]: any
// CHECK-NEXT:  %11 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function outer2(): any
// CHECK-NEXT:frame = [?anon_0_this: any, ?anon_1_new.target: undefined|closure, inner3: any, innerArrow4: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %this: any
// CHECK-NEXT:  %1 = CoerceThisNSInst (:object) %0: any
// CHECK-NEXT:  %2 = StoreFrameInst %1: object, [?anon_0_this]: any
// CHECK-NEXT:  %3 = GetNewTargetInst (:undefined|closure) %new.target: undefined|closure
// CHECK-NEXT:  %4 = StoreFrameInst %3: undefined|closure, [?anon_1_new.target]: undefined|closure
// CHECK-NEXT:  %5 = StoreFrameInst undefined: undefined, [inner3]: any
// CHECK-NEXT:  %6 = StoreFrameInst undefined: undefined, [innerArrow4]: any
// CHECK-NEXT:  %7 = CreateFunctionInst (:closure) %inner3(): any
// CHECK-NEXT:  %8 = StoreFrameInst %7: closure, [inner3]: any
// CHECK-NEXT:  %9 = CreateFunctionInst (:closure) %innerArrow4(): any
// CHECK-NEXT:  %10 = StoreFrameInst %9: closure, [innerArrow4]: any
// CHECK-NEXT:  %11 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:arrow innerArrow1(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst (:any) [?anon_0_this@outer1]: any
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) %0: any, "x": string
// CHECK-NEXT:  %2 = ReturnInst %1: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:arrow innerArrow2(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst (:any) [?anon_0_this@outer1]: any
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) %0: any, "y": string
// CHECK-NEXT:  %2 = ReturnInst %1: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function inner3(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %this: any
// CHECK-NEXT:  %1 = CoerceThisNSInst (:object) %0: any
// CHECK-NEXT:  %2 = LoadPropertyInst (:any) %1: object, "a": string
// CHECK-NEXT:  %3 = ReturnInst %2: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:arrow innerArrow4(): any
// CHECK-NEXT:frame = [nestedInnerArrow5: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined: undefined, [nestedInnerArrow5]: any
// CHECK-NEXT:  %1 = LoadFrameInst (:any) [?anon_0_this@outer2]: any
// CHECK-NEXT:  %2 = StorePropertyLooseInst 10: number, %1: any, "b": string
// CHECK-NEXT:  %3 = CreateFunctionInst (:closure) %nestedInnerArrow5(): any
// CHECK-NEXT:  %4 = StoreFrameInst %3: closure, [nestedInnerArrow5]: any
// CHECK-NEXT:  %5 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:arrow nestedInnerArrow5(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst (:any) [?anon_0_this@outer2]: any
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) %0: any, "b": string
// CHECK-NEXT:  %2 = ReturnInst %1: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end
