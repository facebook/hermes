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
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "ctor_this_test": string
// CHECK-NEXT:       DeclareGlobalVarInst "ctor_load_store_test": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) empty: any, empty: any, %ctor_this_test(): functionCode
// CHECK-NEXT:       StorePropertyStrictInst %2: object, globalObject: object, "ctor_this_test": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) empty: any, empty: any, %ctor_load_store_test(): functionCode
// CHECK-NEXT:       StorePropertyStrictInst %4: object, globalObject: object, "ctor_load_store_test": string
// CHECK-NEXT:       ReturnInst "use strict": string
// CHECK-NEXT:function_end

// CHECK:function ctor_this_test(): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) empty: any, empty: any, %use_this(): functionCode
// CHECK-NEXT:  %1 = CreateThisInst (:object) %0: object, %0: object
// CHECK-NEXT:  %2 = CallInst (:any) %0: object, %use_this(): functionCode, true: boolean, empty: any, undefined: undefined, %1: object, 12: number
// CHECK-NEXT:       ReturnInst %1: object
// CHECK-NEXT:function_end

// CHECK:scope %VS0 [use_this: object]

// CHECK:function ctor_load_store_test(): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) empty: any, empty: any, %"use_this 1#"(): functionCode
// CHECK-NEXT:       StoreFrameInst %0: environment, %1: object, [%VS0.use_this]: object
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %0: environment, %VS0: any, %construct_use_this(): functionCode
// CHECK-NEXT:  %4 = CallInst (:object) %3: object, %construct_use_this(): functionCode, true: boolean, %0: environment, undefined: undefined, 0: number
// CHECK-NEXT:       ReturnInst %4: object
// CHECK-NEXT:function_end

// CHECK:function use_this(k: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %k: any
// CHECK-NEXT:       StorePropertyStrictInst %1: any, %0: any, "k": string
// CHECK-NEXT:       ReturnInst %0: any
// CHECK-NEXT:function_end

// CHECK:function "use_this 1#"(k: any): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %k: any
// CHECK-NEXT:       StorePropertyStrictInst %1: any, %0: any, "k": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function construct_use_this(): object [allCallsitesKnownInStrictMode]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:object) %0: environment, [%VS0.use_this]: object
// CHECK-NEXT:  %2 = CreateThisInst (:object) %1: object, %1: object
// CHECK-NEXT:  %3 = CallInst (:undefined) %1: object, %"use_this 1#"(): functionCode, true: boolean, empty: any, undefined: undefined, %2: object, 12: number
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end
