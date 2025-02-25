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

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %1 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) empty: any, empty: any, %foo(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %2: object, %1: object, "foo": string
// CHECK-NEXT:  %4 = LoadPropertyInst (:any) %1: object, "foo": string
// CHECK-NEXT:  %5 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) empty: any, empty: any, %""(): functionCode
// CHECK-NEXT:  %7 = CallInst (:any) %4: any, empty: any, false: boolean, empty: any, %5: undefined, %5: undefined, %6: object
// CHECK-NEXT:       ReturnInst %7: any
// CHECK-NEXT:function_end

// CHECK:scope %VS0 [x: undefined|number]

// CHECK:function foo(sink: any): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  %1 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %VS0: any, %bar(): functionCode
// CHECK-NEXT:  %3 = LoadParamInst (:any) %sink: any
// CHECK-NEXT:  %4 = CallInst (:any) %3: any, empty: any, false: boolean, empty: any, %1: undefined, %1: undefined, %2: object
// CHECK-NEXT:  %5 = HBCLoadConstInst (:number) 0: number
// CHECK-NEXT:       StoreFrameInst %0: environment, %5: number, [%VS0.x]: undefined|number
// CHECK-NEXT:       ReturnInst %1: undefined
// CHECK-NEXT:function_end

// CHECK:arrow ""(fn: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  %1 = LoadParamInst (:any) %fn: any
// CHECK-NEXT:  %2 = CallInst (:any) %1: any, empty: any, false: boolean, empty: any, %0: undefined, %0: undefined
// CHECK-NEXT:       ReturnInst %2: any
// CHECK-NEXT:function_end

// CHECK:function bar(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst (:any) %0: object, "print": string
// CHECK-NEXT:  %2 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %3 = LoadFrameInst (:undefined|number) %2: environment, [%VS0.x]: undefined|number
// CHECK-NEXT:  %4 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  %5 = HBCLoadConstInst (:number) 1: number
// CHECK-NEXT:  %6 = BinaryAddInst (:number) %3: undefined|number, %5: number
// CHECK-NEXT:  %7 = CallInst (:any) %1: any, empty: any, false: boolean, empty: any, %4: undefined, %4: undefined, %6: number
// CHECK-NEXT:       ReturnInst %4: undefined
// CHECK-NEXT:function_end
