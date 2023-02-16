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

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "dummy": string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "normal": string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "bar": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:closure) %normal(): any
// CHECK-NEXT:  %5 = StorePropertyStrictInst %4: closure, globalObject: object, "normal": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:closure) %foo(): any
// CHECK-NEXT:  %7 = StorePropertyStrictInst %6: closure, globalObject: object, "foo": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:closure) %bar(): any
// CHECK-NEXT:  %9 = StorePropertyStrictInst %8: closure, globalObject: object, "bar": string
// CHECK-NEXT:  %10 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %11 = StoreStackInst undefined: undefined, %10: any
// CHECK-NEXT:  %12 = StoreStackInst "use strict": string, %10: any
// CHECK-NEXT:  %13 = LoadStackInst (:any) %10: any
// CHECK-NEXT:  %14 = ReturnInst (:any) %13: any
// CHECK-NEXT:function_end

// CHECK:function normal(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsInst (:object)
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) %0: object, 0: number
// CHECK-NEXT:  %2 = StorePropertyStrictInst %1: any, globalObject: object, "dummy": string
// CHECK-NEXT:  %3 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function foo(): any
// CHECK-NEXT:frame = [?anon_0_this: any, ?anon_1_new.target: any, ?anon_2_arguments: any, arrow1: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsInst (:object)
// CHECK-NEXT:  %1 = LoadParamInst (:any) %this: any
// CHECK-NEXT:  %2 = StoreFrameInst %1: any, [?anon_0_this]: any
// CHECK-NEXT:  %3 = GetNewTargetInst (:any) %new.target: any
// CHECK-NEXT:  %4 = StoreFrameInst %3: any, [?anon_1_new.target]: any
// CHECK-NEXT:  %5 = StoreFrameInst %0: object, [?anon_2_arguments]: any
// CHECK-NEXT:  %6 = StoreFrameInst undefined: undefined, [arrow1]: any
// CHECK-NEXT:  %7 = CreateFunctionInst (:closure) %arrow1(): any
// CHECK-NEXT:  %8 = StoreFrameInst %7: closure, [arrow1]: any
// CHECK-NEXT:  %9 = LoadFrameInst (:any) [?anon_2_arguments]: any
// CHECK-NEXT:  %10 = LoadPropertyInst (:any) %9: any, 0: number
// CHECK-NEXT:  %11 = StorePropertyStrictInst %10: any, globalObject: object, "dummy": string
// CHECK-NEXT:  %12 = LoadFrameInst (:any) [arrow1]: any
// CHECK-NEXT:  %13 = ReturnInst (:any) %12: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %14 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function bar(): any
// CHECK-NEXT:frame = [inner: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsInst (:object)
// CHECK-NEXT:  %1 = CreateFunctionInst (:closure) %inner(): any
// CHECK-NEXT:  %2 = StoreFrameInst %1: closure, [inner]: any
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) %0: object, 0: number
// CHECK-NEXT:  %4 = StorePropertyStrictInst %3: any, globalObject: object, "dummy": string
// CHECK-NEXT:  %5 = LoadFrameInst (:any) [inner]: any
// CHECK-NEXT:  %6 = ReturnInst (:any) %5: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:arrow arrow1(): any
// CHECK-NEXT:frame = [arrow2: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined: undefined, [arrow2]: any
// CHECK-NEXT:  %1 = LoadFrameInst (:any) [?anon_2_arguments@foo]: any
// CHECK-NEXT:  %2 = LoadPropertyInst (:any) %1: any, 1: number
// CHECK-NEXT:  %3 = StorePropertyStrictInst %2: any, globalObject: object, "dummy": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:closure) %arrow2(): any
// CHECK-NEXT:  %5 = StoreFrameInst %4: closure, [arrow2]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) [arrow2]: any
// CHECK-NEXT:  %7 = ReturnInst (:any) %6: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function inner(): any
// CHECK-NEXT:frame = [?anon_0_this: any, ?anon_1_new.target: any, ?anon_2_arguments: any, arrow3: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsInst (:object)
// CHECK-NEXT:  %1 = LoadParamInst (:any) %this: any
// CHECK-NEXT:  %2 = StoreFrameInst %1: any, [?anon_0_this]: any
// CHECK-NEXT:  %3 = GetNewTargetInst (:any) %new.target: any
// CHECK-NEXT:  %4 = StoreFrameInst %3: any, [?anon_1_new.target]: any
// CHECK-NEXT:  %5 = StoreFrameInst %0: object, [?anon_2_arguments]: any
// CHECK-NEXT:  %6 = StoreFrameInst undefined: undefined, [arrow3]: any
// CHECK-NEXT:  %7 = LoadFrameInst (:any) [?anon_2_arguments]: any
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) %7: any, 0: number
// CHECK-NEXT:  %9 = StorePropertyStrictInst %8: any, globalObject: object, "dummy": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:closure) %arrow3(): any
// CHECK-NEXT:  %11 = StoreFrameInst %10: closure, [arrow3]: any
// CHECK-NEXT:  %12 = LoadFrameInst (:any) [arrow3]: any
// CHECK-NEXT:  %13 = ReturnInst (:any) %12: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %14 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:arrow arrow2(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst (:any) [?anon_2_arguments@foo]: any
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) %0: any, 2: number
// CHECK-NEXT:  %2 = StorePropertyStrictInst %1: any, globalObject: object, "dummy": string
// CHECK-NEXT:  %3 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:arrow arrow3(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst (:any) [?anon_2_arguments@inner]: any
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) %0: any, 1: number
// CHECK-NEXT:  %2 = StorePropertyStrictInst %1: any, globalObject: object, "dummy": string
// CHECK-NEXT:  %3 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:function_end
