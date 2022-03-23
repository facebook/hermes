/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -funsafe-intrinsics -O0 %s | %FileCheck --match-full-lines %s
// REQUIRES: run_wasm

// Setup Asm.js/Wasm memory
var Math_imul = Math.imul;

var buffer = new ArrayBuffer(131072);
var HEAP8 = new Int8Array(buffer);
var HEAPU8 = new Uint8Array(buffer);

var test_buffer = new ArrayBuffer(131072);
var test_HEAP8 = new Int8Array(test_buffer);
var test_HEAPU8 = new Uint8Array(test_buffer);

// Populate
for (let i = 0; i < 1024; i++) {
  HEAPU8[3 * i] = i % 256;
  HEAPU8[3 * i + 1] = i * (i + 1) % 256;
  HEAPU8[3 * i + 2] = i * (i % 5) % 256;
  test_HEAPU8[3 * i] = i % 256;
  test_HEAPU8[3 * i + 1] = i * (i + 1) % 256;
  test_HEAPU8[3 * i + 2] = i * (i % 5) % 256;
}

// C code
// void rgb_to_gray(uint8_t *image, uint8_t* output, uint32_t size) {
//   for (uint32_t i = 0; i < size; i++) {
//     uint32_t R = image[3*i];
//     uint32_t G = image[3*i+1];
//     uint32_t B = image[3*i+2];
//     uint32_t Gray = ((299 * R + 587 * G + 114 * B) / 1000);
//     output[i] = (uint8_t)Gray;
//   }
//   return;
// }

// Golden
function $1($0_1, $1_1, $2) {
  $0_1 = $0_1 | 0;
    $1_1 = $1_1 | 0;
    $2 = $2 | 0;
    if ($2) {
      label$2 : while (1) {
        HEAP8[$1_1 >> 0] = (((Math_imul(HEAPU8[($0_1 + 1 | 0) >> 0] | 0, 587) + Math_imul(HEAPU8[$0_1 >> 0] | 0, 299) | 0) + Math_imul(HEAPU8[($0_1 + 2 | 0) >> 0] | 0, 114) | 0) >>> 0) / (1e3 >>> 0) | 0;
        $0_1 = $0_1 + 3 | 0;
        $1_1 = $1_1 + 1 | 0;
        $2 = $2 - 1 | 0;
        if ($2) {
          continue label$2
        }
        break label$2;
      }
  }
}

// Intrinsics
function gray(image, output, size) {
  if (size) {
    label: while (1) {
      // R = __uasm.loadu8(test_HEAPU8, image);
      // G = __uasm.loadu8(test_HEAPU8, __uasm.add32(image, 1));
      // B = __uasm.loadu8(test_HEAPU8, __uasm.add32(image, 2));
      // sum = __uasm.add32(__uasm.add32(__uasm.mul32(299, R), __uasm.mul32(587, G)), __uasm.mul32(114, B));
      // Res = __uasm.divu32(sum, 1000);
      __uasm.store8(test_HEAPU8, output, __uasm.divu32(__uasm.add32(__uasm.add32(__uasm.mul32(299, __uasm.loadu8(test_HEAPU8, image)), __uasm.mul32(587, __uasm.loadu8(test_HEAPU8, __uasm.add32(image, 1)))), __uasm.mul32(114, __uasm.loadu8(test_HEAPU8, __uasm.add32(image, 2)))), 1000));
      image = __uasm.add32(image, 3);
      output = __uasm.add32(output, 1);
      size = __uasm.sub32(size, 1);
      if (size) {
        continue label;
      }
      break label;
    }
  }
}

// Run golden
$1(0, 3072, 1024);

// Run ours
gray(0, 3072, 1024);

// Cross check
var correct = 0;
for (let i = 0; i < 1024; i++) {
  if (HEAPU8[3072+i] == test_HEAPU8[3072+i]) {
    correct = correct + 1;
  }
}
print(correct === 1024);
//CHECK: true
