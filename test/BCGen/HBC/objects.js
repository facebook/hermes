/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-bytecode -pretty-disassemble=false -O %s | %FileCheck --match-full-lines %s

function foo(p) {
  var obj = {a: 1};
  obj.a = 1;
  obj[p] = 1;
  obj["b"] = obj.a;
  obj["2"] = obj[p];
  delete obj["b"];
  delete obj[p];
}

//CHECK-LABEL:Function<foo>(2 params, 4 registers, 0 symbols):
//CHECK-NEXT:Offset in debug table: {{.*}}
//CHECK-NEXT:[@ {{.*}}] LoadParam 1<Reg8>, 1<UInt8>
//CHECK-NEXT:[@ {{.*}}] NewObject 0<Reg8>
//CHECK-NEXT:[@ {{.*}}] LoadConstUInt8 2<Reg8>, 1<UInt8>
//CHECK-NEXT:[@ {{.*}}] PutNewOwnByIdShort 0<Reg8>, 2<Reg8>, 1<UInt8>
//CHECK-NEXT:[@ {{.*}}] PutById 0<Reg8>, 2<Reg8>,  1<UInt8>, 1<UInt16>
//CHECK-NEXT:[@ {{.*}}] PutByVal 0<Reg8>, 1<Reg8>, 2<Reg8>
//CHECK-NEXT:[@ {{.*}}] GetByIdShort 2<Reg8>, 0<Reg8>, 1<UInt8>, 1<UInt8>
//CHECK-NEXT:[@ {{.*}}] PutById 0<Reg8>, 2<Reg8>, 2<UInt8>, 2<UInt16>
//CHECK-NEXT:[@ {{.*}}] GetByVal 3<Reg8>, 0<Reg8>, 1<Reg8>
//CHECK-NEXT:[@ {{.*}}] LoadConstUInt8 2<Reg8>, 2<UInt8>
//CHECK-NEXT:[@ {{.*}}] PutByVal 0<Reg8>, 2<Reg8>, 3<Reg8>
//CHECK-NEXT:[@ {{.*}}] DelById 2<Reg8>, 0<Reg8>, 2<UInt16>
//CHECK-NEXT:[@ {{.*}}] DelByVal 0<Reg8>, 0<Reg8>, 1<Reg8>
//CHECK-NEXT:[@ {{.*}}] LoadConstUndefined 0<Reg8>
//CHECK-NEXT:[@ {{.*}}] Ret 0<Reg8>
