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

// Setup Asm.js/Wasm memory
var buffer = new ArrayBuffer(131072);
var HEAP8 = new Int8Array(buffer);
var HEAP16 = new Int16Array(buffer);
var HEAP32 = new Int32Array(buffer);
var HEAPU8 = new Uint8Array(buffer);
var HEAPU16 = new Uint16Array(buffer);
var HEAPU32 = new Uint32Array(buffer);
var HEAPF32 = new Float32Array(buffer);
var HEAPF64 = new Float64Array(buffer);

for (let i = 0; i < 256; i++) {
  HEAPU8[i] = i;
}

// Loadi8
print(__uasm.loadi8(HEAP8, 1) === 1);
//CHKBC: Loadi8
//CHECK: true

print(__uasm.loadi8(HEAP8, 255) === -1);
//CHKBC: Loadi8
//CHECK-NEXT: true

// Loadu8
print(__uasm.loadu8(HEAPU8, 1) === 1);
//CHKBC: Loadu8
//CHECK-NEXT: true

print(__uasm.loadu8(HEAPU8, 255) === 255);
//CHKBC: Loadu8
//CHECK-NEXT: true

// Loadi32 -- Asm.js/Wasm uses little-endian.
// 50462976 == 0000 0011 0000 0010 0000 0001 0000 0000
print(__uasm.loadi32(HEAP32, 0) === 50462976);
//CHKBC: Loadi32
//CHECK-NEXT: true

// -66052 == 1111 1111 1111 1110 1111 11101 1111 1100
print(__uasm.loadi32(HEAP32, 252) === -66052);
//CHKBC: Loadi32
//CHECK-NEXT: true

// 4294901244 == 1111 1111 1111 1110 1111 11101 1111 1100
print(__uasm.loadu32(HEAPU32, 252) === -66052);
//CHKBC: Loadu32
//CHECK-NEXT: true

// Store32
__uasm.store32(HEAP32, 256, 42);
print(HEAP32[256 >> 2] === 42);
//CHKBC: Store32
//CHECK-NEXT: true

// Can also handle negative numbers.
__uasm.store32(HEAP32, 256, -1);
print(HEAP32[256 >> 2] === -1);
//CHKBC: Store32
//CHECK-NEXT: true
print(HEAPU32[256 >> 2] === 4294967295);
//CHECK-NEXT: true

// Store8
__uasm.store8(HEAP8, 256, 255);
print(HEAPU8[256] === 255);
//CHKBC: Store8
//CHECK-NEXT: true
print(HEAP8[256] === -1);
//CHECK-NEXT: true

__uasm.store8(HEAP8, 256, -1);
print(HEAPU8[256] === 255);
//CHKBC: Store8
//CHECK-NEXT: true
print(HEAP8[256] === -1);
//CHECK-NEXT: true
