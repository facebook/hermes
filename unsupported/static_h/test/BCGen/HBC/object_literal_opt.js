/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-bytecode -pretty-disassemble=false -O %s | %FileCheck --match-full-lines %s

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

//CHECK-LABEL:Global String Table:
//CHECK-NEXT:s0[ASCII, {{.*}}]: fail
//CHECK-NEXT:s1[ASCII, {{.*}}]: global
//CHECK-NEXT:i2[ASCII, {{.*}}] #{{[0-9A-F]+}}: a
//CHECK-NEXT:i3[ASCII, {{.*}}] #{{[0-9A-F]+}}: b
//CHECK-NEXT:i4[ASCII, {{.*}}] #{{[0-9A-F]+}}: foo
//CHECK-NEXT:i5[ASCII, {{.*}}] #{{[0-9A-F]+}}: hello
//CHECK-NEXT:i6[ASCII, {{.*}}] #{{[0-9A-F]+}}: x
//CHECK-LABEL:Object Key Buffer:
//CHECK-NEXT:[String 4]
//CHECK-NEXT:[String 5]
//CHECK-NEXT:[String 6]
//CHECK-NEXT:[String 2]
//CHECK-NEXT:[String 3]
//CHECK-LABEL:Object Value Buffer:
//CHECK-NEXT:[String 0]
//CHECK-NEXT:null
//CHECK-NEXT:[int 5]
//CHECK-NEXT:[int 0]
//CHECK-NEXT:[int 1]

//CHECK-LABEL:Function<global>(1 params, 2 registers, 0 symbols):
//CHECK-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
//CHECK-NEXT:[@ {{.*}}] DeclareGlobalVar 4<UInt32>
//CHECK-NEXT:[@ {{.*}}] CreateEnvironment 0<Reg8>
//CHECK-NEXT:[@ {{.*}}] CreateClosure 0<Reg8>, 0<Reg8>, 1<UInt16>
//CHECK-NEXT:[@ {{.*}}] GetGlobalObject 1<Reg8>
//CHECK-NEXT:[@ {{.*}}] PutById 1<Reg8>, 0<Reg8>, 1<UInt8>, 4<UInt16>
//CHECK-NEXT:[@ {{.*}}] NewObjectWithBuffer 0<Reg8>, 3<UInt16>, 3<UInt16>, 0<UInt16>, 0<UInt16>
//CHECK-NEXT:[@ {{.*}}] PutById 0<Reg8>, 1<Reg8>, 2<UInt8>, 5<UInt16>
//CHECK-NEXT:[@ {{.*}}] Ret 0<Reg8>

//CHECK-LABEL:Function<foo>(2 params, 1 registers, 0 symbols):
//CHECK-NEXT:[@ {{.*}}] NewObjectWithBuffer 0<Reg8>, 2<UInt16>, 2<UInt16>, 4<UInt16>, 8<UInt16>
//CHECK-NEXT:[@ {{.*}}] Ret 0<Reg8>
