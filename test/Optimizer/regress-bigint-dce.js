/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir %s -O | %FileCheckOrRegen %s --match-full-lines

// Verify that arithmetic operations with a BigInt operand and non-BigInt
// operand (except adds) are not DCE-d, because that would throw a runtime
// exception.

function f1_throws() { 1n << 1; }
function f2_throws() { 1n + 1; }
function f3_ok() { "a" + 1n; }
function f4_ok() { 1n + "a"; }
function f5_ok() { 1n < "a"; }
function f6_ok() { "a" >= 1n; }
function f7_ok() { 1 >= 1n; }
function f8_ok() { 1 * "a"; }

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "f1_throws": string
// CHECK-NEXT:       DeclareGlobalVarInst "f2_throws": string
// CHECK-NEXT:       DeclareGlobalVarInst "f3_ok": string
// CHECK-NEXT:       DeclareGlobalVarInst "f4_ok": string
// CHECK-NEXT:       DeclareGlobalVarInst "f5_ok": string
// CHECK-NEXT:       DeclareGlobalVarInst "f6_ok": string
// CHECK-NEXT:       DeclareGlobalVarInst "f7_ok": string
// CHECK-NEXT:       DeclareGlobalVarInst "f8_ok": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %f1_throws(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %8: object, globalObject: object, "f1_throws": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %f2_throws(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %10: object, globalObject: object, "f2_throws": string
// CHECK-NEXT:  %12 = CreateFunctionInst (:object) %f3_ok(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %12: object, globalObject: object, "f3_ok": string
// CHECK-NEXT:  %14 = CreateFunctionInst (:object) %f4_ok(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %14: object, globalObject: object, "f4_ok": string
// CHECK-NEXT:  %16 = CreateFunctionInst (:object) %f5_ok(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %16: object, globalObject: object, "f5_ok": string
// CHECK-NEXT:  %18 = CreateFunctionInst (:object) %f6_ok(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %18: object, globalObject: object, "f6_ok": string
// CHECK-NEXT:  %20 = CreateFunctionInst (:object) %f7_ok(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %20: object, globalObject: object, "f7_ok": string
// CHECK-NEXT:  %22 = CreateFunctionInst (:object) %f8_ok(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %22: object, globalObject: object, "f8_ok": string
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function f1_throws(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = BinaryLeftShiftInst (:number) 1n: bigint, 1: number
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function f2_throws(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = BinaryAddInst (:number) 1n: bigint, 1: number
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function f3_ok(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function f4_ok(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function f5_ok(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function f6_ok(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function f7_ok(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function f8_ok(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
