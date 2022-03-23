/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheck %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O


//CHECK-LABEL:function global()
//CHECK-NEXT:frame = [], globals = [y, sink]
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = CreateFunctionInst %sink()
//CHECK-NEXT:    %1 = StorePropertyInst %0 : closure, globalObject : object, "sink" : string
//CHECK-NEXT:    %2 = AllocStackInst $?anon_0_ret
//CHECK-NEXT:    %3 = StoreStackInst undefined : undefined, %2
//CHECK-NEXT:    %4 = StorePropertyInst 2 : number, globalObject : object, "y" : string
//CHECK-NEXT:    %5 = LoadPropertyInst globalObject : object, "y" : string
//CHECK-NEXT:    %6 = StorePropertyInst 3 : number, %5, "bar" : string
//CHECK-NEXT:    %7 = StoreStackInst 3 : number, %2
//CHECK-NEXT:    %8 = LoadPropertyInst globalObject : object, "sink" : string
//CHECK-NEXT:    %9 = LoadPropertyInst globalObject : object, "y" : string
//CHECK-NEXT:    %10 = CallInst %8, undefined : undefined, %9
//CHECK-NEXT:    %11 = StoreStackInst %10, %2
//CHECK-NEXT:    %12 = LoadStackInst %2
//CHECK-NEXT:    %13 = ReturnInst %12
//CHECK-NEXT:function_end

//CHECK-LABEL:function sink(x, y)
//CHECK-NEXT:frame = [x, y]
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = StoreFrameInst %x, [x]
//CHECK-NEXT:    %1 = StoreFrameInst %y, [y]
//CHECK-NEXT:    %2 = LoadFrameInst [x]
//CHECK-NEXT:    %3 = LoadPropertyInst %2, "bar" : string
//CHECK-NEXT:    %4 = ReturnInst %3
//CHECK-NEXT:  %BB1:
//CHECK-NEXT:    %5 = LoadFrameInst [x]
//CHECK-NEXT:    %6 = LoadPropertyInst %5, "bar" : string
//CHECK-NEXT:    %7 = ReturnInst %6
//CHECK-NEXT:  %BB2:
//CHECK-NEXT:    %8 = LoadFrameInst [x]
//CHECK-NEXT:    %9 = LoadFrameInst [y]
//CHECK-NEXT:    %10 = LoadPropertyInst %8, %9
//CHECK-NEXT:    %11 = ReturnInst %10
//CHECK-NEXT:  %BB3:
//CHECK-NEXT:    %12 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
var y = 2;
y.bar = 3;
sink(y)

function sink(x, y) {
  return x.bar
  return x["bar"]
  return x[y]
}

