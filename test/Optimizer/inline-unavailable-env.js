/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

// Test our ability to inline functions where their enclosing environment is not
// available at the call site.

function foo(sink){
  let x;
  let numBar = 0;
  function bar() {
    let y = ++numBar;
    // Assign x in bar, so the parent environment of the function is unavailable
    // when  we call x, and we are forced to get the environment from the
    // closure.
    x = function () { "inline"; return ++y; };
  }
  sink(bar);
  return x();
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %VS0: any, %foo(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "foo": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [x: undefined|object, numBar: number]

// CHECK:scope %VS2 [y: number]

// CHECK:function foo(sink: any): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %sink: any
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %1: environment, %VS1: any, %bar(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.x]: undefined|object
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [%VS1.numBar]: number
// CHECK-NEXT:  %6 = CallInst (:any) %2: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %3: object
// CHECK-NEXT:  %7 = LoadFrameInst (:undefined|object) %1: environment, [%VS1.x]: undefined|object
// CHECK-NEXT:  %8 = TypeOfIsInst (:boolean) %7: undefined|object, typeOfIs(Function)
// CHECK-NEXT:       CondBranchInst %8: boolean, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:        ThrowTypeErrorInst "Trying to call a non-function": string
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %11 = GetClosureScopeInst (:environment) %VS2: any, %x(): functionCode, %7: undefined|object
// CHECK-NEXT:  %12 = LoadFrameInst (:number) %11: environment, [%VS2.y]: number
// CHECK-NEXT:  %13 = FAddInst (:number) %12: number, 1: number
// CHECK-NEXT:        StoreFrameInst %11: environment, %13: number, [%VS2.y]: number
// CHECK-NEXT:        ReturnInst %13: number
// CHECK-NEXT:function_end

// CHECK:function bar(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS2: any, %0: environment
// CHECK-NEXT:  %2 = LoadFrameInst (:number) %0: environment, [%VS1.numBar]: number
// CHECK-NEXT:  %3 = FAddInst (:number) %2: number, 1: number
// CHECK-NEXT:       StoreFrameInst %0: environment, %3: number, [%VS1.numBar]: number
// CHECK-NEXT:       StoreFrameInst %1: environment, %3: number, [%VS2.y]: number
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %1: environment, %VS2: any, %x(): functionCode
// CHECK-NEXT:       StoreFrameInst %0: environment, %6: object, [%VS1.x]: undefined|object
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function x(): number [allCallsitesKnownInStrictMode,unreachable]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       UnreachableInst
// CHECK-NEXT:function_end
