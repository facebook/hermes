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
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %main(): object
// CHECK-NEXT:  %2 = StorePropertyLooseInst %1: object, globalObject: object, "main": string
// CHECK-NEXT:  %3 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function main(p: any): object
// CHECK-NEXT:frame = [p: any, k: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %p: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [p]: any
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %bar(): undefined
// CHECK-NEXT:  %3 = StoreFrameInst %0: any, [k]: any
// CHECK-NEXT:  %4 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %5 = StoreFrameInst %4: any, [p]: any
// CHECK-NEXT:  %6 = ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:function bar(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst (:any) [p@main]: any
// CHECK-NEXT:  %1 = LoadFrameInst (:any) [k@main]: any
// CHECK-NEXT:  %2 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %1: any
// CHECK-NEXT:  %3 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %1: any
// CHECK-NEXT:  %4 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %1: any
// CHECK-NEXT:  %5 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %1: any
// CHECK-NEXT:  %6 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %1: any
// CHECK-NEXT:  %7 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %1: any
// CHECK-NEXT:  %8 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end
