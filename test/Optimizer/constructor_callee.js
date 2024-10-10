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

// CHECK:scope %VS0 []

// CHECK:function global(): string
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "ctor_this_test": string
// CHECK-NEXT:       DeclareGlobalVarInst "ctor_load_store_test": string
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %0: environment, %ctor_this_test(): functionCode
// CHECK-NEXT:       StorePropertyStrictInst %3: object, globalObject: object, "ctor_this_test": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %0: environment, %ctor_load_store_test(): functionCode
// CHECK-NEXT:       StorePropertyStrictInst %5: object, globalObject: object, "ctor_load_store_test": string
// CHECK-NEXT:       ReturnInst "use strict": string
// CHECK-NEXT:function_end

// CHECK:function ctor_this_test(): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %0: environment, %use_this(): functionCode
// CHECK-NEXT:  %2 = CreateThisInst (:undefined|object) %1: object, empty: any
// CHECK-NEXT:  %3 = CallInst (:undefined|object) %1: object, %use_this(): functionCode, true: boolean, empty: any, undefined: undefined, %2: undefined|object, 12: number
// CHECK-NEXT:  %4 = GetConstructedObjectInst (:object) %2: undefined|object, %2: undefined|object
// CHECK-NEXT:       ReturnInst %4: object
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [use_this: object]

// CHECK:function ctor_load_store_test(): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %1: environment, %"use_this 1#"(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: object, [%VS1.use_this]: object
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %1: environment, %construct_use_this(): functionCode
// CHECK-NEXT:  %5 = CallInst (:object) %4: object, %construct_use_this(): functionCode, true: boolean, %1: environment, undefined: undefined, 0: number
// CHECK-NEXT:       ReturnInst %5: object
// CHECK-NEXT:function_end

// CHECK:function use_this(k: number): undefined|object [allCallsitesKnownInStrictMode]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:undefined|object) %<this>: undefined|object
// CHECK-NEXT:       StorePropertyStrictInst 12: number, %0: undefined|object, "k": string
// CHECK-NEXT:       ReturnInst %0: undefined|object
// CHECK-NEXT:function_end

// CHECK:function "use_this 1#"(k: number): undefined [allCallsitesKnownInStrictMode]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:undefined|object) %<this>: undefined|object
// CHECK-NEXT:       StorePropertyStrictInst 12: number, %0: undefined|object, "k": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function construct_use_this(): object [allCallsitesKnownInStrictMode]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:object) %0: environment, [%VS1.use_this]: object
// CHECK-NEXT:  %2 = CreateThisInst (:undefined|object) %1: object, empty: any
// CHECK-NEXT:  %3 = CallInst (:undefined) %1: object, %"use_this 1#"(): functionCode, true: boolean, empty: any, undefined: undefined, %2: undefined|object, 12: number
// CHECK-NEXT:       ReturnInst %2: undefined|object
// CHECK-NEXT:function_end
