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
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "ctor_this_test": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) empty: any, empty: any, %ctor_this_test(): functionCode
// CHECK-NEXT:       StorePropertyStrictInst %1: object, globalObject: object, "ctor_this_test": string
// CHECK-NEXT:       ReturnInst "use strict": string
// CHECK-NEXT:function_end

// CHECK:function ctor_this_test(): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) empty: any, empty: any, %use_this(): functionCode
// CHECK-NEXT:  %1 = CreateThisInst (:object) %0: object, empty: any
// CHECK-NEXT:  %2 = CallInst (:object) %0: object, %use_this(): functionCode, true: boolean, empty: any, undefined: undefined, %1: object, 12: number
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:function use_this(k: any): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %k: any
// CHECK-NEXT:       StorePropertyStrictInst %1: any, %0: any, "k": string
// CHECK-NEXT:  %3 = CreateRegExpInst (:object) "regexp": string, "": string
// CHECK-NEXT:       ReturnInst %3: object
// CHECK-NEXT:function_end
