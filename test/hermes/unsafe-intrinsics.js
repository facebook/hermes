/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -funsafe-intrinsics -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -funsafe-intrinsics -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s
// RUN: %hermes -funsafe-intrinsics -O -dump-bytecode %s | %FileCheck --check-prefix=CHKBC %s

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
