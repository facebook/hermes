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
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "test_newline": string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "test_quote": string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "test_slash": string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "test_hex": string
// CHECK-NEXT:  %4 = DeclareGlobalVarInst "test_hex_printable": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:closure) %test_newline(): any
// CHECK-NEXT:  %6 = StorePropertyLooseInst %5: closure, globalObject: object, "test_newline": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:closure) %test_quote(): any
// CHECK-NEXT:  %8 = StorePropertyLooseInst %7: closure, globalObject: object, "test_quote": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:closure) %test_slash(): any
// CHECK-NEXT:  %10 = StorePropertyLooseInst %9: closure, globalObject: object, "test_slash": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:closure) %test_hex(): any
// CHECK-NEXT:  %12 = StorePropertyLooseInst %11: closure, globalObject: object, "test_hex": string
// CHECK-NEXT:  %13 = CreateFunctionInst (:closure) %test_hex_printable(): any
// CHECK-NEXT:  %14 = StorePropertyLooseInst %13: closure, globalObject: object, "test_hex_printable": string
// CHECK-NEXT:  %15 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %16 = StoreStackInst undefined: undefined, %15: any
// CHECK-NEXT:  %17 = LoadStackInst (:any) %15: any
// CHECK-NEXT:  %18 = ReturnInst (:any) %17: any
// CHECK-NEXT:function_end

// CHECK:function test_newline(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, "A string with a newline\\n": string
// CHECK-NEXT:  %2 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function test_quote(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, "A string with a newline\\\"": string
// CHECK-NEXT:  %2 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function test_slash(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, "A string with a newline\\\\": string
// CHECK-NEXT:  %2 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function test_hex(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, "A string with a hex: \\x03": string
// CHECK-NEXT:  %2 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function test_hex_printable(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, "A string with a hex printable: a": string
// CHECK-NEXT:  %2 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:function_end
