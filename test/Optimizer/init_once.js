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

// CHECK:function global() : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "main" : string
// CHECK-NEXT:  %1 = CreateFunctionInst %main() : closure
// CHECK-NEXT:  %2 = StorePropertyLooseInst %1 : closure, globalObject : object, "main" : string
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function main(p) : closure
// CHECK-NEXT:frame = [p, k]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %p
// CHECK-NEXT:  %1 = StoreFrameInst %0, [p]
// CHECK-NEXT:  %2 = CreateFunctionInst %bar() : undefined
// CHECK-NEXT:  %3 = StoreFrameInst %0, [k]
// CHECK-NEXT:  %4 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %5 = StoreFrameInst %4, [p]
// CHECK-NEXT:  %6 = ReturnInst %2 : closure
// CHECK-NEXT:function_end

// CHECK:function bar() : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst [p@main]
// CHECK-NEXT:  %1 = LoadFrameInst [k@main]
// CHECK-NEXT:  %2 = CallInst %0, empty, empty, undefined : undefined, %1
// CHECK-NEXT:  %3 = LoadFrameInst [p@main]
// CHECK-NEXT:  %4 = CallInst %3, empty, empty, undefined : undefined, %1
// CHECK-NEXT:  %5 = LoadFrameInst [p@main]
// CHECK-NEXT:  %6 = CallInst %5, empty, empty, undefined : undefined, %1
// CHECK-NEXT:  %7 = LoadFrameInst [p@main]
// CHECK-NEXT:  %8 = CallInst %7, empty, empty, undefined : undefined, %1
// CHECK-NEXT:  %9 = LoadFrameInst [p@main]
// CHECK-NEXT:  %10 = CallInst %9, empty, empty, undefined : undefined, %1
// CHECK-NEXT:  %11 = LoadFrameInst [p@main]
// CHECK-NEXT:  %12 = CallInst %11, empty, empty, undefined : undefined, %1
// CHECK-NEXT:  %13 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
