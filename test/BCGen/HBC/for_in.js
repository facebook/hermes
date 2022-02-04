/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-bytecode -pretty-disassemble=false -target=HBC %s -O | %FileCheck %s --match-full-lines
// This checks that compilation without optimization at least doesn't fail hard
// RUN: %hermes -dump-bytecode -target=HBC %s
// This tests disassembleBytecode in CompilerDriver.cpp
// RUN: %hermes -emit-binary -O -target=HBC -out %t %s && %hermes -dump-bytecode -pretty-disassemble=false -b %t | %FileCheck %s --match-full-lines

//CHECK-LABEL:Function<test_one>(3 params, 7 registers, 0 symbols):
//CHECK-NEXT:Offset in debug table: {{.*}}
//CHECK-NEXT:[@ {{.*}}] LoadParam 5<Reg8>, 1<UInt8>
//CHECK-NEXT:[@ {{.*}}] Mov 3<Reg8>, 5<Reg8>
//CHECK-NEXT:[@ {{.*}}] GetPNameList 4<Reg8>, 3<Reg8>, 2<Reg8>, 1<Reg8>
//CHECK-NEXT:[@ {{.*}}] JmpUndefined 21<Addr8>, 4<Reg8>
//CHECK-NEXT:[@ {{.*}}] GetNextPName 0<Reg8>, 4<Reg8>, 3<Reg8>, 2<Reg8>, 1<Reg8>
//CHECK-NEXT:[@ {{.*}}] JmpUndefined 12<Addr8>, 0<Reg8>
//CHECK-NEXT:[@ {{.*}}] Mov 6<Reg8>, 0<Reg8>
//CHECK-NEXT:[@ {{.*}}] GetByVal 6<Reg8>, 5<Reg8>, 6<Reg8>
//CHECK-NEXT:[@ {{.*}}] Jmp -16<Addr8>
//CHECK-NEXT:[@ {{.*}}] LoadConstUndefined 0<Reg8>
//CHECK-NEXT:[@ {{.*}}] Ret 0<Reg8>
function test_one(x, f) {
  for (var v in x) {
    x[v];
  }
}

test_one(1,2,3)

