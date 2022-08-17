/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheck %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O


//CHECK-LABEL:function test_newline()
//CHECK-NEXT: frame = []
//CHECK-NEXT: %BB0:
//CHECK-NEXT:   %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHECK-NEXT:   %1 = CallInst %0, undefined : undefined, "A string with a newline\\n" : string
//CHECK-NEXT:   %2 = ReturnInst undefined : undefined
//CHECK-NEXT: function_end
function test_newline() {
  print("A string with a newline\n");
}


//CHECK-LABEL:function test_quote()
//CHECK-NEXT: frame = []
//CHECK-NEXT: %BB0:
//CHECK-NEXT:   %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHECK-NEXT:   %1 = CallInst %0, undefined : undefined, "A string with a newline\\\"" : string
//CHECK-NEXT:   %2 = ReturnInst undefined : undefined
//CHECK-NEXT: function_end
function test_quote() {
  print("A string with a newline\"");
}


//CHECK-LABEL:function test_slash()
//CHECK-NEXT: frame = []
//CHECK-NEXT: %BB0:
//CHECK-NEXT:   %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHECK-NEXT:   %1 = CallInst %0, undefined : undefined, "A string with a newline\\\\" : string
//CHECK-NEXT:   %2 = ReturnInst undefined : undefined
//CHECK-NEXT: function_end
function test_slash() {
  print("A string with a newline\\");
}



//CHECK-LABEL:function test_hex()
//CHECK-NEXT: frame = []
//CHECK-NEXT: %BB0:
//CHECK-NEXT:   %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHECK-NEXT:   %1 = CallInst %0, undefined : undefined, "A string with a hex: \\x03" : string
//CHECK-NEXT:   %2 = ReturnInst undefined : undefined
//CHECK-NEXT: function_end
function test_hex() {
  print("A string with a hex: \x03");
}


//CHECK-LABEL:function test_hex_printable()
//CHECK-NEXT: frame = []
//CHECK-NEXT: %BB0:
//CHECK-NEXT:   %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHECK-NEXT:   %1 = CallInst %0, undefined : undefined, "A string with a hex printable: a" : string
//CHECK-NEXT:   %2 = ReturnInst undefined : undefined
//CHECK-NEXT: function_end
function test_hex_printable() {
  print("A string with a hex printable: \x61");
}

