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
// CHECK-NEXT:globals = [b, rest, d]
// CHECK-NEXT:S{global#0()#1} = []
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
// CHECK-NEXT:  %10 = CallBuiltinInst [HermesBuiltin.silentSetPrototypeOf] : number, undefined : undefined, undefined : undefined, %9 : object, null : null
// CHECK-NEXT:  %11 = AllocObjectInst 0 : number, empty
// CHECK-NEXT:  %12 = CallBuiltinInst [HermesBuiltin.copyDataProperties] : number, undefined : undefined, undefined : undefined, %11 : object, %6, %9 : object
// CHECK-NEXT:  %13 = StorePropertyInst %12, globalObject : object, "rest" : string
// CHECK-NEXT:  %14 = TryLoadGlobalPropertyInst globalObject : object, "x" : string
// CHECK-NEXT:  %15 = TryLoadGlobalPropertyInst globalObject : object, "foo" : string
// CHECK-NEXT:  %16 = CallInst %15, undefined : undefined, undefined : undefined
// CHECK-NEXT:  %17 = LoadPropertyInst %14, %16
// CHECK-NEXT:  %18 = StorePropertyInst %17, globalObject : object, "b" : string
// CHECK-NEXT:  %19 = LoadPropertyInst %14, "c" : string
// CHECK-NEXT:  %20 = StorePropertyInst %19, globalObject : object, "d" : string
// CHECK-NEXT:  %21 = AllocObjectLiteralInst "c" : string, 0 : number
// CHECK-NEXT:  %22 = CallBuiltinInst [HermesBuiltin.silentSetPrototypeOf] : number, undefined : undefined, undefined : undefined, %21 : object, null : null
// CHECK-NEXT:  %23 = StorePropertyInst 0 : number, %21 : object, %16
// CHECK-NEXT:  %24 = AllocObjectInst 0 : number, empty
// CHECK-NEXT:  %25 = CallBuiltinInst [HermesBuiltin.copyDataProperties] : number, undefined : undefined, undefined : undefined, %24 : object, %14, %21 : object
// CHECK-NEXT:  %26 = StorePropertyInst %25, globalObject : object, "rest" : string
// CHECK-NEXT:  %27 = TryLoadGlobalPropertyInst globalObject : object, "x" : string
// CHECK-NEXT:  %28 = BinaryOperatorInst '==', %27, null : null
// CHECK-NEXT:  %29 = CondBranchInst %28, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %30 = CallBuiltinInst [HermesBuiltin.throwTypeError] : number, undefined : undefined, undefined : undefined, %27, "Cannot destructure 'undefined' or 'null'." : string
// CHECK-NEXT:  %31 = ThrowInst undefined : undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %32 = LoadStackInst %1
// CHECK-NEXT:  %33 = ReturnInst %32
// CHECK-NEXT:function_end
