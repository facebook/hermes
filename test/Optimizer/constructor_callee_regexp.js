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
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "ctor_this_test": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %ctor_this_test(): functionCode
// CHECK-NEXT:       StorePropertyStrictInst %2: object, globalObject: object, "ctor_this_test": string
// CHECK-NEXT:       ReturnInst "use strict": string
// CHECK-NEXT:function_end

// CHECK:function ctor_this_test(): object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %0: environment, %use_this(): functionCode
// CHECK-NEXT:  %2 = LoadPropertyInst (:any) %1: object, "prototype": string
// CHECK-NEXT:  %3 = CreateThisInst (:object) %2: any, %1: object
// CHECK-NEXT:  %4 = CallInst (:object) %1: object, %use_this(): functionCode, %0: environment, undefined: undefined, %3: object, 12: number
// CHECK-NEXT:       ReturnInst %4: object
// CHECK-NEXT:function_end

// CHECK:function use_this(k: number): object [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:       StorePropertyStrictInst 12: number, %0: object, "k": string
// CHECK-NEXT:  %2 = CreateRegExpInst (:object) "regexp": string, "": string
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end
