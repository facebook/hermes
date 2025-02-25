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
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS1: any, empty: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %sink: any
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %VS1: any, %bar(): functionCode
// CHECK-NEXT:       StoreFrameInst %0: environment, undefined: undefined, [%VS1.x]: undefined|object
// CHECK-NEXT:       StoreFrameInst %0: environment, 0: number, [%VS1.numBar]: number
// CHECK-NEXT:  %5 = CallInst (:any) %1: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %2: object
// CHECK-NEXT:  %6 = LoadFrameInst (:undefined|object) %0: environment, [%VS1.x]: undefined|object
// CHECK-NEXT:  %7 = TypeOfIsInst (:boolean) %6: undefined|object, typeOfIs(Function)
// CHECK-NEXT:       CondBranchInst %7: boolean, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       ThrowTypeErrorInst "Trying to call a non-function": string
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %10 = GetClosureScopeInst (:environment) %VS2: any, %x(): functionCode, %6: undefined|object
// CHECK-NEXT:  %11 = LoadFrameInst (:number) %10: environment, [%VS2.y]: number
// CHECK-NEXT:  %12 = FAddInst (:number) %11: number, 1: number
// CHECK-NEXT:        StoreFrameInst %10: environment, %12: number, [%VS2.y]: number
// CHECK-NEXT:        ReturnInst %12: number
// CHECK-NEXT:function_end

// CHECK:function bar(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS2: any, empty: any
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
