/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

// Ensure that 'arguments' is captured correctly for arrow functions.

"use strict";

var dummy; // A dummy receiver to prevent optimizations.

// No special treatment of arguments here
function normal() {
    dummy = arguments[0];
}

function foo() {
    var arrow1 = () => {
        dummy = arguments[1];
        var arrow2 = () => {
            dummy = arguments[2];
        }
        return arrow2;
    }
    dummy = arguments[0];
    return arrow1;
}

// Here only inner's arguments should be captured.
function bar() {
    dummy = arguments[0];
    function inner() {
        dummy = arguments[0];
        var arrow3 = () => {
            dummy = arguments[1];
        }
        return arrow3;
    }
    return inner;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [dummy, normal, foo, bar]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %normal#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "normal" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %foo#0#1()#3, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %bar#0#1()#6, %0
// CHECK-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "bar" : string
// CHECK-NEXT:  %7 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %8 = StoreStackInst undefined : undefined, %7
// CHECK-NEXT:  %9 = StoreStackInst "use strict" : string, %7
// CHECK-NEXT:  %10 = LoadStackInst %7
// CHECK-NEXT:  %11 = ReturnInst %10
// CHECK-NEXT:function_end

// CHECK:function normal#0#1()#2
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{normal#0#1()#2}
// CHECK-NEXT:  %1 = CreateArgumentsInst
// CHECK-NEXT:  %2 = LoadPropertyInst %1 : object, 0 : number
// CHECK-NEXT:  %3 = StorePropertyInst %2, globalObject : object, "dummy" : string
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function foo#0#1()#3
// CHECK-NEXT:frame = [?anon_0_this#3, ?anon_1_new.target#3, ?anon_2_arguments#3, arrow1#3]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{foo#0#1()#3}
// CHECK-NEXT:  %1 = CreateArgumentsInst
// CHECK-NEXT:  %2 = StoreFrameInst %this, [?anon_0_this#3], %0
// CHECK-NEXT:  %3 = GetNewTargetInst
// CHECK-NEXT:  %4 = StoreFrameInst %3, [?anon_1_new.target#3], %0
// CHECK-NEXT:  %5 = StoreFrameInst %1 : object, [?anon_2_arguments#3], %0
// CHECK-NEXT:  %6 = StoreFrameInst undefined : undefined, [arrow1#3], %0
// CHECK-NEXT:  %7 = CreateFunctionInst %arrow1#1#3()#4, %0
// CHECK-NEXT:  %8 = StoreFrameInst %7 : closure, [arrow1#3], %0
// CHECK-NEXT:  %9 = LoadFrameInst [?anon_2_arguments#3], %0
// CHECK-NEXT:  %10 = LoadPropertyInst %9, 0 : number
// CHECK-NEXT:  %11 = StorePropertyInst %10, globalObject : object, "dummy" : string
// CHECK-NEXT:  %12 = LoadFrameInst [arrow1#3], %0
// CHECK-NEXT:  %13 = ReturnInst %12
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %14 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:arrow arrow1#1#3()#4
// CHECK-NEXT:frame = [arrow2#4]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{arrow1#1#3()#4}
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [arrow2#4], %0
// CHECK-NEXT:  %2 = LoadFrameInst [?anon_2_arguments#3@foo], %0
// CHECK-NEXT:  %3 = LoadPropertyInst %2, 1 : number
// CHECK-NEXT:  %4 = StorePropertyInst %3, globalObject : object, "dummy" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %arrow2#3#4()#5, %0
// CHECK-NEXT:  %6 = StoreFrameInst %5 : closure, [arrow2#4], %0
// CHECK-NEXT:  %7 = LoadFrameInst [arrow2#4], %0
// CHECK-NEXT:  %8 = ReturnInst %7
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:arrow arrow2#3#4()#5
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{arrow2#3#4()#5}
// CHECK-NEXT:  %1 = LoadFrameInst [?anon_2_arguments#3@foo], %0
// CHECK-NEXT:  %2 = LoadPropertyInst %1, 2 : number
// CHECK-NEXT:  %3 = StorePropertyInst %2, globalObject : object, "dummy" : string
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function bar#0#1()#6
// CHECK-NEXT:frame = [inner#6]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{bar#0#1()#6}
// CHECK-NEXT:  %1 = CreateArgumentsInst
// CHECK-NEXT:  %2 = CreateFunctionInst %inner#1#6()#7, %0
// CHECK-NEXT:  %3 = StoreFrameInst %2 : closure, [inner#6], %0
// CHECK-NEXT:  %4 = LoadPropertyInst %1 : object, 0 : number
// CHECK-NEXT:  %5 = StorePropertyInst %4, globalObject : object, "dummy" : string
// CHECK-NEXT:  %6 = LoadFrameInst [inner#6], %0
// CHECK-NEXT:  %7 = ReturnInst %6
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function inner#1#6()#7
// CHECK-NEXT:frame = [?anon_0_this#7, ?anon_1_new.target#7, ?anon_2_arguments#7, arrow3#7]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{inner#1#6()#7}
// CHECK-NEXT:  %1 = CreateArgumentsInst
// CHECK-NEXT:  %2 = StoreFrameInst %this, [?anon_0_this#7], %0
// CHECK-NEXT:  %3 = GetNewTargetInst
// CHECK-NEXT:  %4 = StoreFrameInst %3, [?anon_1_new.target#7], %0
// CHECK-NEXT:  %5 = StoreFrameInst %1 : object, [?anon_2_arguments#7], %0
// CHECK-NEXT:  %6 = StoreFrameInst undefined : undefined, [arrow3#7], %0
// CHECK-NEXT:  %7 = LoadFrameInst [?anon_2_arguments#7], %0
// CHECK-NEXT:  %8 = LoadPropertyInst %7, 0 : number
// CHECK-NEXT:  %9 = StorePropertyInst %8, globalObject : object, "dummy" : string
// CHECK-NEXT:  %10 = CreateFunctionInst %arrow3#6#7()#8, %0
// CHECK-NEXT:  %11 = StoreFrameInst %10 : closure, [arrow3#7], %0
// CHECK-NEXT:  %12 = LoadFrameInst [arrow3#7], %0
// CHECK-NEXT:  %13 = ReturnInst %12
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %14 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:arrow arrow3#6#7()#8
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{arrow3#6#7()#8}
// CHECK-NEXT:  %1 = LoadFrameInst [?anon_2_arguments#7@inner], %0
// CHECK-NEXT:  %2 = LoadPropertyInst %1, 1 : number
// CHECK-NEXT:  %3 = StorePropertyInst %2, globalObject : object, "dummy" : string
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
