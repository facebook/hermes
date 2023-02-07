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

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "test_newline" : string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "test_quote" : string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "test_slash" : string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "test_hex" : string
// CHECK-NEXT:  %4 = DeclareGlobalVarInst "test_hex_printable" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %test_newline()
// CHECK-NEXT:  %6 = StorePropertyLooseInst %5 : closure, globalObject : object, "test_newline" : string
// CHECK-NEXT:  %7 = CreateFunctionInst %test_quote()
// CHECK-NEXT:  %8 = StorePropertyLooseInst %7 : closure, globalObject : object, "test_quote" : string
// CHECK-NEXT:  %9 = CreateFunctionInst %test_slash()
// CHECK-NEXT:  %10 = StorePropertyLooseInst %9 : closure, globalObject : object, "test_slash" : string
// CHECK-NEXT:  %11 = CreateFunctionInst %test_hex()
// CHECK-NEXT:  %12 = StorePropertyLooseInst %11 : closure, globalObject : object, "test_hex" : string
// CHECK-NEXT:  %13 = CreateFunctionInst %test_hex_printable()
// CHECK-NEXT:  %14 = StorePropertyLooseInst %13 : closure, globalObject : object, "test_hex_printable" : string
// CHECK-NEXT:  %15 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %16 = StoreStackInst undefined : undefined, %15
// CHECK-NEXT:  %17 = LoadStackInst %15
// CHECK-NEXT:  %18 = ReturnInst %17
// CHECK-NEXT:function_end

// CHECK:function test_newline()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, "A string with a newline\\n" : string
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test_quote()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, "A string with a newline\\\"" : string
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test_slash()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, "A string with a newline\\\\" : string
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test_hex()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, "A string with a hex: \\x03" : string
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test_hex_printable()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, "A string with a hex printable: a" : string
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
