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

// CHECK:function global(): string
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "outer": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) empty: any, empty: any, %outer(): functionCode
// CHECK-NEXT:       StorePropertyStrictInst %1: object, globalObject: object, "outer": string
// CHECK-NEXT:       ReturnInst "use strict": string
// CHECK-NEXT:function_end

// CHECK:function outer(a: any): string|number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) empty: any, empty: any, %fooNoInline(): functionCode
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) empty: any, empty: any, %fooNoInlineLast(): functionCode
// CHECK-NEXT:  %3 = BinaryAddInst (:string|number) %0: any, 5: number
// CHECK-NEXT:  %4 = CallInst (:string|number) %1: object, %fooNoInline(): functionCode, true: boolean, empty: any, undefined: undefined, 0: number, %0: any
// CHECK-NEXT:  %5 = BinaryAddInst (:string|number) %3: string|number, %4: string|number
// CHECK-NEXT:  %6 = BinaryAddInst (:string|number) %0: any, 50: number
// CHECK-NEXT:  %7 = BinaryAddInst (:string|number) %5: string|number, %6: string|number
// CHECK-NEXT:  %8 = CallInst (:string|number) %2: object, %fooNoInlineLast(): functionCode, true: boolean, empty: any, undefined: undefined, 0: number, %0: any
// CHECK-NEXT:  %9 = BinaryAddInst (:string|number) %7: string|number, %8: string|number
// CHECK-NEXT:        ReturnInst %9: string|number
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
