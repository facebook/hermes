/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O | %FileCheckOrRegen %s --match-full-lines

(function () {
  // res should be number.
  var res = 0;

  function foo() {
    bar(123);
  }

  function bar(o) {
    res += o;
  }

  return foo;
});

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %0: environment, %""(): functionCode
// CHECK-NEXT:       ReturnInst %1: object
// CHECK-NEXT:function_end

// CHECK:function ""(): object
// CHECK-NEXT:frame = [res: string|number, bar: object]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %""(): any, %0: environment
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %1: environment, %foo(): functionCode
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %1: environment, %bar(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %3: object, [bar]: object
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [res]: string|number
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:function foo(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %""(): any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:object) %0: environment, [bar@""]: object
// CHECK-NEXT:  %2 = GetClosureScopeInst (:environment) %""(): any, %1: object
// CHECK-NEXT:  %3 = LoadFrameInst (:string|number) %2: environment, [res@""]: string|number
// CHECK-NEXT:  %4 = BinaryAddInst (:string|number) %3: string|number, 123: number
// CHECK-NEXT:       StoreFrameInst %2: environment, %4: string|number, [res@""]: string|number
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function bar(o: any): undefined [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %""(): any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadParamInst (:any) %o: any
// CHECK-NEXT:  %2 = LoadFrameInst (:string|number) %0: environment, [res@""]: string|number
// CHECK-NEXT:  %3 = BinaryAddInst (:string|number) %2: string|number, %1: any
// CHECK-NEXT:       StoreFrameInst %0: environment, %3: string|number, [res@""]: string|number
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
