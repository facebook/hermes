/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O | %FileCheckOrRegen %s --match-full-lines

// Make sure we can remove all trampolines from our code.

function sink() {}

function test_one(x,y) {
  var sink = x;

  sink(x + 2);

  sink(2 + 2);

  sink(x * 2 + x * 2);

  sink((x|0) + (x|0));

  sink("hi" + "bye");

  sink(x + y);

  sink("hi" + y);

  sink(null + null);

  sink({} + {});

  sink(undefined + undefined);
}

function test_unary(x) {
  var sk = sink;
  var y = x;

  sk(void x); // Undef
  sk(!x);     // bool
  sk(y++);    // number
  sk(typeof(x)); // string

}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "sink": string
// CHECK-NEXT:       DeclareGlobalVarInst "test_one": string
// CHECK-NEXT:       DeclareGlobalVarInst "test_unary": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %0: environment, %sink(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %4: object, globalObject: object, "sink": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %0: environment, %test_one(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %6: object, globalObject: object, "test_one": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %0: environment, %test_unary(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %8: object, globalObject: object, "test_unary": string
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function sink(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function test_one(x: any, y: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  %2 = BinaryAddInst (:string|number) %0: any, 2: number
// CHECK-NEXT:  %3 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %2: string|number
// CHECK-NEXT:  %4 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 4: number
// CHECK-NEXT:  %5 = BinaryMultiplyInst (:number) %0: any, 2: number
// CHECK-NEXT:  %6 = BinaryMultiplyInst (:number) %0: any, 2: number
// CHECK-NEXT:  %7 = FAddInst (:number) %5: number, %6: number
// CHECK-NEXT:  %8 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %7: number
// CHECK-NEXT:  %9 = AsInt32Inst (:number) %0: any
// CHECK-NEXT:  %10 = AsInt32Inst (:number) %0: any
// CHECK-NEXT:  %11 = FAddInst (:number) %9: number, %10: number
// CHECK-NEXT:  %12 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %11: number
// CHECK-NEXT:  %13 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, "hibye": string
// CHECK-NEXT:  %14 = BinaryAddInst (:string|number|bigint) %0: any, %1: any
// CHECK-NEXT:  %15 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %14: string|number|bigint
// CHECK-NEXT:  %16 = BinaryAddInst (:string) "hi": string, %1: any
// CHECK-NEXT:  %17 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %16: string
// CHECK-NEXT:  %18 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:  %19 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:  %20 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:  %21 = BinaryAddInst (:string|number|bigint) %19: object, %20: object
// CHECK-NEXT:  %22 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %21: string|number|bigint
// CHECK-NEXT:  %23 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, NaN: number
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function test_unary(x: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) globalObject: object, "sink": string
// CHECK-NEXT:  %2 = UnaryVoidInst (:undefined) %0: any
// CHECK-NEXT:  %3 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %2: undefined
// CHECK-NEXT:  %4 = UnaryBangInst (:boolean) %0: any
// CHECK-NEXT:  %5 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %4: boolean
// CHECK-NEXT:  %6 = AsNumericInst (:number|bigint) %0: any
// CHECK-NEXT:  %7 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %6: number|bigint
// CHECK-NEXT:  %8 = UnaryTypeofInst (:string) %0: any
// CHECK-NEXT:  %9 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %8: string
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end
