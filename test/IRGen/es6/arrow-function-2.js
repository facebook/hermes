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
// CHECK-NEXT:frame = [], globals = [outer1, outer2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %outer1()
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "outer1" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %outer2()
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2 : closure, globalObject : object, "outer2" : string
// CHECK-NEXT:  %4 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %5 = StoreStackInst undefined : undefined, %4
// CHECK-NEXT:  %6 = LoadStackInst %4
// CHECK-NEXT:  %7 = ReturnInst %6
// CHECK-NEXT:function_end

// CHECK:function outer1()
// CHECK-NEXT:frame = [innerArrow1, innerArrow2, ?anon_0_this, ?anon_1_new.target]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [innerArrow1]
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [innerArrow2]
// CHECK-NEXT:  %2 = StoreFrameInst %this, [?anon_0_this]
// CHECK-NEXT:  %3 = GetNewTargetInst
// CHECK-NEXT:  %4 = StoreFrameInst %3, [?anon_1_new.target]
// CHECK-NEXT:  %5 = CreateFunctionInst %innerArrow1()
// CHECK-NEXT:  %6 = StoreFrameInst %5 : closure, [innerArrow1]
// CHECK-NEXT:  %7 = CreateFunctionInst %innerArrow2()
// CHECK-NEXT:  %8 = StoreFrameInst %7 : closure, [innerArrow2]
// CHECK-NEXT:  %9 = ReturnInst undefined : undefined
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

// CHECK:function outer2()
// CHECK-NEXT:frame = [innerArrow4, inner3, ?anon_0_this, ?anon_1_new.target]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [innerArrow4]
// CHECK-NEXT:  %1 = StoreFrameInst %this, [?anon_0_this]
// CHECK-NEXT:  %2 = GetNewTargetInst
// CHECK-NEXT:  %3 = StoreFrameInst %2, [?anon_1_new.target]
// CHECK-NEXT:  %4 = CreateFunctionInst %inner3()
// CHECK-NEXT:  %5 = StoreFrameInst %4 : closure, [inner3]
// CHECK-NEXT:  %6 = CreateFunctionInst %innerArrow4()
// CHECK-NEXT:  %7 = StoreFrameInst %6 : closure, [innerArrow4]
// CHECK-NEXT:  %8 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function inner3()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadPropertyInst %this, "a" : string
// CHECK-NEXT:  %1 = ReturnInst %0
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
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
