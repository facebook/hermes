/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -funsafe-intrinsics -O0 %s | %FileCheck --match-full-lines %s
// RUN: %hermes -funsafe-intrinsics -O0 -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s
// RUN: %hermes -funsafe-intrinsics -O0 -dump-bytecode %s | %FileCheck --check-prefix=CHKBC %s
// REQUIRES: run_wasm

// Check regular case
print(__uasm.add32(19, 23) === (19 + 23));
//CHKBC: Add32
//CHECK: true

// Check overflow
print(__uasm.add32(2147483647, 1) === -2147483648);
//CHKBC: Add32
//CHECK-NEXT: true

print(__uasm.add32(2147483647, 1) === (2147483647 + 1));
//CHKBC: Add32
//CHECK-NEXT: false

print(__uasm.add32(2147483647, 1) === (((2147483647 | 0) + (1 | 0)) | 0));
//CHKBC: Add32
//CHECK-NEXT: true

// Corner: one operand actually overflows
print(__uasm.add32(4294967296, 2) === 2);
//CHKBC: Add32
//CHECK-NEXT: true

print(__uasm.add32(4294967296, 2) === (4294967296 + 2));
//CHKBC: Add32
//CHECK-NEXT: false

print(__uasm.add32(4294967296, 2) === (((4294967296 | 0) + (2 | 0)) | 0));
//CHKBC: Add32
//CHECK-NEXT: true

// Multiplication
print(__uasm.mul32(6, 7) === 42);
//CHKBC: Mul32
//CHECK-NEXT: true

print(__uasm.mul32(6, -7) === -42);
//CHKBC: Mul32
//CHECK-NEXT: true

print(__uasm.mul32(-1, -1) === 1);
//CHKBC: Mul32
//CHECK-NEXT: true

print(__uasm.mul32(1, -1) === -1);
//CHKBC: Mul32
//CHECK-NEXT: true

print(__uasm.mul32(2147483647, 2) === -2);
//CHKBC: Mul32
//CHECK-NEXT: true

// Division
print(__uasm.divi32(42, 7) === 6);
//CHKBC: Divi32
//CHECK-NEXT: true

print(__uasm.divi32(43, -7) === -6);
//CHKBC: Divi32
//CHECK-NEXT: true

print(__uasm.divu32(43, 7) === 6);
//CHKBC: Divu32
//CHECK-NEXT: true

// Signedness matters.
print(__uasm.divi32(-2, 2) === -1);
//CHKBC: Divi32
//CHECK-NEXT: true

print(__uasm.divu32(-2, 2) === 2147483647);
//CHKBC: Divu32
//CHECK-NEXT: true

// divu32 returns a signed number
print(__uasm.divu32(-2, 1) === -2);
//CHKBC: Divu32
//CHECK-NEXT: true
