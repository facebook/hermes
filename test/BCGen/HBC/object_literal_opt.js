/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-bytecode -pretty-disassemble=false -fno-inline -O %s | %FileCheckOrRegen --match-full-lines %s

function foo(p) {
  var obj = {a: 0, b: 1};
  return obj;
}

// Check that storing the global object in an object literal generates a
// placeholder. This is a special case because the global object is treated
// as a Literal inside the compiler.
(function () {
  var o2 = {
    "foo": "fail",
    "hello": this,
    "x" : 5
  };
  return o2;
}());

// Auto-generated content below. Please do not modify manually.

// CHECK:Bytecode File Information:
// CHECK-NEXT:  Bytecode version number: {{.*}}
// CHECK-NEXT:  Source hash: {{.*}}
// CHECK-NEXT:  Function count: 3
// CHECK-NEXT:  String count: 8
// CHECK-NEXT:  BigInt count: 0
// CHECK-NEXT:  String Kind Entry count: 2
// CHECK-NEXT:  RegExp count: 0
// CHECK-NEXT:  Segment ID: 0
// CHECK-NEXT:  CommonJS module count: 0
// CHECK-NEXT:  CommonJS module count (static): 0
// CHECK-NEXT:  Function source count: 0
// CHECK-NEXT:  Bytecode options:
// CHECK-NEXT:    staticBuiltins: 0
// CHECK-NEXT:    cjsModulesStaticallyResolved: 0

// CHECK:Global String Table:
// CHECK-NEXT:s0[ASCII, 0..-1]:
// CHECK-NEXT:s1[ASCII, 0..3]: fail
// CHECK-NEXT:s2[ASCII, 4..9]: global
// CHECK-NEXT:i3[ASCII, 8..8] #00018270: a
// CHECK-NEXT:i4[ASCII, 10..10] #00018E43: b
// CHECK-NEXT:i5[ASCII, 11..13] #9290584E: foo
// CHECK-NEXT:i6[ASCII, 14..18] #C6C55603: hello
// CHECK-NEXT:i7[ASCII, 19..19] #0001E7F9: x

// CHECK:Object Key Buffer:
// CHECK-NEXT:[String 3]
// CHECK-NEXT:[String 4]
// CHECK-NEXT:[String 5]
// CHECK-NEXT:[String 6]
// CHECK-NEXT:[String 7]
// CHECK-NEXT:Object Value Buffer:
// CHECK-NEXT:[int 0]
// CHECK-NEXT:[int 1]
// CHECK-NEXT:[String 1]
// CHECK-NEXT:null
// CHECK-NEXT:[int 5]
// CHECK-NEXT:Function<global>(1 params, 11 registers, 0 symbols):
// CHECK-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// CHECK-NEXT:[@ 0] CreateFunctionEnvironment 0<Reg8>
// CHECK-NEXT:[@ 2] DeclareGlobalVar 5<UInt32>
// CHECK-NEXT:[@ 7] CreateClosure 2<Reg8>, 0<Reg8>, 1<UInt16>
// CHECK-NEXT:[@ 12] GetGlobalObject 1<Reg8>
// CHECK-NEXT:[@ 14] PutByIdLoose 1<Reg8>, 2<Reg8>, 1<UInt8>, 5<UInt16>
// CHECK-NEXT:[@ 20] CreateClosure 1<Reg8>, 0<Reg8>, 2<UInt16>
// CHECK-NEXT:[@ 25] LoadConstUndefined 0<Reg8>
// CHECK-NEXT:[@ 27] Call1 0<Reg8>, 1<Reg8>, 0<Reg8>
// CHECK-NEXT:[@ 31] Ret 0<Reg8>

// CHECK:Function<foo>(2 params, 1 registers, 0 symbols):
// CHECK-NEXT:[@ 0] NewObjectWithBuffer 0<Reg8>, 2<UInt16>, 2<UInt16>, 0<UInt16>, 0<UInt16>
// CHECK-NEXT:[@ 10] Ret 0<Reg8>

// CHECK:Function<>(1 params, 2 registers, 0 symbols):
// CHECK-NEXT:Offset in debug table: source 0x000d, lexical 0x0000
// CHECK-NEXT:[@ 0] NewObjectWithBuffer 0<Reg8>, 3<UInt16>, 3<UInt16>, 3<UInt16>, 9<UInt16>
// CHECK-NEXT:[@ 10] LoadThisNS 1<Reg8>
// CHECK-NEXT:[@ 12] PutByIdLoose 0<Reg8>, 1<Reg8>, 1<UInt8>, 6<UInt16>
// CHECK-NEXT:[@ 18] Ret 0<Reg8>

// CHECK:Debug filename table:
// CHECK-NEXT:  0: {{.*}}object_literal_opt.js

// CHECK:Debug file table:
// CHECK-NEXT:  source table offset 0x0000: filename id 0

// CHECK:Debug source table:
// CHECK-NEXT:  0x0000  function idx 0, starts at line 10 col 1
// CHECK-NEXT:    bc 2: line 10 col 1
// CHECK-NEXT:    bc 14: line 10 col 1
// CHECK-NEXT:    bc 27: line 25 col 2
// CHECK-NEXT:  0x000d  function idx 2, starts at line 18 col 2
// CHECK-NEXT:    bc 12: line 19 col 12
// CHECK-NEXT:  0x0014  end of debug source table

// CHECK:Debug lexical table:
// CHECK-NEXT:  0x0000  lexical parent: none, variable count: 0
// CHECK-NEXT:  0x0002  end of debug lexical table
