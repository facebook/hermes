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

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "outer1" : string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "outer2" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %outer1()
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2 : closure, globalObject : object, "outer1" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %outer2()
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4 : closure, globalObject : object, "outer2" : string
// CHECK-NEXT:  %6 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %7 = StoreStackInst undefined : undefined, %6
// CHECK-NEXT:  %8 = LoadStackInst %6
// CHECK-NEXT:  %9 = ReturnInst %8
// CHECK-NEXT:function_end

// CHECK:function outer1()
// CHECK-NEXT:frame = [?anon_0_this, ?anon_1_new.target, innerArrow1, innerArrow2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %this
// CHECK-NEXT:  %1 = CoerceThisNSInst %0
// CHECK-NEXT:  %2 = StoreFrameInst %1 : object, [?anon_0_this]
// CHECK-NEXT:  %3 = GetNewTargetInst
// CHECK-NEXT:  %4 = StoreFrameInst %3, [?anon_1_new.target]
// CHECK-NEXT:  %5 = StoreFrameInst undefined : undefined, [innerArrow1]
// CHECK-NEXT:  %6 = StoreFrameInst undefined : undefined, [innerArrow2]
// CHECK-NEXT:  %7 = CreateFunctionInst %innerArrow1()
// CHECK-NEXT:  %8 = StoreFrameInst %7 : closure, [innerArrow1]
// CHECK-NEXT:  %9 = CreateFunctionInst %innerArrow2()
// CHECK-NEXT:  %10 = StoreFrameInst %9 : closure, [innerArrow2]
// CHECK-NEXT:  %11 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function outer2()
// CHECK-NEXT:frame = [?anon_0_this, ?anon_1_new.target, inner3, innerArrow4]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %this
// CHECK-NEXT:  %1 = CoerceThisNSInst %0
// CHECK-NEXT:  %2 = StoreFrameInst %1 : object, [?anon_0_this]
// CHECK-NEXT:  %3 = GetNewTargetInst
// CHECK-NEXT:  %4 = StoreFrameInst %3, [?anon_1_new.target]
// CHECK-NEXT:  %5 = StoreFrameInst undefined : undefined, [innerArrow4]
// CHECK-NEXT:  %6 = CreateFunctionInst %inner3()
// CHECK-NEXT:  %7 = StoreFrameInst %6 : closure, [inner3]
// CHECK-NEXT:  %8 = CreateFunctionInst %innerArrow4()
// CHECK-NEXT:  %9 = StoreFrameInst %8 : closure, [innerArrow4]
// CHECK-NEXT:  %10 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:arrow innerArrow1()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst [?anon_0_this@outer1]
// CHECK-NEXT:  %1 = LoadPropertyInst %0, "x" : string
// CHECK-NEXT:  %2 = ReturnInst %1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:arrow innerArrow2()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst [?anon_0_this@outer1]
// CHECK-NEXT:  %1 = LoadPropertyInst %0, "y" : string
// CHECK-NEXT:  %2 = ReturnInst %1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function inner3()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %this
// CHECK-NEXT:  %1 = CoerceThisNSInst %0
// CHECK-NEXT:  %2 = LoadPropertyInst %1 : object, "a" : string
// CHECK-NEXT:  %3 = ReturnInst %2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:arrow innerArrow4()
// CHECK-NEXT:frame = [nestedInnerArrow5]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [nestedInnerArrow5]
// CHECK-NEXT:  %1 = LoadFrameInst [?anon_0_this@outer2]
// CHECK-NEXT:  %2 = StorePropertyLooseInst 10 : number, %1, "b" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %nestedInnerArrow5()
// CHECK-NEXT:  %4 = StoreFrameInst %3 : closure, [nestedInnerArrow5]
// CHECK-NEXT:  %5 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:arrow nestedInnerArrow5()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst [?anon_0_this@outer2]
// CHECK-NEXT:  %1 = LoadPropertyInst %0, "b" : string
// CHECK-NEXT:  %2 = ReturnInst %1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
