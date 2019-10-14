// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -dump-ir %s | %FileCheck --match-full-lines %s
// RUN: %hermes -dump-ir -O %s | %FileCheck --match-full-lines -check-prefix=OPT %s

function foo(fn, x) {
  fn(...x);
  new fn(...x);
}
// CHECK-LABEL: function foo(fn, x)
// CHECK-NEXT: frame = [fn, x]
// CHECK-NEXT: %BB0:
// CHECK-NEXT:   %0 = StoreFrameInst %fn, [fn]
// CHECK-NEXT:   %1 = StoreFrameInst %x, [x]
// CHECK-NEXT:   %2 = LoadFrameInst [fn]
// CHECK-NEXT:   %3 = AllocStackInst $nextIndex
// CHECK-NEXT:   %4 = StoreStackInst 0 : number, %3
// CHECK-NEXT:   %5 = LoadFrameInst [x]
// CHECK-NEXT:   %6 = AllocArrayInst 0 : number
// CHECK-NEXT:   %7 = LoadStackInst %3
// CHECK-NEXT:   %8 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
// CHECK-NEXT:   %9 = LoadPropertyInst %8, "arraySpread" : string
// CHECK-NEXT:   %10 = CallInst %9, undefined : undefined, %6 : object, %5, %7
// CHECK-NEXT:   %11 = StoreStackInst %10, %3
// CHECK-NEXT:   %12 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
// CHECK-NEXT:   %13 = LoadPropertyInst %12, "apply" : string
// CHECK-NEXT:   %14 = CallInst %13, undefined : undefined, %2, %6 : object, undefined : undefined
// CHECK-NEXT:   %15 = LoadFrameInst [fn]
// CHECK-NEXT:   %16 = AllocStackInst $nextIndex
// CHECK-NEXT:   %17 = StoreStackInst 0 : number, %16
// CHECK-NEXT:   %18 = LoadFrameInst [x]
// CHECK-NEXT:   %19 = AllocArrayInst 0 : number
// CHECK-NEXT:   %20 = LoadStackInst %16
// CHECK-NEXT:   %21 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
// CHECK-NEXT:   %22 = LoadPropertyInst %21, "arraySpread" : string
// CHECK-NEXT:   %23 = CallInst %22, undefined : undefined, %19 : object, %18, %20
// CHECK-NEXT:   %24 = StoreStackInst %23, %16
// CHECK-NEXT:   %25 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
// CHECK-NEXT:   %26 = LoadPropertyInst %25, "apply" : string
// CHECK-NEXT:   %27 = CallInst %26, undefined : undefined, %15, %19 : object
// CHECK-NEXT:   %28 = ReturnInst undefined : undefined
// CHECK-NEXT: function_end

// OPT-LABEL: function foo(fn, x) : undefined
// OPT-NEXT: frame = []
// OPT-NEXT: %BB0:
// OPT-NEXT:   %0 = AllocArrayInst 0 : number
// OPT-NEXT:   %1 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
// OPT-NEXT:   %2 = LoadPropertyInst %1, "arraySpread" : string
// OPT-NEXT:   %3 = CallInst %2, undefined : undefined, %0 : object, %x, 0 : number
// OPT-NEXT:   %4 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
// OPT-NEXT:   %5 = LoadPropertyInst %4, "apply" : string
// OPT-NEXT:   %6 = CallInst %5, undefined : undefined, %fn, %0 : object, undefined : undefined
// OPT-NEXT:   %7 = AllocArrayInst 0 : number
// OPT-NEXT:   %8 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
// OPT-NEXT:   %9 = LoadPropertyInst %8, "arraySpread" : string
// OPT-NEXT:   %10 = CallInst %9, undefined : undefined, %7 : object, %x, 0 : number
// OPT-NEXT:   %11 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
// OPT-NEXT:   %12 = LoadPropertyInst %11, "apply" : string
// OPT-NEXT:   %13 = CallInst %12, undefined : undefined, %fn, %7 : object
// OPT-NEXT:   %14 = ReturnInst undefined : undefined
// OPT-NEXT: function_end
