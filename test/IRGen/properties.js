/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

var y = 2;
y.bar = 3;
sink(y)

function sink(x, y) {
  return x.bar
  return x["bar"]
  return x[y]
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = [], globals = [y, sink]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %sink()
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "sink" : string
// CHECK-NEXT:  %2 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %3 = StoreStackInst undefined : undefined, %2
// CHECK-NEXT:  %4 = StorePropertyLooseInst 2 : number, globalObject : object, "y" : string
// CHECK-NEXT:  %5 = LoadPropertyInst globalObject : object, "y" : string
// CHECK-NEXT:  %6 = StorePropertyLooseInst 3 : number, %5, "bar" : string
// CHECK-NEXT:  %7 = StoreStackInst 3 : number, %2
// CHECK-NEXT:  %8 = LoadPropertyInst globalObject : object, "sink" : string
// CHECK-NEXT:  %9 = LoadPropertyInst globalObject : object, "y" : string
// CHECK-NEXT:  %10 = CallInst %8, undefined : undefined, %9
// CHECK-NEXT:  %11 = StoreStackInst %10, %2
// CHECK-NEXT:  %12 = LoadStackInst %2
// CHECK-NEXT:  %13 = ReturnInst %12
// CHECK-NEXT:function_end

// CHECK:function sink(x, y)
// CHECK-NEXT:frame = [x, y]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %x
// CHECK-NEXT:  %1 = StoreFrameInst %0, [x]
// CHECK-NEXT:  %2 = LoadParamInst %y
// CHECK-NEXT:  %3 = StoreFrameInst %2, [y]
// CHECK-NEXT:  %4 = LoadFrameInst [x]
// CHECK-NEXT:  %5 = LoadPropertyInst %4, "bar" : string
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = LoadFrameInst [x]
// CHECK-NEXT:  %8 = LoadPropertyInst %7, "bar" : string
// CHECK-NEXT:  %9 = ReturnInst %8
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %10 = LoadFrameInst [x]
// CHECK-NEXT:  %11 = LoadFrameInst [y]
// CHECK-NEXT:  %12 = LoadPropertyInst %10, %11
// CHECK-NEXT:  %13 = ReturnInst %12
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %14 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
