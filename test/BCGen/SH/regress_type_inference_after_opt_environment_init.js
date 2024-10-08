/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -dump-lir %s | %FileCheckOrRegen %s --match-full-lines

// We previously incorrectly ran TypeInference after OptEnvironmentInit had
// deleted stores of undefined to the environment. This caused us to infer a
// narrower type for variables than they actually had, resulting in incorrect
// codegen.
// This test checks that we do not infer types after OptEnvironmentInit has run.

function foo(sink) {
  // Hoisted store of undefined to x is observable.
  function bar() {
    // x here must have type number|undefined
    print(x+1);
  }
  sink(bar);
  var x = 0;
}
foo((fn) => fn());

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %foo(): functionCode
// CHECK-NEXT:  %3 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:       StorePropertyLooseInst %2: object, %3: object, "foo": string
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) %3: object, "foo": string
// CHECK-NEXT:  %6 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %0: environment, %""(): functionCode
// CHECK-NEXT:  %8 = CallInst (:any) %5: any, empty: any, false: boolean, empty: any, %6: undefined, %6: undefined, %7: object
// CHECK-NEXT:       ReturnInst %8: any
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [x: undefined|number]

// CHECK:function foo(sink: any): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %sink: any
// CHECK-NEXT:  %3 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %1: environment, %bar(): functionCode
// CHECK-NEXT:  %5 = CallInst (:any) %2: any, empty: any, false: boolean, empty: any, %3: undefined, %3: undefined, %4: object
// CHECK-NEXT:  %6 = HBCLoadConstInst (:number) 0: number
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: number, [%VS1.x]: undefined|number
// CHECK-NEXT:       ReturnInst %3: undefined
// CHECK-NEXT:function_end

// CHECK:arrow ""(fn: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %fn: any
// CHECK-NEXT:  %1 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  %2 = CallInst (:any) %0: any, empty: any, false: boolean, empty: any, %1: undefined, %1: undefined
// CHECK-NEXT:       ReturnInst %2: any
// CHECK-NEXT:function_end

// CHECK:function bar(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst (:any) %0: object, "print": string
// CHECK-NEXT:  %2 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %3 = LoadFrameInst (:undefined|number) %2: environment, [%VS1.x]: undefined|number
// CHECK-NEXT:  %4 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  %5 = HBCLoadConstInst (:number) 1: number
// CHECK-NEXT:  %6 = BinaryAddInst (:number) %3: undefined|number, %5: number
// CHECK-NEXT:  %7 = CallInst (:any) %1: any, empty: any, false: boolean, empty: any, %4: undefined, %4: undefined, %6: number
// CHECK-NEXT:       ReturnInst %4: undefined
// CHECK-NEXT:function_end
