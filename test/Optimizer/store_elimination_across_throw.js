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
// CHECK-NEXT:  %0 = AllocStackInst (:object) $throwTDZ: any
// CHECK-NEXT:  %1 = StoreFrameInst undefined: undefined, [x]: undefined|number
// CHECK-NEXT:  %2 = StoreFrameInst empty: empty, [tdz]: empty|undefined
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %throwTDZ(): undefined
// CHECK-NEXT:  %4 = StoreStackInst %3: object, %0: object
// CHECK-NEXT:  %5 = TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = CatchInst (:any)
// CHECK-NEXT:  %7 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %9 = LoadFrameInst (:undefined|number) [x]: undefined|number
// CHECK-NEXT:  %10 = CallInst (:any) %8: any, empty: any, empty: any, undefined: undefined, %9: undefined|number
// CHECK-NEXT:  %11 = StoreFrameInst undefined: undefined, [tdz]: empty|undefined
// CHECK-NEXT:  %12 = ReturnInst undefined: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %13 = LoadStackInst (:object) %0: object
// CHECK-NEXT:  %14 = CallInst (:any) %13: object, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %15 = BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %16 = TryEndInst
// CHECK-NEXT:  %17 = BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function throwTDZ(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst 0: number, [x@""]: undefined|number
// CHECK-NEXT:  %1 = LoadFrameInst (:empty|undefined) [tdz@""]: empty|undefined
// CHECK-NEXT:  %2 = ThrowIfEmptyInst (:undefined) %1: empty|undefined
// CHECK-NEXT:  %3 = StoreFrameInst 1: number, [x@""]: undefined|number
// CHECK-NEXT:  %4 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end
