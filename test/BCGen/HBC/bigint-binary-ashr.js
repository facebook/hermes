/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O %s -dump-ir | %FileCheckOrRegen --check-prefix=CHKIR %s

// << can no longer be assumed to return number -- it returns
// a numeric when its arguments' types are unknown.

function numberPlusBigInt() {
  return (1+(BigInt(2)>>BigInt(1)));
}

// Auto-generated content below. Please do not modify manually.

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "numberPlusBigInt": string
// CHKIR-NEXT:  %1 = CreateFunctionInst (:object) %numberPlusBigInt(): number
// CHKIR-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "numberPlusBigInt": string
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function numberPlusBigInt(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "BigInt": string
// CHKIR-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 2: number
// CHKIR-NEXT:  %2 = TryLoadGlobalPropertyInst (:any) globalObject: object, "BigInt": string
// CHKIR-NEXT:  %3 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 1: number
// CHKIR-NEXT:  %4 = BinaryRightShiftInst (:number|bigint) %1: any, %3: any
// CHKIR-NEXT:  %5 = BinaryAddInst (:number) 1: number, %4: number|bigint
// CHKIR-NEXT:       ReturnInst %5: number
// CHKIR-NEXT:function_end
