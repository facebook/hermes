/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -dump-ir -fno-inline -Xenable-tdz %s | %FileCheckOrRegen --match-full-lines %s

// Make sure that stores to frame variables are not eliminated if throwing an
// exception could make it observable.
(function(){
  var x;
  function throwTDZ(){
    x = 0;
    // This access can throw, so the x=0 above cannot be eliminated.
    tdz;
    x = 1;
  }

  try { throwTDZ() } catch(e) {}
  print(x);
  let tdz;
})();

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %0: environment, %""(): functionCode
// CHECK-NEXT:  %2 = CallInst (:undefined) %1: object, %""(): functionCode, %0: environment, undefined: undefined, undefined: undefined
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function ""(): undefined [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = [x: undefined|number, tdz: empty|undefined]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %""(): any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [x]: undefined|number
// CHECK-NEXT:       StoreFrameInst %1: environment, empty: empty, [tdz]: empty|undefined
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %1: environment, %throwTDZ(): functionCode
// CHECK-NEXT:       TryStartInst %BB1, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = CatchInst (:any)
// CHECK-NEXT:       BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %8 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %9 = LoadFrameInst (:undefined|number) %1: environment, [x]: undefined|number
// CHECK-NEXT:  %10 = CallInst (:any) %8: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %9: undefined|number
// CHECK-NEXT:        StoreFrameInst %1: environment, undefined: undefined, [tdz]: empty|undefined
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %13 = CallInst (:undefined) %4: object, %throwTDZ(): functionCode, %1: environment, undefined: undefined, undefined: undefined
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:function_end

// CHECK:function throwTDZ(): undefined [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %""(): any, %parentScope: environment
// CHECK-NEXT:       StoreFrameInst %0: environment, 0: number, [x@""]: undefined|number
// CHECK-NEXT:  %2 = LoadFrameInst (:empty|undefined) %0: environment, [tdz@""]: empty|undefined
// CHECK-NEXT:  %3 = ThrowIfInst (:undefined) %2: empty|undefined, type(empty)
// CHECK-NEXT:       StoreFrameInst %0: environment, 1: number, [x@""]: undefined|number
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
