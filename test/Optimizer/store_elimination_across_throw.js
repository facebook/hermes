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

// CHECK:scope %VS0 []

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %0: environment, %VS0: any, %""(): functionCode
// CHECK-NEXT:  %2 = CallInst (:undefined) %1: object, %""(): functionCode, true: boolean, %0: environment, undefined: undefined, undefined: undefined
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [x: undefined|number, tdz: empty|undefined]

// CHECK:function ""(): undefined [allCallsitesKnownInStrictMode]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS1: any, empty: any
// CHECK-NEXT:       StoreFrameInst %0: environment, undefined: undefined, [%VS1.x]: undefined|number
// CHECK-NEXT:       StoreFrameInst %0: environment, empty: empty, [%VS1.tdz]: empty|undefined
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %0: environment, %VS1: any, %throwTDZ(): functionCode
// CHECK-NEXT:       TryStartInst %BB1, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = CatchInst (:any)
// CHECK-NEXT:       BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %8 = LoadFrameInst (:undefined|number) %0: environment, [%VS1.x]: undefined|number
// CHECK-NEXT:  %9 = CallInst (:any) %7: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %8: undefined|number
// CHECK-NEXT:        StoreFrameInst %0: environment, undefined: undefined, [%VS1.tdz]: empty|undefined
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %12 = CallInst (:undefined) %3: object, %throwTDZ(): functionCode, true: boolean, %0: environment, undefined: undefined, undefined: undefined
// CHECK-NEXT:        TryEndInst %BB1, %BB2
// CHECK-NEXT:function_end

// CHECK:function throwTDZ(): undefined [allCallsitesKnownInStrictMode]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:       StoreFrameInst %0: environment, 0: number, [%VS1.x]: undefined|number
// CHECK-NEXT:  %2 = LoadFrameInst (:empty|undefined) %0: environment, [%VS1.tdz]: empty|undefined
// CHECK-NEXT:  %3 = ThrowIfInst (:undefined) %2: empty|undefined, type(empty)
// CHECK-NEXT:       StoreFrameInst %0: environment, 1: number, [%VS1.x]: undefined|number
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
