/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

var {['a']: b} = x;

var {['a']: b, ...rest} = x;

var {[foo()]: b, c: d, ...rest} = x;

var {} = x;

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [b, rest, d]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %2 = StoreStackInst undefined : undefined, %1
// CHECK-NEXT:  %3 = TryLoadGlobalPropertyInst globalObject : object, "x" : string
// CHECK-NEXT:  %4 = LoadPropertyInst %3, "a" : string
// CHECK-NEXT:  %5 = StorePropertyInst %4, globalObject : object, "b" : string
// CHECK-NEXT:  %6 = TryLoadGlobalPropertyInst globalObject : object, "x" : string
// CHECK-NEXT:  %7 = LoadPropertyInst %6, "a" : string
// CHECK-NEXT:  %8 = StorePropertyInst %7, globalObject : object, "b" : string
// CHECK-NEXT:  %9 = AllocObjectLiteralInst "a" : string, 0 : number
// CHECK-NEXT:  %10 = AllocObjectInst 0 : number, empty
// CHECK-NEXT:  %11 = CallBuiltinInst [HermesBuiltin.copyDataProperties] : number, undefined : undefined, %10 : object, %6, %9 : object
// CHECK-NEXT:  %12 = StorePropertyInst %11, globalObject : object, "rest" : string
// CHECK-NEXT:  %13 = TryLoadGlobalPropertyInst globalObject : object, "x" : string
// CHECK-NEXT:  %14 = TryLoadGlobalPropertyInst globalObject : object, "foo" : string
// CHECK-NEXT:  %15 = CallInst %14, undefined : undefined
// CHECK-NEXT:  %16 = LoadPropertyInst %13, %15
// CHECK-NEXT:  %17 = StorePropertyInst %16, globalObject : object, "b" : string
// CHECK-NEXT:  %18 = LoadPropertyInst %13, "c" : string
// CHECK-NEXT:  %19 = StorePropertyInst %18, globalObject : object, "d" : string
// CHECK-NEXT:  %20 = AllocObjectLiteralInst "c" : string, 0 : number
// CHECK-NEXT:  %21 = StorePropertyInst 0 : number, %20 : object, %15
// CHECK-NEXT:  %22 = AllocObjectInst 0 : number, empty
// CHECK-NEXT:  %23 = CallBuiltinInst [HermesBuiltin.copyDataProperties] : number, undefined : undefined, %22 : object, %13, %20 : object
// CHECK-NEXT:  %24 = StorePropertyInst %23, globalObject : object, "rest" : string
// CHECK-NEXT:  %25 = TryLoadGlobalPropertyInst globalObject : object, "x" : string
// CHECK-NEXT:  %26 = BinaryOperatorInst '==', %25, null : null
// CHECK-NEXT:  %27 = CondBranchInst %26, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %28 = CallBuiltinInst [HermesBuiltin.throwTypeError] : number, undefined : undefined, %25, "Cannot destructure 'undefined' or 'null'." : string
// CHECK-NEXT:  %29 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %30 = LoadStackInst %1
// CHECK-NEXT:  %31 = ReturnInst %30
// CHECK-NEXT:function_end
