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

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [outer1, outer2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %outer1#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "outer1" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %outer2#0#1()#5, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "outer2" : string
// CHECK-NEXT:  %5 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %6 = StoreStackInst undefined : undefined, %5
// CHECK-NEXT:  %7 = LoadStackInst %5
// CHECK-NEXT:  %8 = ReturnInst %7
// CHECK-NEXT:function_end

// CHECK:function outer1#0#1()#2
// CHECK-NEXT:frame = [?anon_0_this#2, ?anon_1_new.target#2, innerArrow1#2, innerArrow2#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{outer1#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst %this, [?anon_0_this#2], %0
// CHECK-NEXT:  %2 = GetNewTargetInst
// CHECK-NEXT:  %3 = StoreFrameInst %2, [?anon_1_new.target#2], %0
// CHECK-NEXT:  %4 = StoreFrameInst undefined : undefined, [innerArrow1#2], %0
// CHECK-NEXT:  %5 = StoreFrameInst undefined : undefined, [innerArrow2#2], %0
// CHECK-NEXT:  %6 = CreateFunctionInst %innerArrow1#1#2()#3, %0
// CHECK-NEXT:  %7 = StoreFrameInst %6 : closure, [innerArrow1#2], %0
// CHECK-NEXT:  %8 = CreateFunctionInst %innerArrow2#1#2()#4, %0
// CHECK-NEXT:  %9 = StoreFrameInst %8 : closure, [innerArrow2#2], %0
// CHECK-NEXT:  %10 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:arrow innerArrow1#1#2()#3
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{innerArrow1#1#2()#3}
// CHECK-NEXT:  %1 = LoadFrameInst [?anon_0_this#2@outer1], %0
// CHECK-NEXT:  %2 = LoadPropertyInst %1, "x" : string
// CHECK-NEXT:  %3 = ReturnInst %2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:arrow innerArrow2#1#2()#4
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{innerArrow2#1#2()#4}
// CHECK-NEXT:  %1 = LoadFrameInst [?anon_0_this#2@outer1], %0
// CHECK-NEXT:  %2 = LoadPropertyInst %1, "y" : string
// CHECK-NEXT:  %3 = ReturnInst %2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function outer2#0#1()#5
// CHECK-NEXT:frame = [?anon_0_this#5, ?anon_1_new.target#5, innerArrow4#5, inner3#5]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{outer2#0#1()#5}
// CHECK-NEXT:  %1 = StoreFrameInst %this, [?anon_0_this#5], %0
// CHECK-NEXT:  %2 = GetNewTargetInst
// CHECK-NEXT:  %3 = StoreFrameInst %2, [?anon_1_new.target#5], %0
// CHECK-NEXT:  %4 = StoreFrameInst undefined : undefined, [innerArrow4#5], %0
// CHECK-NEXT:  %5 = CreateFunctionInst %inner3#1#5()#6, %0
// CHECK-NEXT:  %6 = StoreFrameInst %5 : closure, [inner3#5], %0
// CHECK-NEXT:  %7 = CreateFunctionInst %innerArrow4#1#5()#7, %0
// CHECK-NEXT:  %8 = StoreFrameInst %7 : closure, [innerArrow4#5], %0
// CHECK-NEXT:  %9 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function inner3#1#5()#6
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{inner3#1#5()#6}
// CHECK-NEXT:  %1 = LoadPropertyInst %this, "a" : string
// CHECK-NEXT:  %2 = ReturnInst %1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:arrow innerArrow4#1#5()#7
// CHECK-NEXT:frame = [nestedInnerArrow5#7]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{innerArrow4#1#5()#7}
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [nestedInnerArrow5#7], %0
// CHECK-NEXT:  %2 = LoadFrameInst [?anon_0_this#5@outer2], %0
// CHECK-NEXT:  %3 = StorePropertyInst 10 : number, %2, "b" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %nestedInnerArrow5#5#7()#8, %0
// CHECK-NEXT:  %5 = StoreFrameInst %4 : closure, [nestedInnerArrow5#7], %0
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:arrow nestedInnerArrow5#5#7()#8
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{nestedInnerArrow5#5#7()#8}
// CHECK-NEXT:  %1 = LoadFrameInst [?anon_0_this#5@outer2], %0
// CHECK-NEXT:  %2 = LoadPropertyInst %1, "b" : string
// CHECK-NEXT:  %3 = ReturnInst %2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
