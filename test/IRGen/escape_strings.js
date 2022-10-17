/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

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

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [test_newline, test_quote, test_slash, test_hex, test_hex_printable]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %test_newline#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "test_newline" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %test_quote#0#1()#3, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "test_quote" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %test_slash#0#1()#4, %0
// CHECK-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "test_slash" : string
// CHECK-NEXT:  %7 = CreateFunctionInst %test_hex#0#1()#5, %0
// CHECK-NEXT:  %8 = StorePropertyInst %7 : closure, globalObject : object, "test_hex" : string
// CHECK-NEXT:  %9 = CreateFunctionInst %test_hex_printable#0#1()#6, %0
// CHECK-NEXT:  %10 = StorePropertyInst %9 : closure, globalObject : object, "test_hex_printable" : string
// CHECK-NEXT:  %11 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %12 = StoreStackInst undefined : undefined, %11
// CHECK-NEXT:  %13 = LoadStackInst %11
// CHECK-NEXT:  %14 = ReturnInst %13
// CHECK-NEXT:function_end

// CHECK:function test_newline#0#1()#2
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{test_newline#0#1()#2}
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %2 = CallInst %1, undefined : undefined, "A string with a newline\\n" : string
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test_quote#0#1()#3
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{test_quote#0#1()#3}
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %2 = CallInst %1, undefined : undefined, "A string with a newline\\\"" : string
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test_slash#0#1()#4
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{test_slash#0#1()#4}
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %2 = CallInst %1, undefined : undefined, "A string with a newline\\\\" : string
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test_hex#0#1()#5
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{test_hex#0#1()#5}
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %2 = CallInst %1, undefined : undefined, "A string with a hex: \\x03" : string
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test_hex_printable#0#1()#6
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{test_hex_printable#0#1()#6}
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %2 = CallInst %1, undefined : undefined, "A string with a hex printable: a" : string
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
