/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -hermes-parser -dump-ir %s -O

function test_newline() {
  print("A string with a newline\n");
}

function test_quote() {
  print("A string with a newline\"");
}

function test_slash() {
  print("A string with a newline\\");
}

function test_hex() {
  print("A string with a hex: \x03");
}

function test_hex_printable() {
  print("A string with a hex printable: \x61");
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "test_newline": string
// CHECK-NEXT:       DeclareGlobalVarInst "test_quote": string
// CHECK-NEXT:       DeclareGlobalVarInst "test_slash": string
// CHECK-NEXT:       DeclareGlobalVarInst "test_hex": string
// CHECK-NEXT:       DeclareGlobalVarInst "test_hex_printable": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %test_newline(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %5: object, globalObject: object, "test_newline": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %test_quote(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %7: object, globalObject: object, "test_quote": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %test_slash(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %9: object, globalObject: object, "test_slash": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %test_hex(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %11: object, globalObject: object, "test_hex": string
// CHECK-NEXT:  %13 = CreateFunctionInst (:object) %test_hex_printable(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %13: object, globalObject: object, "test_hex_printable": string
// CHECK-NEXT:  %15 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %15: any
// CHECK-NEXT:  %17 = LoadStackInst (:any) %15: any
// CHECK-NEXT:        ReturnInst %17: any
// CHECK-NEXT:function_end

// CHECK:function test_newline(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, "A string with a newline\\n": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function test_quote(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, "A string with a newline\\\"": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function test_slash(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, "A string with a newline\\\\": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function test_hex(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, "A string with a hex: \\x03": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function test_hex_printable(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, "A string with a hex printable: a": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
