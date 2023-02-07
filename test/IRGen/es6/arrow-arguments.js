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

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "dummy" : string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "normal" : string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "foo" : string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "bar" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %normal()
// CHECK-NEXT:  %5 = StorePropertyStrictInst %4 : closure, globalObject : object, "normal" : string
// CHECK-NEXT:  %6 = CreateFunctionInst %foo()
// CHECK-NEXT:  %7 = StorePropertyStrictInst %6 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %8 = CreateFunctionInst %bar()
// CHECK-NEXT:  %9 = StorePropertyStrictInst %8 : closure, globalObject : object, "bar" : string
// CHECK-NEXT:  %10 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %11 = StoreStackInst undefined : undefined, %10
// CHECK-NEXT:  %12 = StoreStackInst "use strict" : string, %10
// CHECK-NEXT:  %13 = LoadStackInst %10
// CHECK-NEXT:  %14 = ReturnInst %13
// CHECK-NEXT:function_end

// CHECK:function normal()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsInst
// CHECK-NEXT:  %1 = LoadPropertyInst %0 : object, 0 : number
// CHECK-NEXT:  %2 = StorePropertyStrictInst %1, globalObject : object, "dummy" : string
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function foo()
// CHECK-NEXT:frame = [?anon_0_this, ?anon_1_new.target, ?anon_2_arguments, arrow1]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsInst
// CHECK-NEXT:  %1 = LoadParamInst %this
// CHECK-NEXT:  %2 = StoreFrameInst %1, [?anon_0_this]
// CHECK-NEXT:  %3 = GetNewTargetInst %new.target
// CHECK-NEXT:  %4 = StoreFrameInst %3, [?anon_1_new.target]
// CHECK-NEXT:  %5 = StoreFrameInst %0 : object, [?anon_2_arguments]
// CHECK-NEXT:  %6 = StoreFrameInst undefined : undefined, [arrow1]
// CHECK-NEXT:  %7 = CreateFunctionInst %arrow1()
// CHECK-NEXT:  %8 = StoreFrameInst %7 : closure, [arrow1]
// CHECK-NEXT:  %9 = LoadFrameInst [?anon_2_arguments]
// CHECK-NEXT:  %10 = LoadPropertyInst %9, 0 : number
// CHECK-NEXT:  %11 = StorePropertyStrictInst %10, globalObject : object, "dummy" : string
// CHECK-NEXT:  %12 = LoadFrameInst [arrow1]
// CHECK-NEXT:  %13 = ReturnInst %12
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %14 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function bar()
// CHECK-NEXT:frame = [inner]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsInst
// CHECK-NEXT:  %1 = CreateFunctionInst %inner()
// CHECK-NEXT:  %2 = StoreFrameInst %1 : closure, [inner]
// CHECK-NEXT:  %3 = LoadPropertyInst %0 : object, 0 : number
// CHECK-NEXT:  %4 = StorePropertyStrictInst %3, globalObject : object, "dummy" : string
// CHECK-NEXT:  %5 = LoadFrameInst [inner]
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:arrow arrow1()
// CHECK-NEXT:frame = [arrow2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [arrow2]
// CHECK-NEXT:  %1 = LoadFrameInst [?anon_2_arguments@foo]
// CHECK-NEXT:  %2 = LoadPropertyInst %1, 1 : number
// CHECK-NEXT:  %3 = StorePropertyStrictInst %2, globalObject : object, "dummy" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %arrow2()
// CHECK-NEXT:  %5 = StoreFrameInst %4 : closure, [arrow2]
// CHECK-NEXT:  %6 = LoadFrameInst [arrow2]
// CHECK-NEXT:  %7 = ReturnInst %6
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function inner()
// CHECK-NEXT:frame = [?anon_0_this, ?anon_1_new.target, ?anon_2_arguments, arrow3]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsInst
// CHECK-NEXT:  %1 = LoadParamInst %this
// CHECK-NEXT:  %2 = StoreFrameInst %1, [?anon_0_this]
// CHECK-NEXT:  %3 = GetNewTargetInst %new.target
// CHECK-NEXT:  %4 = StoreFrameInst %3, [?anon_1_new.target]
// CHECK-NEXT:  %5 = StoreFrameInst %0 : object, [?anon_2_arguments]
// CHECK-NEXT:  %6 = StoreFrameInst undefined : undefined, [arrow3]
// CHECK-NEXT:  %7 = LoadFrameInst [?anon_2_arguments]
// CHECK-NEXT:  %8 = LoadPropertyInst %7, 0 : number
// CHECK-NEXT:  %9 = StorePropertyStrictInst %8, globalObject : object, "dummy" : string
// CHECK-NEXT:  %10 = CreateFunctionInst %arrow3()
// CHECK-NEXT:  %11 = StoreFrameInst %10 : closure, [arrow3]
// CHECK-NEXT:  %12 = LoadFrameInst [arrow3]
// CHECK-NEXT:  %13 = ReturnInst %12
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %14 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:arrow arrow2()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst [?anon_2_arguments@foo]
// CHECK-NEXT:  %1 = LoadPropertyInst %0, 2 : number
// CHECK-NEXT:  %2 = StorePropertyStrictInst %1, globalObject : object, "dummy" : string
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:arrow arrow3()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst [?anon_2_arguments@inner]
// CHECK-NEXT:  %1 = LoadPropertyInst %0, 1 : number
// CHECK-NEXT:  %2 = StorePropertyStrictInst %1, globalObject : object, "dummy" : string
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
