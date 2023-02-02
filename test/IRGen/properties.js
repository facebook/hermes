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
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "y" : string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "sink" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %sink()
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2 : closure, globalObject : object, "sink" : string
// CHECK-NEXT:  %4 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %5 = StoreStackInst undefined : undefined, %4
// CHECK-NEXT:  %6 = StorePropertyLooseInst 2 : number, globalObject : object, "y" : string
// CHECK-NEXT:  %7 = LoadPropertyInst globalObject : object, "y" : string
// CHECK-NEXT:  %8 = StorePropertyLooseInst 3 : number, %7, "bar" : string
// CHECK-NEXT:  %9 = StoreStackInst 3 : number, %4
// CHECK-NEXT:  %10 = LoadPropertyInst globalObject : object, "sink" : string
// CHECK-NEXT:  %11 = LoadPropertyInst globalObject : object, "y" : string
// CHECK-NEXT:  %12 = CallInst %10, undefined : undefined, %11
// CHECK-NEXT:  %13 = StoreStackInst %12, %4
// CHECK-NEXT:  %14 = LoadStackInst %4
// CHECK-NEXT:  %15 = ReturnInst %14
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
