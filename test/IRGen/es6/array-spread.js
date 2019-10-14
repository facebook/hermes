// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermesc -dump-ir %s | %FileCheck --match-full-lines %s
// RUN: %hermesc -O -dump-ir %s | %FileCheck --match-full-lines -check-prefix OPT %s

function foo(x) {
  return [1, 2, ...x, 3, 4];
}
// CHECK-LABEL: function foo(x)
// CHECK-NEXT: frame = [x]
// CHECK-NEXT: %BB0:
// CHECK-NEXT:   %0 = StoreFrameInst %x, [x]
// CHECK-NEXT:   %1 = AllocStackInst $nextIndex
// CHECK-NEXT:   %2 = StoreStackInst 0 : number, %1
// CHECK-NEXT:   %3 = LoadStackInst %1
// CHECK-NEXT:   %4 = BinaryOperatorInst '+', %3, 1 : number
// CHECK-NEXT:   %5 = StoreStackInst %4, %1
// CHECK-NEXT:   %6 = LoadStackInst %1
// CHECK-NEXT:   %7 = BinaryOperatorInst '+', %6, 1 : number
// CHECK-NEXT:   %8 = StoreStackInst %7, %1
// CHECK-NEXT:   %9 = LoadFrameInst [x]
// CHECK-NEXT:   %10 = AllocArrayInst 4 : number, 1 : number, 2 : number
// CHECK-NEXT:   %11 = LoadStackInst %1
// CHECK-NEXT:   %12 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
// CHECK-NEXT:   %13 = LoadPropertyInst %12, "arraySpread" : string
// CHECK-NEXT:   %14 = CallInst %13, undefined : undefined, %10 : object, %9, %11
// CHECK-NEXT:   %15 = StoreStackInst %14, %1
// CHECK-NEXT:   %16 = LoadStackInst %1
// CHECK-NEXT:   %17 = StoreOwnPropertyInst 3 : number, %10 : object, %16, true : boolean
// CHECK-NEXT:   %18 = LoadStackInst %1
// CHECK-NEXT:   %19 = BinaryOperatorInst '+', %18, 1 : number
// CHECK-NEXT:   %20 = StoreStackInst %19, %1
// CHECK-NEXT:   %21 = LoadStackInst %1
// CHECK-NEXT:   %22 = StoreOwnPropertyInst 4 : number, %10 : object, %21, true : boolean
// CHECK-NEXT:   %23 = LoadStackInst %1
// CHECK-NEXT:   %24 = BinaryOperatorInst '+', %23, 1 : number
// CHECK-NEXT:   %25 = StoreStackInst %24, %1
// CHECK-NEXT:   %26 = ReturnInst %10 : object
// CHECK-NEXT: %BB1:
// CHECK-NEXT:   %27 = ReturnInst undefined : undefined
// CHECK-NEXT: function_end

// OPT-LABEL: function foo(x) : object
// OPT-NEXT: frame = []
// OPT-NEXT: %BB0:
// OPT-NEXT:   %0 = AllocArrayInst 4 : number, 1 : number, 2 : number
// OPT-NEXT:   %1 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
// OPT-NEXT:   %2 = LoadPropertyInst %1, "arraySpread" : string
// OPT-NEXT:   %3 = CallInst %2, undefined : undefined, %0 : object, %x, 2 : number
// OPT-NEXT:   %4 = StoreOwnPropertyInst 3 : number, %0 : object, %3, true : boolean
// OPT-NEXT:   %5 = BinaryOperatorInst '+', %3, 1 : number
// OPT-NEXT:   %6 = StoreOwnPropertyInst 4 : number, %0 : object, %5 : string|number, true : boolean
// OPT-NEXT:   %7 = ReturnInst %0 : object
// OPT-NEXT: function_end
