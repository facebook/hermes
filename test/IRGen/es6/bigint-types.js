/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O %s -dump-ir | %FileCheckOrRegen --check-prefix=CHKIR %s

// Operations can no longer be assumed to return number -- they return a
// numeric when arguments types are unknown. This means that
//
//     number + ("thing" + "thing")
//
// can no longer be emitted with an AddN. This is also true for all other
// <foo>N operations.

function add() {
  return (1+(BigInt(2)+BigInt(0)));
}
function shl() {
  return (1+(BigInt(2)<<BigInt(1)));
}
function asr() {
  return (1+(BigInt(2)>>BigInt(1)));
}
function div() {
  return (1+(BigInt(2)/BigInt(1)));
}
function mul() {
  return (1+(BigInt(2)*BigInt(0)));
}
function rem() {
  return (1+(BigInt(2)%BigInt(1)));
}
function sub() {
  return (1+(a-BigInt(0)));
}
function and() {
  return (1+(BigInt(2)&BigInt(1)));
}
function or() {
  return (1+(BigInt(2)|BigInt(1)));
}
function xor() {
  return (1+(BigInt(2)^BigInt(1)));
}
function not() {
  return (1+(~a));
}
function neg() {
  return (1+(-a));
}

// Auto-generated content below. Please do not modify manually.

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = DeclareGlobalVarInst "add": string
// CHKIR-NEXT:  %1 = DeclareGlobalVarInst "shl": string
// CHKIR-NEXT:  %2 = DeclareGlobalVarInst "asr": string
// CHKIR-NEXT:  %3 = DeclareGlobalVarInst "div": string
// CHKIR-NEXT:  %4 = DeclareGlobalVarInst "mul": string
// CHKIR-NEXT:  %5 = DeclareGlobalVarInst "rem": string
// CHKIR-NEXT:  %6 = DeclareGlobalVarInst "sub": string
// CHKIR-NEXT:  %7 = DeclareGlobalVarInst "and": string
// CHKIR-NEXT:  %8 = DeclareGlobalVarInst "or": string
// CHKIR-NEXT:  %9 = DeclareGlobalVarInst "xor": string
// CHKIR-NEXT:  %10 = DeclareGlobalVarInst "not": string
// CHKIR-NEXT:  %11 = DeclareGlobalVarInst "neg": string
// CHKIR-NEXT:  %12 = CreateFunctionInst (:object) %add(): string|number
// CHKIR-NEXT:  %13 = StorePropertyLooseInst %12: object, globalObject: object, "add": string
// CHKIR-NEXT:  %14 = CreateFunctionInst (:object) %shl(): number
// CHKIR-NEXT:  %15 = StorePropertyLooseInst %14: object, globalObject: object, "shl": string
// CHKIR-NEXT:  %16 = CreateFunctionInst (:object) %asr(): number
// CHKIR-NEXT:  %17 = StorePropertyLooseInst %16: object, globalObject: object, "asr": string
// CHKIR-NEXT:  %18 = CreateFunctionInst (:object) %div(): number
// CHKIR-NEXT:  %19 = StorePropertyLooseInst %18: object, globalObject: object, "div": string
// CHKIR-NEXT:  %20 = CreateFunctionInst (:object) %mul(): number
// CHKIR-NEXT:  %21 = StorePropertyLooseInst %20: object, globalObject: object, "mul": string
// CHKIR-NEXT:  %22 = CreateFunctionInst (:object) %rem(): number
// CHKIR-NEXT:  %23 = StorePropertyLooseInst %22: object, globalObject: object, "rem": string
// CHKIR-NEXT:  %24 = CreateFunctionInst (:object) %sub(): number
// CHKIR-NEXT:  %25 = StorePropertyLooseInst %24: object, globalObject: object, "sub": string
// CHKIR-NEXT:  %26 = CreateFunctionInst (:object) %and(): number
// CHKIR-NEXT:  %27 = StorePropertyLooseInst %26: object, globalObject: object, "and": string
// CHKIR-NEXT:  %28 = CreateFunctionInst (:object) %or(): number
// CHKIR-NEXT:  %29 = StorePropertyLooseInst %28: object, globalObject: object, "or": string
// CHKIR-NEXT:  %30 = CreateFunctionInst (:object) %xor(): number
// CHKIR-NEXT:  %31 = StorePropertyLooseInst %30: object, globalObject: object, "xor": string
// CHKIR-NEXT:  %32 = CreateFunctionInst (:object) %not(): number
// CHKIR-NEXT:  %33 = StorePropertyLooseInst %32: object, globalObject: object, "not": string
// CHKIR-NEXT:  %34 = CreateFunctionInst (:object) %neg(): number
// CHKIR-NEXT:  %35 = StorePropertyLooseInst %34: object, globalObject: object, "neg": string
// CHKIR-NEXT:  %36 = ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function add(): string|number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "BigInt": string
// CHKIR-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 2: number
// CHKIR-NEXT:  %2 = TryLoadGlobalPropertyInst (:any) globalObject: object, "BigInt": string
// CHKIR-NEXT:  %3 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, 0: number
// CHKIR-NEXT:  %4 = BinaryAddInst (:string|number|bigint) %1: any, %3: any
// CHKIR-NEXT:  %5 = BinaryAddInst (:string|number) 1: number, %4: string|number|bigint
// CHKIR-NEXT:  %6 = ReturnInst %5: string|number
// CHKIR-NEXT:function_end

// CHKIR:function shl(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "BigInt": string
// CHKIR-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 2: number
// CHKIR-NEXT:  %2 = TryLoadGlobalPropertyInst (:any) globalObject: object, "BigInt": string
// CHKIR-NEXT:  %3 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, 1: number
// CHKIR-NEXT:  %4 = BinaryLeftShiftInst (:number|bigint) %1: any, %3: any
// CHKIR-NEXT:  %5 = BinaryAddInst (:number) 1: number, %4: number|bigint
// CHKIR-NEXT:  %6 = ReturnInst %5: number
// CHKIR-NEXT:function_end

// CHKIR:function asr(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "BigInt": string
// CHKIR-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 2: number
// CHKIR-NEXT:  %2 = TryLoadGlobalPropertyInst (:any) globalObject: object, "BigInt": string
// CHKIR-NEXT:  %3 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, 1: number
// CHKIR-NEXT:  %4 = BinaryRightShiftInst (:number|bigint) %1: any, %3: any
// CHKIR-NEXT:  %5 = BinaryAddInst (:number) 1: number, %4: number|bigint
// CHKIR-NEXT:  %6 = ReturnInst %5: number
// CHKIR-NEXT:function_end

// CHKIR:function div(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "BigInt": string
// CHKIR-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 2: number
// CHKIR-NEXT:  %2 = TryLoadGlobalPropertyInst (:any) globalObject: object, "BigInt": string
// CHKIR-NEXT:  %3 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, 1: number
// CHKIR-NEXT:  %4 = BinaryDivideInst (:number|bigint) %1: any, %3: any
// CHKIR-NEXT:  %5 = BinaryAddInst (:number) 1: number, %4: number|bigint
// CHKIR-NEXT:  %6 = ReturnInst %5: number
// CHKIR-NEXT:function_end

// CHKIR:function mul(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "BigInt": string
// CHKIR-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 2: number
// CHKIR-NEXT:  %2 = TryLoadGlobalPropertyInst (:any) globalObject: object, "BigInt": string
// CHKIR-NEXT:  %3 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, 0: number
// CHKIR-NEXT:  %4 = BinaryMultiplyInst (:number|bigint) %1: any, %3: any
// CHKIR-NEXT:  %5 = BinaryAddInst (:number) 1: number, %4: number|bigint
// CHKIR-NEXT:  %6 = ReturnInst %5: number
// CHKIR-NEXT:function_end

// CHKIR:function rem(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "BigInt": string
// CHKIR-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 2: number
// CHKIR-NEXT:  %2 = TryLoadGlobalPropertyInst (:any) globalObject: object, "BigInt": string
// CHKIR-NEXT:  %3 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, 1: number
// CHKIR-NEXT:  %4 = BinaryModuloInst (:number|bigint) %1: any, %3: any
// CHKIR-NEXT:  %5 = BinaryAddInst (:number) 1: number, %4: number|bigint
// CHKIR-NEXT:  %6 = ReturnInst %5: number
// CHKIR-NEXT:function_end

// CHKIR:function sub(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "a": string
// CHKIR-NEXT:  %1 = TryLoadGlobalPropertyInst (:any) globalObject: object, "BigInt": string
// CHKIR-NEXT:  %2 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, 0: number
// CHKIR-NEXT:  %3 = BinarySubtractInst (:number|bigint) %0: any, %2: any
// CHKIR-NEXT:  %4 = BinaryAddInst (:number) 1: number, %3: number|bigint
// CHKIR-NEXT:  %5 = ReturnInst %4: number
// CHKIR-NEXT:function_end

// CHKIR:function and(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "BigInt": string
// CHKIR-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 2: number
// CHKIR-NEXT:  %2 = TryLoadGlobalPropertyInst (:any) globalObject: object, "BigInt": string
// CHKIR-NEXT:  %3 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, 1: number
// CHKIR-NEXT:  %4 = BinaryAndInst (:number|bigint) %1: any, %3: any
// CHKIR-NEXT:  %5 = BinaryAddInst (:number) 1: number, %4: number|bigint
// CHKIR-NEXT:  %6 = ReturnInst %5: number
// CHKIR-NEXT:function_end

// CHKIR:function or(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "BigInt": string
// CHKIR-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 2: number
// CHKIR-NEXT:  %2 = TryLoadGlobalPropertyInst (:any) globalObject: object, "BigInt": string
// CHKIR-NEXT:  %3 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, 1: number
// CHKIR-NEXT:  %4 = BinaryOrInst (:number|bigint) %1: any, %3: any
// CHKIR-NEXT:  %5 = BinaryAddInst (:number) 1: number, %4: number|bigint
// CHKIR-NEXT:  %6 = ReturnInst %5: number
// CHKIR-NEXT:function_end

// CHKIR:function xor(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "BigInt": string
// CHKIR-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 2: number
// CHKIR-NEXT:  %2 = TryLoadGlobalPropertyInst (:any) globalObject: object, "BigInt": string
// CHKIR-NEXT:  %3 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, 1: number
// CHKIR-NEXT:  %4 = BinaryXorInst (:number|bigint) %1: any, %3: any
// CHKIR-NEXT:  %5 = BinaryAddInst (:number) 1: number, %4: number|bigint
// CHKIR-NEXT:  %6 = ReturnInst %5: number
// CHKIR-NEXT:function_end

// CHKIR:function not(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "a": string
// CHKIR-NEXT:  %1 = UnaryTildeInst (:number|bigint) %0: any
// CHKIR-NEXT:  %2 = BinaryAddInst (:number) 1: number, %1: number|bigint
// CHKIR-NEXT:  %3 = ReturnInst %2: number
// CHKIR-NEXT:function_end

// CHKIR:function neg(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "a": string
// CHKIR-NEXT:  %1 = UnaryMinusInst (:number|bigint) %0: any
// CHKIR-NEXT:  %2 = BinaryAddInst (:number) 1: number, %1: number|bigint
// CHKIR-NEXT:  %3 = ReturnInst %2: number
// CHKIR-NEXT:function_end
