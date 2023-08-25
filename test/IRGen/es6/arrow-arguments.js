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
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %normal(): any
// CHECK-NEXT:  %5 = StorePropertyStrictInst %4: object, globalObject: object, "normal": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %foo(): any
// CHECK-NEXT:  %7 = StorePropertyStrictInst %6: object, globalObject: object, "foo": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %bar(): any
// CHECK-NEXT:  %9 = StorePropertyStrictInst %8: object, globalObject: object, "bar": string
// CHECK-NEXT:  %10 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %11 = StoreStackInst undefined: undefined, %10: any
// CHECK-NEXT:  %12 = StoreStackInst "use strict": string, %10: any
// CHECK-NEXT:  %13 = LoadStackInst (:any) %10: any
// CHECK-NEXT:  %14 = ReturnInst %13: any
// CHECK-NEXT:function_end

// CHECK:function normal(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsStrictInst (:object)
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) %0: object, 0: number
// CHECK-NEXT:  %2 = StorePropertyStrictInst %1: any, globalObject: object, "dummy": string
// CHECK-NEXT:  %3 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function foo(): any
// CHECK-NEXT:frame = [?anon_0_this: any, ?anon_1_new.target: undefined|object, ?anon_2_arguments: object, arrow1: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsStrictInst (:object)
// CHECK-NEXT:  %1 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %2 = StoreFrameInst %1: any, [?anon_0_this]: any
// CHECK-NEXT:  %3 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:  %4 = StoreFrameInst %3: undefined|object, [?anon_1_new.target]: undefined|object
// CHECK-NEXT:  %5 = StoreFrameInst %0: object, [?anon_2_arguments]: object
// CHECK-NEXT:  %6 = StoreFrameInst undefined: undefined, [arrow1]: any
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %arrow1(): any
// CHECK-NEXT:  %8 = StoreFrameInst %7: object, [arrow1]: any
// CHECK-NEXT:  %9 = LoadFrameInst (:object) [?anon_2_arguments]: object
// CHECK-NEXT:  %10 = LoadPropertyInst (:any) %9: object, 0: number
// CHECK-NEXT:  %11 = StorePropertyStrictInst %10: any, globalObject: object, "dummy": string
// CHECK-NEXT:  %12 = LoadFrameInst (:any) [arrow1]: any
// CHECK-NEXT:  %13 = ReturnInst %12: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %14 = UnreachableInst
// CHECK-NEXT:function_end

// CHECK:function bar(): any
// CHECK-NEXT:frame = [inner: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsStrictInst (:object)
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %inner(): any
// CHECK-NEXT:  %2 = StoreFrameInst %1: object, [inner]: any
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) %0: object, 0: number
// CHECK-NEXT:  %4 = StorePropertyStrictInst %3: any, globalObject: object, "dummy": string
// CHECK-NEXT:  %5 = LoadFrameInst (:any) [inner]: any
// CHECK-NEXT:  %6 = ReturnInst %5: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = UnreachableInst
// CHECK-NEXT:function_end

// CHECK:arrow arrow1(): any
// CHECK-NEXT:frame = [arrow2: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined: undefined, [arrow2]: any
// CHECK-NEXT:  %1 = LoadFrameInst (:object) [?anon_2_arguments@foo]: object
// CHECK-NEXT:  %2 = LoadPropertyInst (:any) %1: object, 1: number
// CHECK-NEXT:  %3 = StorePropertyStrictInst %2: any, globalObject: object, "dummy": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %arrow2(): any
// CHECK-NEXT:  %5 = StoreFrameInst %4: object, [arrow2]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) [arrow2]: any
// CHECK-NEXT:  %7 = ReturnInst %6: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function inner(): any
// CHECK-NEXT:frame = [?anon_0_this: any, ?anon_1_new.target: undefined|object, ?anon_2_arguments: object, arrow3: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsStrictInst (:object)
// CHECK-NEXT:  %1 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %2 = StoreFrameInst %1: any, [?anon_0_this]: any
// CHECK-NEXT:  %3 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:  %4 = StoreFrameInst %3: undefined|object, [?anon_1_new.target]: undefined|object
// CHECK-NEXT:  %5 = StoreFrameInst %0: object, [?anon_2_arguments]: object
// CHECK-NEXT:  %6 = StoreFrameInst undefined: undefined, [arrow3]: any
// CHECK-NEXT:  %7 = LoadFrameInst (:object) [?anon_2_arguments]: object
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) %7: object, 0: number
// CHECK-NEXT:  %9 = StorePropertyStrictInst %8: any, globalObject: object, "dummy": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %arrow3(): any
// CHECK-NEXT:  %11 = StoreFrameInst %10: object, [arrow3]: any
// CHECK-NEXT:  %12 = LoadFrameInst (:any) [arrow3]: any
// CHECK-NEXT:  %13 = ReturnInst %12: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %14 = UnreachableInst
// CHECK-NEXT:function_end

// CHECK:arrow arrow2(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst (:object) [?anon_2_arguments@foo]: object
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) %0: object, 2: number
// CHECK-NEXT:  %2 = StorePropertyStrictInst %1: any, globalObject: object, "dummy": string
// CHECK-NEXT:  %3 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:arrow arrow3(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst (:object) [?anon_2_arguments@inner]: object
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) %0: object, 1: number
// CHECK-NEXT:  %2 = StorePropertyStrictInst %1: any, globalObject: object, "dummy": string
// CHECK-NEXT:  %3 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end
