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
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "dummy": string
// CHECK-NEXT:       DeclareGlobalVarInst "normal": string
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:       DeclareGlobalVarInst "bar": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %0: environment, %normal(): functionCode
// CHECK-NEXT:       StorePropertyStrictInst %5: object, globalObject: object, "normal": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %0: environment, %foo(): functionCode
// CHECK-NEXT:       StorePropertyStrictInst %7: object, globalObject: object, "foo": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %0: environment, %bar(): functionCode
// CHECK-NEXT:        StorePropertyStrictInst %9: object, globalObject: object, "bar": string
// CHECK-NEXT:  %11 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %11: any
// CHECK-NEXT:        StoreStackInst "use strict": string, %11: any
// CHECK-NEXT:  %14 = LoadStackInst (:any) %11: any
// CHECK-NEXT:        ReturnInst %14: any
// CHECK-NEXT:function_end

// CHECK:function normal(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsStrictInst (:object)
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %normal(): any, %1: environment
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) %0: object, 0: number
// CHECK-NEXT:       StorePropertyStrictInst %3: any, globalObject: object, "dummy": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function foo(): any
// CHECK-NEXT:frame = [?anon_0_this: any, ?anon_1_new.target: undefined|object, ?anon_2_arguments: object, arrow1: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsStrictInst (:object)
// CHECK-NEXT:  %1 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %2 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %3 = CreateScopeInst (:environment) %foo(): any, %2: environment
// CHECK-NEXT:       StoreFrameInst %3: environment, %1: any, [?anon_0_this]: any
// CHECK-NEXT:  %5 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:       StoreFrameInst %3: environment, %5: undefined|object, [?anon_1_new.target]: undefined|object
// CHECK-NEXT:       StoreFrameInst %3: environment, %0: object, [?anon_2_arguments]: object
// CHECK-NEXT:       StoreFrameInst %3: environment, undefined: undefined, [arrow1]: any
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %3: environment, %arrow1(): functionCode
// CHECK-NEXT:        StoreFrameInst %3: environment, %9: object, [arrow1]: any
// CHECK-NEXT:  %11 = LoadFrameInst (:object) %3: environment, [?anon_2_arguments]: object
// CHECK-NEXT:  %12 = LoadPropertyInst (:any) %11: object, 0: number
// CHECK-NEXT:        StorePropertyStrictInst %12: any, globalObject: object, "dummy": string
// CHECK-NEXT:  %14 = LoadFrameInst (:any) %3: environment, [arrow1]: any
// CHECK-NEXT:        ReturnInst %14: any
// CHECK-NEXT:function_end

// CHECK:function bar(): any
// CHECK-NEXT:frame = [inner: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsStrictInst (:object)
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %bar(): any, %1: environment
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %2: environment, %inner(): functionCode
// CHECK-NEXT:       StoreFrameInst %2: environment, %3: object, [inner]: any
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) %0: object, 0: number
// CHECK-NEXT:       StorePropertyStrictInst %5: any, globalObject: object, "dummy": string
// CHECK-NEXT:  %7 = LoadFrameInst (:any) %2: environment, [inner]: any
// CHECK-NEXT:       ReturnInst %7: any
// CHECK-NEXT:function_end

// CHECK:arrow arrow1(): any
// CHECK-NEXT:frame = [arrow2: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %foo(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %arrow1(): any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [arrow2]: any
// CHECK-NEXT:  %3 = ResolveScopeInst (:environment) %foo(): any, %1: environment
// CHECK-NEXT:  %4 = LoadFrameInst (:object) %3: environment, [?anon_2_arguments@foo]: object
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) %4: object, 1: number
// CHECK-NEXT:       StorePropertyStrictInst %5: any, globalObject: object, "dummy": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %1: environment, %arrow2(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %7: object, [arrow2]: any
// CHECK-NEXT:  %9 = LoadFrameInst (:any) %1: environment, [arrow2]: any
// CHECK-NEXT:        ReturnInst %9: any
// CHECK-NEXT:function_end

// CHECK:function inner(): any
// CHECK-NEXT:frame = [?anon_0_this: any, ?anon_1_new.target: undefined|object, ?anon_2_arguments: object, arrow3: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsStrictInst (:object)
// CHECK-NEXT:  %1 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %2 = GetParentScopeInst (:environment) %bar(): any, %parentScope: environment
// CHECK-NEXT:  %3 = CreateScopeInst (:environment) %inner(): any, %2: environment
// CHECK-NEXT:       StoreFrameInst %3: environment, %1: any, [?anon_0_this]: any
// CHECK-NEXT:  %5 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:       StoreFrameInst %3: environment, %5: undefined|object, [?anon_1_new.target]: undefined|object
// CHECK-NEXT:       StoreFrameInst %3: environment, %0: object, [?anon_2_arguments]: object
// CHECK-NEXT:       StoreFrameInst %3: environment, undefined: undefined, [arrow3]: any
// CHECK-NEXT:  %9 = LoadFrameInst (:object) %3: environment, [?anon_2_arguments]: object
// CHECK-NEXT:  %10 = LoadPropertyInst (:any) %9: object, 0: number
// CHECK-NEXT:        StorePropertyStrictInst %10: any, globalObject: object, "dummy": string
// CHECK-NEXT:  %12 = CreateFunctionInst (:object) %3: environment, %arrow3(): functionCode
// CHECK-NEXT:        StoreFrameInst %3: environment, %12: object, [arrow3]: any
// CHECK-NEXT:  %14 = LoadFrameInst (:any) %3: environment, [arrow3]: any
// CHECK-NEXT:        ReturnInst %14: any
// CHECK-NEXT:function_end

// CHECK:arrow arrow2(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %arrow1(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %arrow2(): any, %0: environment
// CHECK-NEXT:  %2 = ResolveScopeInst (:environment) %foo(): any, %1: environment
// CHECK-NEXT:  %3 = LoadFrameInst (:object) %2: environment, [?anon_2_arguments@foo]: object
// CHECK-NEXT:  %4 = LoadPropertyInst (:any) %3: object, 2: number
// CHECK-NEXT:       StorePropertyStrictInst %4: any, globalObject: object, "dummy": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:arrow arrow3(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %inner(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %arrow3(): any, %0: environment
// CHECK-NEXT:  %2 = ResolveScopeInst (:environment) %inner(): any, %1: environment
// CHECK-NEXT:  %3 = LoadFrameInst (:object) %2: environment, [?anon_2_arguments@inner]: object
// CHECK-NEXT:  %4 = LoadPropertyInst (:any) %3: object, 1: number
// CHECK-NEXT:       StorePropertyStrictInst %4: any, globalObject: object, "dummy": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
