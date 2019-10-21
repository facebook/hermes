/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir %s | %FileCheck %s --match-full-lines

// CHECK-LABEL: function global()
// CHECK-NEXT: frame = [], globals = [b, rest, d]
// CHECK-NEXT: %BB0:
// CHECK-NEXT:   %0 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:   %1 = StoreStackInst undefined : undefined, %0

var {['a']: b} = x;
// CHECK-NEXT:   %2 = TryLoadGlobalPropertyInst globalObject : object, "x" : string
// CHECK-NEXT:   %3 = LoadPropertyInst %2, "a" : string
// CHECK-NEXT:   %4 = StorePropertyInst %3, globalObject : object, "b" : string

var {['a']: b, ...rest} = x;
// CHECK-NEXT:   %5 = TryLoadGlobalPropertyInst globalObject : object, "x" : string
// CHECK-NEXT:   %6 = LoadPropertyInst %5, "a" : string
// CHECK-NEXT:   %7 = StorePropertyInst %6, globalObject : object, "b" : string
// CHECK-NEXT:   %8 = HBCAllocObjectFromBufferInst 1 : number, "a" : string, 0 : number
// CHECK-NEXT:   %9 = AllocObjectInst 0 : number, empty
// CHECK-NEXT:   %10 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
// CHECK-NEXT:   %11 = LoadPropertyInst %10, "copyDataProperties" : string
// CHECK-NEXT:   %12 = CallInst %11, undefined : undefined, %9 : object, %5, %8 : object
// CHECK-NEXT:   %13 = StorePropertyInst %12, globalObject : object, "rest" : string

var {[foo()]: b, c: d, ...rest} = x;
// CHECK-NEXT:   %14 = TryLoadGlobalPropertyInst globalObject : object, "x" : string
// CHECK-NEXT:   %15 = TryLoadGlobalPropertyInst globalObject : object, "foo" : string
// CHECK-NEXT:   %16 = CallInst %15, undefined : undefined
// CHECK-NEXT:   %17 = LoadPropertyInst %14, %16
// CHECK-NEXT:   %18 = StorePropertyInst %17, globalObject : object, "b" : string
// CHECK-NEXT:   %19 = LoadPropertyInst %14, "c" : string
// CHECK-NEXT:   %20 = StorePropertyInst %19, globalObject : object, "d" : string
// CHECK-NEXT:   %21 = HBCAllocObjectFromBufferInst 2 : number, "c" : string, 0 : number
// CHECK-NEXT:   %22 = StorePropertyInst 0 : number, %21 : object, %16
// CHECK-NEXT:   %23 = AllocObjectInst 0 : number, empty
// CHECK-NEXT:   %24 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
// CHECK-NEXT:   %25 = LoadPropertyInst %24, "copyDataProperties" : string
// CHECK-NEXT:   %26 = CallInst %25, undefined : undefined, %23 : object, %14, %21 : object
// CHECK-NEXT:   %27 = StorePropertyInst %26, globalObject : object, "rest" : string

// CHECK-NEXT:   %28 = LoadStackInst %0
// CHECK-NEXT:   %29 = ReturnInst %28
// CHECK-NEXT: function_end
