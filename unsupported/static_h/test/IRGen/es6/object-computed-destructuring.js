/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheck %s --match-full-lines

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
// CHECK-NEXT:   %8 = AllocObjectLiteralInst "a" : string, 0 : number
// CHECK-NEXT:   %9 = AllocObjectInst 0 : number, empty
// CHECK-NEXT:   %10 = CallBuiltinInst [HermesBuiltin.copyDataProperties] : number, undefined : undefined, %9 : object, %5, %8 : object
// CHECK-NEXT:   %11 = StorePropertyInst %10, globalObject : object, "rest" : string

var {[foo()]: b, c: d, ...rest} = x;
// CHECK-NEXT:   %12 = TryLoadGlobalPropertyInst globalObject : object, "x" : string
// CHECK-NEXT:   %13 = TryLoadGlobalPropertyInst globalObject : object, "foo" : string
// CHECK-NEXT:   %14 = CallInst %13, undefined : undefined
// CHECK-NEXT:   %15 = LoadPropertyInst %12, %14
// CHECK-NEXT:   %16 = StorePropertyInst %15, globalObject : object, "b" : string
// CHECK-NEXT:   %17 = LoadPropertyInst %12, "c" : string
// CHECK-NEXT:   %18 = StorePropertyInst %17, globalObject : object, "d" : string
// CHECK-NEXT:   %19 = AllocObjectLiteralInst "c" : string, 0 : number
// CHECK-NEXT:   %20 = StorePropertyInst 0 : number, %19 : object, %14
// CHECK-NEXT:   %21 = AllocObjectInst 0 : number, empty
// CHECK-NEXT:   %22 = CallBuiltinInst [HermesBuiltin.copyDataProperties] : number, undefined : undefined, %21 : object, %12, %19 : object
// CHECK-NEXT:   %23 = StorePropertyInst %22, globalObject : object, "rest" : string

var {} = x;
// CHECK-NEXT:   %24 = TryLoadGlobalPropertyInst globalObject : object, "x" : string
// CHECK-NEXT:   %25 = BinaryOperatorInst '==', %24, null : null
// CHECK-NEXT:   %26 = CondBranchInst %25, %BB1, %BB2
// CHECK-NEXT: %BB1:
// CHECK-NEXT:   %27 = CallBuiltinInst [HermesBuiltin.throwTypeError] : number, undefined : undefined, %24, "Cannot destructure 'undefined' or 'null'." : string
// CHECK-NEXT:   %28 = ReturnInst undefined : undefined
// CHECK-NEXT: %BB2:
// CHECK-NEXT:   %29 = LoadStackInst %0
// CHECK-NEXT:   %30 = ReturnInst %29
// CHECK-NEXT: function_end
