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

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
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

// CHECK:scope %VS1 []

// CHECK:function normal(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsStrictInst (:object)
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS1: any, %1: environment
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) %0: object, 0: number
// CHECK-NEXT:       StorePropertyStrictInst %3: any, globalObject: object, "dummy": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS2 [?anon_0_this: any, ?anon_1_new.target: undefined|object, ?anon_2_arguments: object, arrow1: any]

// CHECK:function foo(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsStrictInst (:object)
// CHECK-NEXT:  %1 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %2 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %3 = CreateScopeInst (:environment) %VS2: any, %2: environment
// CHECK-NEXT:       StoreFrameInst %3: environment, %1: any, [%VS2.?anon_0_this]: any
// CHECK-NEXT:  %5 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:       StoreFrameInst %3: environment, %5: undefined|object, [%VS2.?anon_1_new.target]: undefined|object
// CHECK-NEXT:       StoreFrameInst %3: environment, %0: object, [%VS2.?anon_2_arguments]: object
// CHECK-NEXT:       StoreFrameInst %3: environment, undefined: undefined, [%VS2.arrow1]: any
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %3: environment, %arrow1(): functionCode
// CHECK-NEXT:        StoreFrameInst %3: environment, %9: object, [%VS2.arrow1]: any
// CHECK-NEXT:  %11 = LoadFrameInst (:object) %3: environment, [%VS2.?anon_2_arguments]: object
// CHECK-NEXT:  %12 = LoadPropertyInst (:any) %11: object, 0: number
// CHECK-NEXT:        StorePropertyStrictInst %12: any, globalObject: object, "dummy": string
// CHECK-NEXT:  %14 = LoadFrameInst (:any) %3: environment, [%VS2.arrow1]: any
// CHECK-NEXT:        ReturnInst %14: any
// CHECK-NEXT:function_end

// CHECK:scope %VS3 [inner: any]

// CHECK:function bar(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsStrictInst (:object)
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS3: any, %1: environment
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %2: environment, %inner(): functionCode
// CHECK-NEXT:       StoreFrameInst %2: environment, %3: object, [%VS3.inner]: any
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) %0: object, 0: number
// CHECK-NEXT:       StorePropertyStrictInst %5: any, globalObject: object, "dummy": string
// CHECK-NEXT:  %7 = LoadFrameInst (:any) %2: environment, [%VS3.inner]: any
// CHECK-NEXT:       ReturnInst %7: any
// CHECK-NEXT:function_end

// CHECK:scope %VS4 [arrow2: any]

// CHECK:arrow arrow1(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS2: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS4: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS4.arrow2]: any
// CHECK-NEXT:  %3 = LoadFrameInst (:object) %0: environment, [%VS2.?anon_2_arguments]: object
// CHECK-NEXT:  %4 = LoadPropertyInst (:any) %3: object, 1: number
// CHECK-NEXT:       StorePropertyStrictInst %4: any, globalObject: object, "dummy": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %1: environment, %arrow2(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: object, [%VS4.arrow2]: any
// CHECK-NEXT:  %8 = LoadFrameInst (:any) %1: environment, [%VS4.arrow2]: any
// CHECK-NEXT:       ReturnInst %8: any
// CHECK-NEXT:function_end

// CHECK:scope %VS5 [?anon_0_this: any, ?anon_1_new.target: undefined|object, ?anon_2_arguments: object, arrow3: any]

// CHECK:function inner(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsStrictInst (:object)
// CHECK-NEXT:  %1 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %2 = GetParentScopeInst (:environment) %VS3: any, %parentScope: environment
// CHECK-NEXT:  %3 = CreateScopeInst (:environment) %VS5: any, %2: environment
// CHECK-NEXT:       StoreFrameInst %3: environment, %1: any, [%VS5.?anon_0_this]: any
// CHECK-NEXT:  %5 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:       StoreFrameInst %3: environment, %5: undefined|object, [%VS5.?anon_1_new.target]: undefined|object
// CHECK-NEXT:       StoreFrameInst %3: environment, %0: object, [%VS5.?anon_2_arguments]: object
// CHECK-NEXT:       StoreFrameInst %3: environment, undefined: undefined, [%VS5.arrow3]: any
// CHECK-NEXT:  %9 = LoadFrameInst (:object) %3: environment, [%VS5.?anon_2_arguments]: object
// CHECK-NEXT:  %10 = LoadPropertyInst (:any) %9: object, 0: number
// CHECK-NEXT:        StorePropertyStrictInst %10: any, globalObject: object, "dummy": string
// CHECK-NEXT:  %12 = CreateFunctionInst (:object) %3: environment, %arrow3(): functionCode
// CHECK-NEXT:        StoreFrameInst %3: environment, %12: object, [%VS5.arrow3]: any
// CHECK-NEXT:  %14 = LoadFrameInst (:any) %3: environment, [%VS5.arrow3]: any
// CHECK-NEXT:        ReturnInst %14: any
// CHECK-NEXT:function_end

// CHECK:scope %VS6 []

// CHECK:arrow arrow2(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS4: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS6: any, %0: environment
// CHECK-NEXT:  %2 = ResolveScopeInst (:environment) %VS2: any, %VS4: any, %0: environment
// CHECK-NEXT:  %3 = LoadFrameInst (:object) %2: environment, [%VS2.?anon_2_arguments]: object
// CHECK-NEXT:  %4 = LoadPropertyInst (:any) %3: object, 2: number
// CHECK-NEXT:       StorePropertyStrictInst %4: any, globalObject: object, "dummy": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS7 []

// CHECK:arrow arrow3(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS5: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS7: any, %0: environment
// CHECK-NEXT:  %2 = LoadFrameInst (:object) %0: environment, [%VS5.?anon_2_arguments]: object
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) %2: object, 1: number
// CHECK-NEXT:       StorePropertyStrictInst %3: any, globalObject: object, "dummy": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
