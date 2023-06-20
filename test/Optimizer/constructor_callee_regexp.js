/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir %s -fno-inline | %FileCheckOrRegen --match-full-lines %s

"use strict";

function ctor_this_test() {
    function use_this(k) {
        this.k = k
        return /regexp/;
    }

    return new use_this(12)
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): string
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "ctor_this_test": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %ctor_this_test(): object
// CHECK-NEXT:  %2 = StorePropertyStrictInst %1: object, globalObject: object, "ctor_this_test": string
// CHECK-NEXT:  %3 = ReturnInst "use strict": string
// CHECK-NEXT:function_end

// CHECK:function ctor_this_test(): object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %use_this(): object
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) %0: object, "prototype": string
// CHECK-NEXT:  %2 = CreateThisInst (:object) %1: any, %0: object
// CHECK-NEXT:  %3 = CallInst (:object) %0: object, %use_this(): object, empty: any, %0: object, %2: object, 12: number
// CHECK-NEXT:  %4 = ReturnInst %3: object
// CHECK-NEXT:function_end

// CHECK:function use_this(k: number): object [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %this: object
// CHECK-NEXT:  %1 = StorePropertyStrictInst 12: number, %0: object, "k": string
// CHECK-NEXT:  %2 = CreateRegExpInst (:object) "regexp": string, "": string
// CHECK-NEXT:  %3 = ReturnInst %2: object
// CHECK-NEXT:function_end
