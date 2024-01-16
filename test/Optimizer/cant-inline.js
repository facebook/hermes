/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

// Make sure that we are not inlining if copyRestArgs() is used.

function outer1() {
    return (function dontInline(...rest) {
        return rest;
    })(1);
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "outer1": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %outer1(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "outer1": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function outer1(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %dontInline(): functionCode
// CHECK-NEXT:  %1 = CallInst (:any) %0: object, %dontInline(): functionCode, empty: any, undefined: undefined, undefined: undefined, 1: number
// CHECK-NEXT:       ReturnInst %1: any
// CHECK-NEXT:function_end

// CHECK:function dontInline(): any [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CallBuiltinInst (:any) [HermesBuiltin.copyRestArgs]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:       ReturnInst %0: any
// CHECK-NEXT:function_end
