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
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %""(): undefined
// CHECK-NEXT:  %1 = CallInst (:undefined) %0: object, %""(): undefined, empty: any, undefined: undefined
// CHECK-NEXT:  %2 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function ""(): undefined [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = [x: undefined|number, tdz: empty|undefined]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined: undefined, [x]: undefined|number
// CHECK-NEXT:  %1 = StoreFrameInst empty: empty, [tdz]: empty|undefined
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %throwTDZ(): undefined
// CHECK-NEXT:  %3 = TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = CatchInst (:any)
// CHECK-NEXT:  %5 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %6 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %7 = LoadFrameInst (:undefined|number) [x]: undefined|number
// CHECK-NEXT:  %8 = CallInst (:any) %6: any, empty: any, empty: any, undefined: undefined, %7: undefined|number
// CHECK-NEXT:  %9 = StoreFrameInst undefined: undefined, [tdz]: empty|undefined
// CHECK-NEXT:  %10 = ReturnInst undefined: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %11 = CallInst (:undefined) %2: object, %throwTDZ(): undefined, empty: any, undefined: undefined
// CHECK-NEXT:  %12 = BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %13 = TryEndInst
// CHECK-NEXT:  %14 = BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function throwTDZ(): undefined [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst 0: number, [x@""]: undefined|number
// CHECK-NEXT:  %1 = LoadFrameInst (:empty|undefined) [tdz@""]: empty|undefined
// CHECK-NEXT:  %2 = ThrowIfEmptyInst (:undefined) %1: empty|undefined
// CHECK-NEXT:  %3 = StoreFrameInst 1: number, [x@""]: undefined|number
// CHECK-NEXT:  %4 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end
