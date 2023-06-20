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
        return this;
    }

    return new use_this(12)
}

function ctor_load_store_test() {
  function use_this(k) {
    this.k = k
  }

  function construct_use_this(){
    return new use_this(12)
  }

  return construct_use_this();
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): string
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "ctor_this_test": string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "ctor_load_store_test": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %ctor_this_test(): object
// CHECK-NEXT:  %3 = StorePropertyStrictInst %2: object, globalObject: object, "ctor_this_test": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %ctor_load_store_test(): object
// CHECK-NEXT:  %5 = StorePropertyStrictInst %4: object, globalObject: object, "ctor_load_store_test": string
// CHECK-NEXT:  %6 = ReturnInst "use strict": string
// CHECK-NEXT:function_end

// CHECK:function ctor_this_test(): object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %use_this(): object
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) %0: object, "prototype": string
// CHECK-NEXT:  %2 = CreateThisInst (:object) %1: any, %0: object
// CHECK-NEXT:  %3 = CallInst (:object) %0: object, %use_this(): object, empty: any, %0: object, %2: object, 12: number
// CHECK-NEXT:  %4 = ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:function ctor_load_store_test(): object
// CHECK-NEXT:frame = [use_this: object]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %"use_this 1#"(): undefined
// CHECK-NEXT:  %1 = StoreFrameInst %0: object, [use_this]: object
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %construct_use_this(): object
// CHECK-NEXT:  %3 = CallInst (:object) %2: object, %construct_use_this(): object, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %4 = ReturnInst %3: object
// CHECK-NEXT:function_end

// CHECK:function use_this(k: number): object [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %this: object
// CHECK-NEXT:  %1 = StorePropertyStrictInst 12: number, %0: object, "k": string
// CHECK-NEXT:  %2 = ReturnInst %0: object
// CHECK-NEXT:function_end

// CHECK:function "use_this 1#"(k: number): undefined [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %this: object
// CHECK-NEXT:  %1 = StorePropertyStrictInst 12: number, %0: object, "k": string
// CHECK-NEXT:  %2 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function construct_use_this(): object [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst (:object) [use_this@ctor_load_store_test]: object
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) %0: object, "prototype": string
// CHECK-NEXT:  %2 = CreateThisInst (:object) %1: any, %0: object
// CHECK-NEXT:  %3 = CallInst (:undefined) %0: object, %"use_this 1#"(): undefined, empty: any, %0: object, %2: object, 12: number
// CHECK-NEXT:  %4 = ReturnInst %2: object
// CHECK-NEXT:function_end
