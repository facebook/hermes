/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-bytecode -pretty-disassemble=false -O %s | %FileCheck --match-full-lines %s
// Ensure that NewArray instructions don't emit entries in the debug table.

function foo() {
    return [[], [], []];
}

//CHECK-LABEL:Function<foo>(1 params, 2 registers, 0 symbols):
//CHECK-NEXT:Offset in debug table: {{.*}}
//CHECK-NEXT:[@ 0] NewArray 0<Reg8>, 3<UInt16>
//CHECK-NEXT:[@ 4] NewArray 1<Reg8>, 0<UInt16>
//CHECK-NEXT:[@ 8] PutOwnByIndex 0<Reg8>, 1<Reg8>, 0<UInt8>
//CHECK-NEXT:[@ 12] NewArray 1<Reg8>, 0<UInt16>
//CHECK-NEXT:[@ 16] PutOwnByIndex 0<Reg8>, 1<Reg8>, 1<UInt8>
//CHECK-NEXT:[@ 20] NewArray 1<Reg8>, 0<UInt16>
//CHECK-NEXT:[@ 24] PutOwnByIndex 0<Reg8>, 1<Reg8>, 2<UInt8>
//CHECK-NEXT:[@ 28] Ret 0<Reg8>

//CHECK-LABEL:Debug source table:
//CHECK:  0x{{[0-9a-f]+}}  function idx 1, starts at line 11 col 1
//CHECK:  0x{{[0-9a-f]+}}  end of debug source table
