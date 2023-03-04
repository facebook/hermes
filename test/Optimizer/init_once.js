/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir %s -O | %FileCheckOrRegen --match-full-lines %s

function main(p) {
  var k = p;
  var p = print;
  function bar() {
    p(k)
    p(k)
    p(k)
    p(k)
    p(k)
    p(k)
  }

  return bar;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "main": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:closure) %main(): closure
// CHECK-NEXT:  %2 = StorePropertyLooseInst %1: closure, globalObject: object, "main": string
// CHECK-NEXT:  %3 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function main(p: any): closure
// CHECK-NEXT:frame = [p: any, k: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %p: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [p]: any
// CHECK-NEXT:  %2 = CreateFunctionInst (:closure) %bar(): undefined
// CHECK-NEXT:  %3 = StoreFrameInst %0: any, [k]: any
// CHECK-NEXT:  %4 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %5 = StoreFrameInst %4: any, [p]: any
// CHECK-NEXT:  %6 = ReturnInst %2: closure
// CHECK-NEXT:function_end

// CHECK:function bar(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst (:any) [p@main]: any
// CHECK-NEXT:  %1 = LoadFrameInst (:any) [k@main]: any
// CHECK-NEXT:  %2 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, %1: any
// CHECK-NEXT:  %3 = LoadFrameInst (:any) [p@main]: any
// CHECK-NEXT:  %4 = CallInst (:any) %3: any, empty: any, empty: any, undefined: undefined, %1: any
// CHECK-NEXT:  %5 = LoadFrameInst (:any) [p@main]: any
// CHECK-NEXT:  %6 = CallInst (:any) %5: any, empty: any, empty: any, undefined: undefined, %1: any
// CHECK-NEXT:  %7 = LoadFrameInst (:any) [p@main]: any
// CHECK-NEXT:  %8 = CallInst (:any) %7: any, empty: any, empty: any, undefined: undefined, %1: any
// CHECK-NEXT:  %9 = LoadFrameInst (:any) [p@main]: any
// CHECK-NEXT:  %10 = CallInst (:any) %9: any, empty: any, empty: any, undefined: undefined, %1: any
// CHECK-NEXT:  %11 = LoadFrameInst (:any) [p@main]: any
// CHECK-NEXT:  %12 = CallInst (:any) %11: any, empty: any, empty: any, undefined: undefined, %1: any
// CHECK-NEXT:  %13 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end
