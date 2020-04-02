/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-bytecode -pretty-disassemble=false -O %s | %FileCheck --match-full-lines %s

function foo(p) {
  var obj = {a: 0, b: 1};
  return obj;
}

//CHECK-LABEL:Object Key Buffer:
//CHECK-NEXT:[String {{.*}}]
//CHECK-NEXT:[String {{.*}}]
//CHECK-LABEL:Object Value Buffer:
//CHECK-NEXT:[int {{.*}}]
//CHECK-NEXT:[int {{.*}}]

//CHECK-LABEL:Function<foo>(2 params, 1 registers, 0 symbols):
//CHECK-NEXT:[@ {{.*}}] NewObjectWithBuffer 0<Reg8>, 2<UInt16>, 2<UInt16>, 0<UInt16>, 0<UInt16>
//CHECK-NEXT:[@ {{.*}}] Ret 0<Reg8>

