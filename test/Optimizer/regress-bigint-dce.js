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
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "f1_throws": string
// CHECK-NEXT:       DeclareGlobalVarInst "f2_throws": string
// CHECK-NEXT:       DeclareGlobalVarInst "f3_ok": string
// CHECK-NEXT:       DeclareGlobalVarInst "f4_ok": string
// CHECK-NEXT:       DeclareGlobalVarInst "f5_ok": string
// CHECK-NEXT:       DeclareGlobalVarInst "f6_ok": string
// CHECK-NEXT:       DeclareGlobalVarInst "f7_ok": string
// CHECK-NEXT:       DeclareGlobalVarInst "f8_ok": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %0: environment, %f1_throws(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %9: object, globalObject: object, "f1_throws": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %0: environment, %f2_throws(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %11: object, globalObject: object, "f2_throws": string
// CHECK-NEXT:  %13 = CreateFunctionInst (:object) %0: environment, %f3_ok(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %13: object, globalObject: object, "f3_ok": string
// CHECK-NEXT:  %15 = CreateFunctionInst (:object) %0: environment, %f4_ok(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %15: object, globalObject: object, "f4_ok": string
// CHECK-NEXT:  %17 = CreateFunctionInst (:object) %0: environment, %f5_ok(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %17: object, globalObject: object, "f5_ok": string
// CHECK-NEXT:  %19 = CreateFunctionInst (:object) %0: environment, %f6_ok(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %19: object, globalObject: object, "f6_ok": string
// CHECK-NEXT:  %21 = CreateFunctionInst (:object) %0: environment, %f7_ok(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %21: object, globalObject: object, "f7_ok": string
// CHECK-NEXT:  %23 = CreateFunctionInst (:object) %0: environment, %f8_ok(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %23: object, globalObject: object, "f8_ok": string
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
