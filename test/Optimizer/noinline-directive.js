/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

'use strict';

function outer(a) {
    // A function simple enough to be naturally inlined.
    function foo(x) {
        return x + 5;
    }

    // The same function, but with the noinline directive.
    function fooNoInline(x) {
        'noinline'
        return x + 5;
    }

    // If there are multiple directive, last one controls.

    // Like "foo", but with two directives, 'inline' last.
    function fooInlineLast(x) {
        'noinline'
        'inline'
        return x + 50;
    }

    // Like "fooNoInline", but with two directives, 'noinline' last.
    function fooNoInlineLast(x) {
        'inline'
        'noinline'
        return x + 50;
    }

    // We should inline the first, but not the second, call.
    return foo(a) + fooNoInline(a) + fooInlineLast(a) + fooNoInlineLast(a);
};

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): string
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "outer": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %outer(): functionCode
// CHECK-NEXT:       StorePropertyStrictInst %2: object, globalObject: object, "outer": string
// CHECK-NEXT:       ReturnInst "use strict": string
// CHECK-NEXT:function_end

// CHECK:function outer(a: any): string|number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %fooNoInline(): functionCode
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %0: environment, %fooNoInlineLast(): functionCode
// CHECK-NEXT:  %4 = BinaryAddInst (:string|number) %1: any, 5: number
// CHECK-NEXT:  %5 = CallInst (:string|number) %2: object, %fooNoInline(): functionCode, false: boolean, empty: any, undefined: undefined, 0: number, %1: any
// CHECK-NEXT:  %6 = BinaryAddInst (:string|number) %4: string|number, %5: string|number
// CHECK-NEXT:  %7 = BinaryAddInst (:string|number) %1: any, 50: number
// CHECK-NEXT:  %8 = BinaryAddInst (:string|number) %6: string|number, %7: string|number
// CHECK-NEXT:  %9 = CallInst (:string|number) %3: object, %fooNoInlineLast(): functionCode, false: boolean, empty: any, undefined: undefined, 0: number, %1: any
// CHECK-NEXT:  %10 = BinaryAddInst (:string|number) %8: string|number, %9: string|number
// CHECK-NEXT:        ReturnInst %10: string|number
// CHECK-NEXT:function_end

// CHECK:function fooNoInline(x: any): string|number [allCallsitesKnownInStrictMode]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = BinaryAddInst (:string|number) %0: any, 5: number
// CHECK-NEXT:       ReturnInst %1: string|number
// CHECK-NEXT:function_end

// CHECK:function fooNoInlineLast(x: any): string|number [allCallsitesKnownInStrictMode]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = BinaryAddInst (:string|number) %0: any, 50: number
// CHECK-NEXT:       ReturnInst %1: string|number
// CHECK-NEXT:function_end
