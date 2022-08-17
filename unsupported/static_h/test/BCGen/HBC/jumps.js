/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-bytecode -pretty-disassemble=false -O %s | %FileCheck --match-full-lines %s

function foo(a) {
  if (a)
    return 1;
  else if (a)
    return 2;
  else if (a)
    return 3;
  else if (a)
    return 4;
  else if (a)
    return 5;
  else if (a)
    return 6;
  else if (a)
    return 7;
  else if (a)
    return 8;
  else if (a)
    return 2;
  else if (a)
    return 3;
  else if (a)
    return 2;
  else if (a)
    return 3;
  else if (a)
    return 4;
  else if (a)
    return 5;
  else if (a)
    return 6;
  else if (a)
    return 7;
  else if (a)
    return 8;
  else if (a)
    return 2;
  else if (a)
    return 3;
  else
    return 9;
}
//CHECK:[@ {{.*}}] LoadParam 0<Reg8>, 1<UInt8>
//CHECK-NEXT:[@ {{.*}}] JmpTrueLong 164<Addr32>, 0<Reg8>
//CHECK-NEXT:[@ {{.*}}] JmpTrueLong 153<Addr32>, 0<Reg8>
//CHECK-NEXT:[@ {{.*}}] JmpTrueLong 142<Addr32>, 0<Reg8>
//CHECK-NEXT:[@ {{.*}}] JmpTrueLong 131<Addr32>, 0<Reg8>
//CHECK-NEXT:[@ {{.*}}] JmpTrue 120<Addr8>, 0<Reg8>
//CHECK-NEXT:[@ {{.*}}] JmpTrue 112<Addr8>, 0<Reg8>
//CHECK-NEXT:[@ {{.*}}] JmpTrue 104<Addr8>, 0<Reg8>
//CHECK-NEXT:[@ {{.*}}] JmpTrue 96<Addr8>, 0<Reg8>
//CHECK-NEXT:[@ {{.*}}] JmpTrue 88<Addr8>, 0<Reg8>
//CHECK-NEXT:[@ {{.*}}] JmpTrue 80<Addr8>, 0<Reg8>
//CHECK-NEXT:[@ {{.*}}] JmpTrue 72<Addr8>, 0<Reg8>
//CHECK-NEXT:[@ {{.*}}] JmpTrue 64<Addr8>, 0<Reg8>
//CHECK-NEXT:[@ {{.*}}] JmpTrue 56<Addr8>, 0<Reg8>
//CHECK-NEXT:[@ {{.*}}] JmpTrue 48<Addr8>, 0<Reg8>
//CHECK-NEXT:[@ {{.*}}] JmpTrue 40<Addr8>, 0<Reg8>
//CHECK-NEXT:[@ {{.*}}] JmpTrue 32<Addr8>, 0<Reg8>
//CHECK-NEXT:[@ {{.*}}] JmpTrue 24<Addr8>, 0<Reg8>
//CHECK-NEXT:[@ {{.*}}] JmpTrue 16<Addr8>, 0<Reg8>
//CHECK-NEXT:[@ {{.*}}] JmpTrue 8<Addr8>, 0<Reg8>
//CHECK-NEXT:[@ {{.*}}] LoadConstUInt8 0<Reg8>, 9<UInt8>
//CHECK-NEXT:[@ {{.*}}] Ret 0<Reg8>
//CHECK-NEXT:[@ {{.*}}] LoadConstUInt8 0<Reg8>, 3<UInt8>
//CHECK-NEXT:[@ {{.*}}] Ret 0<Reg8>
//CHECK-NEXT:[@ {{.*}}] LoadConstUInt8 0<Reg8>, 2<UInt8>
//CHECK-NEXT:[@ {{.*}}] Ret 0<Reg8>
//CHECK-NEXT:[@ {{.*}}] LoadConstUInt8 0<Reg8>, 8<UInt8>
//CHECK-NEXT:[@ {{.*}}] Ret 0<Reg8>
//CHECK-NEXT:[@ {{.*}}] LoadConstUInt8 0<Reg8>, 7<UInt8>
//CHECK-NEXT:[@ {{.*}}] Ret 0<Reg8>
//CHECK-NEXT:[@ {{.*}}] LoadConstUInt8 0<Reg8>, 6<UInt8>
//CHECK-NEXT:[@ {{.*}}] Ret 0<Reg8>
//CHECK-NEXT:[@ {{.*}}] LoadConstUInt8 0<Reg8>, 5<UInt8>
//CHECK-NEXT:[@ {{.*}}] Ret 0<Reg8>
//CHECK-NEXT:[@ {{.*}}] LoadConstUInt8 0<Reg8>, 4<UInt8>
//CHECK-NEXT:[@ {{.*}}] Ret 0<Reg8>
//CHECK-NEXT:[@ {{.*}}] LoadConstUInt8 0<Reg8>, 3<UInt8>
//CHECK-NEXT:[@ {{.*}}] Ret 0<Reg8>
//CHECK-NEXT:[@ {{.*}}] LoadConstUInt8 0<Reg8>, 2<UInt8>
//CHECK-NEXT:[@ {{.*}}] Ret 0<Reg8>
//CHECK-NEXT:[@ {{.*}}] LoadConstUInt8 0<Reg8>, 3<UInt8>
//CHECK-NEXT:[@ {{.*}}] Ret 0<Reg8>
//CHECK-NEXT:[@ {{.*}}] LoadConstUInt8 0<Reg8>, 2<UInt8>
//CHECK-NEXT:[@ {{.*}}] Ret 0<Reg8>
//CHECK-NEXT:[@ {{.*}}] LoadConstUInt8 0<Reg8>, 8<UInt8>
//CHECK-NEXT:[@ {{.*}}] Ret 0<Reg8>
//CHECK-NEXT:[@ {{.*}}] LoadConstUInt8 0<Reg8>, 7<UInt8>
//CHECK-NEXT:[@ {{.*}}] Ret 0<Reg8>
//CHECK-NEXT:[@ {{.*}}] LoadConstUInt8 0<Reg8>, 6<UInt8>
//CHECK-NEXT:[@ {{.*}}] Ret 0<Reg8>
//CHECK-NEXT:[@ {{.*}}] LoadConstUInt8 0<Reg8>, 5<UInt8>
//CHECK-NEXT:[@ {{.*}}] Ret 0<Reg8>
//CHECK-NEXT:[@ {{.*}}] LoadConstUInt8 0<Reg8>, 4<UInt8>
//CHECK-NEXT:[@ {{.*}}] Ret 0<Reg8>
//CHECK-NEXT:[@ {{.*}}] LoadConstUInt8 0<Reg8>, 3<UInt8>
//CHECK-NEXT:[@ {{.*}}] Ret 0<Reg8>
//CHECK-NEXT:[@ {{.*}}] LoadConstUInt8 0<Reg8>, 2<UInt8>
//CHECK-NEXT:[@ {{.*}}] Ret 0<Reg8>
//CHECK-NEXT:[@ {{.*}}] LoadConstUInt8 0<Reg8>, 1<UInt8>
//CHECK-NEXT:[@ {{.*}}] Ret 0<Reg8>
