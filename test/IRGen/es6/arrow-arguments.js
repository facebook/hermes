/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheck --match-full-lines %s

// Ensure that 'arguments' is captured correctly for arrow functions.

"use strict";

var dummy; // A dummy receiver to prevent optimizations.

// No special treatment of arguments here
function normal() {
    dummy = arguments[0];
}
//CHECK-LABEL:function normal()
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = CreateArgumentsInst
//CHECK-NEXT:  %1 = LoadPropertyInst %0 : object, 0 : number
//CHECK-NEXT:  %2 = StorePropertyInst %1, globalObject : object, "dummy" : string
//CHECK-NEXT:  %3 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end


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
//CHECK-LABEL:function foo()
//CHECK-NEXT:frame = [arrow1, ?anon_0_this, ?anon_1_new.target, ?anon_2_arguments]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = CreateArgumentsInst
//CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [arrow1]
//CHECK-NEXT:  %2 = StoreFrameInst %this, [?anon_0_this]
//CHECK-NEXT:  %3 = GetNewTargetInst
//CHECK-NEXT:  %4 = StoreFrameInst %3, [?anon_1_new.target]
//CHECK-NEXT:  %5 = StoreFrameInst %0 : object, [?anon_2_arguments]
//CHECK-NEXT:  %6 = CreateFunctionInst %arrow1()
//CHECK-NEXT:  %7 = StoreFrameInst %6 : closure, [arrow1]
//CHECK-NEXT:  %8 = LoadFrameInst [?anon_2_arguments]
//CHECK-NEXT:  %9 = LoadPropertyInst %8, 0 : number
//CHECK-NEXT:  %10 = StorePropertyInst %9, globalObject : object, "dummy" : string
//CHECK-NEXT:  %11 = LoadFrameInst [arrow1]
//CHECK-NEXT:  %12 = ReturnInst %11
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %13 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end

//CHECK-LABEL:arrow arrow1()
//CHECK-NEXT:frame = [arrow2]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [arrow2]
//CHECK-NEXT:  %1 = LoadFrameInst [?anon_2_arguments@foo]
//CHECK-NEXT:  %2 = LoadPropertyInst %1, 1 : number
//CHECK-NEXT:  %3 = StorePropertyInst %2, globalObject : object, "dummy" : string
//CHECK-NEXT:  %4 = CreateFunctionInst %arrow2()
//CHECK-NEXT:  %5 = StoreFrameInst %4 : closure, [arrow2]
//CHECK-NEXT:  %6 = LoadFrameInst [arrow2]
//CHECK-NEXT:  %7 = ReturnInst %6
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %8 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end

//CHECK-LABEL:arrow arrow2()
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = LoadFrameInst [?anon_2_arguments@foo]
//CHECK-NEXT:  %1 = LoadPropertyInst %0, 2 : number
//CHECK-NEXT:  %2 = StorePropertyInst %1, globalObject : object, "dummy" : string
//CHECK-NEXT:  %3 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end


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
//CHECK-LABEL:function bar()
//CHECK-NEXT:frame = [inner]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = CreateArgumentsInst
//CHECK-NEXT:  %1 = CreateFunctionInst %inner()
//CHECK-NEXT:  %2 = StoreFrameInst %1 : closure, [inner]
//CHECK-NEXT:  %3 = LoadPropertyInst %0 : object, 0 : number
//CHECK-NEXT:  %4 = StorePropertyInst %3, globalObject : object, "dummy" : string
//CHECK-NEXT:  %5 = LoadFrameInst [inner]
//CHECK-NEXT:  %6 = ReturnInst %5
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %7 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end

//CHECK-LABEL:function inner()
//CHECK-NEXT:frame = [arrow3, ?anon_0_this, ?anon_1_new.target, ?anon_2_arguments]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = CreateArgumentsInst
//CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [arrow3]
//CHECK-NEXT:  %2 = StoreFrameInst %this, [?anon_0_this]
//CHECK-NEXT:  %3 = GetNewTargetInst
//CHECK-NEXT:  %4 = StoreFrameInst %3, [?anon_1_new.target]
//CHECK-NEXT:  %5 = StoreFrameInst %0 : object, [?anon_2_arguments]
//CHECK-NEXT:  %6 = LoadFrameInst [?anon_2_arguments]
//CHECK-NEXT:  %7 = LoadPropertyInst %6, 0 : number
//CHECK-NEXT:  %8 = StorePropertyInst %7, globalObject : object, "dummy" : string
//CHECK-NEXT:  %9 = CreateFunctionInst %arrow3()
//CHECK-NEXT:  %10 = StoreFrameInst %9 : closure, [arrow3]
//CHECK-NEXT:  %11 = LoadFrameInst [arrow3]
//CHECK-NEXT:  %12 = ReturnInst %11
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %13 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end

//CHECK-LABEL:arrow arrow3()
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = LoadFrameInst [?anon_2_arguments@inner]
//CHECK-NEXT:  %1 = LoadPropertyInst %0, 1 : number
//CHECK-NEXT:  %2 = StorePropertyInst %1, globalObject : object, "dummy" : string
//CHECK-NEXT:  %3 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
