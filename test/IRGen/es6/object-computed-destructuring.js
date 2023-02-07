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

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "b" : string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "rest" : string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "d" : string
// CHECK-NEXT:  %3 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %4 = StoreStackInst undefined : undefined, %3
// CHECK-NEXT:  %5 = TryLoadGlobalPropertyInst globalObject : object, "x" : string
// CHECK-NEXT:  %6 = LoadPropertyInst %5, "a" : string
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6, globalObject : object, "b" : string
// CHECK-NEXT:  %8 = TryLoadGlobalPropertyInst globalObject : object, "x" : string
// CHECK-NEXT:  %9 = LoadPropertyInst %8, "a" : string
// CHECK-NEXT:  %10 = StorePropertyLooseInst %9, globalObject : object, "b" : string
// CHECK-NEXT:  %11 = AllocObjectLiteralInst "a" : string, 0 : number
// CHECK-NEXT:  %12 = AllocObjectInst 0 : number, empty
// CHECK-NEXT:  %13 = CallBuiltinInst [HermesBuiltin.copyDataProperties] : number, empty, empty, undefined : undefined, %12 : object, %8, %11 : object
// CHECK-NEXT:  %14 = StorePropertyLooseInst %13, globalObject : object, "rest" : string
// CHECK-NEXT:  %15 = TryLoadGlobalPropertyInst globalObject : object, "x" : string
// CHECK-NEXT:  %16 = TryLoadGlobalPropertyInst globalObject : object, "foo" : string
// CHECK-NEXT:  %17 = CallInst %16, empty, empty, undefined : undefined
// CHECK-NEXT:  %18 = LoadPropertyInst %15, %17
// CHECK-NEXT:  %19 = StorePropertyLooseInst %18, globalObject : object, "b" : string
// CHECK-NEXT:  %20 = LoadPropertyInst %15, "c" : string
// CHECK-NEXT:  %21 = StorePropertyLooseInst %20, globalObject : object, "d" : string
// CHECK-NEXT:  %22 = AllocObjectLiteralInst "c" : string, 0 : number
// CHECK-NEXT:  %23 = StorePropertyLooseInst 0 : number, %22 : object, %17
// CHECK-NEXT:  %24 = AllocObjectInst 0 : number, empty
// CHECK-NEXT:  %25 = CallBuiltinInst [HermesBuiltin.copyDataProperties] : number, empty, empty, undefined : undefined, %24 : object, %15, %22 : object
// CHECK-NEXT:  %26 = StorePropertyLooseInst %25, globalObject : object, "rest" : string
// CHECK-NEXT:  %27 = TryLoadGlobalPropertyInst globalObject : object, "x" : string
// CHECK-NEXT:  %28 = BinaryOperatorInst '==', %27, null : null
// CHECK-NEXT:  %29 = CondBranchInst %28, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %30 = CallBuiltinInst [HermesBuiltin.throwTypeError] : number, empty, empty, undefined : undefined, %27, "Cannot destructure 'undefined' or 'null'." : string
// CHECK-NEXT:  %31 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %32 = LoadStackInst %3
// CHECK-NEXT:  %33 = ReturnInst %32
// CHECK-NEXT:function_end
