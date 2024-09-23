/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#if (defined(__GNUC__) && defined(__x86_64__)) || \
    (defined(_MSC_VER) && defined(_M_X64))
// =============================================================================
//
// Clang/GCC/MSVC x86-64
//

#include <emmintrin.h>

static inline int _sh_trunc_f64_to_i32(double x) {
  return _mm_cvttsd_si32(_mm_set_sd(x));
}
static inline unsigned _sh_trunc_f64_to_u32(double x) {
  return _mm_cvttsd_si64(_mm_set_sd(x));
}

#elif (defined(__GNUC__) && defined(__i386__) && defined(__SSE2__)) || \
    (defined(_MSC_VER) && defined(_M_IX86) && defined(_M_IX86_FP) &&   \
     _M_IX86_FP >= 2)
// =============================================================================
//
// Clang/GCC/MSVC x86+SSE2
//

#include <emmintrin.h>

static inline int _sh_trunc_f64_to_i32(double x) {
  return _mm_cvttsd_si32(_mm_set_sd(x));
}

static inline unsigned _sh_trunc_f64_to_u32(double x) {
  __m128d val = _mm_set_sd(x);
  // The truncated x, or 1 << 31 on overflow.
  int truncOrTwo31 = _mm_cvttsd_si32(val);
  // The truncated (x - (1 << 31))
  int truncMinusTwo31 = _mm_cvttsd_si32(_mm_sub_sd(val, _mm_set_sd(1u << 31)));
  // If truncOrTwo31 overflowed, (truncMinusTwo31 | 2**31), otherwise
  // truncOrTwo31
  return (truncMinusTwo31 & (truncOrTwo31 >> 31)) | truncOrTwo31;
}

#elif defined(__GNUC__) && defined(__i386__)
// =============================================================================
//
// Clang/GCC x86+FP
//

static inline int _sh_trunc_f64_to_i32(double x) {
  int result;
  unsigned short control_word, new_control_word;

  // Read the current FPU control word
  asm volatile("fnstcw %0" : "=m"(control_word));

  // Modify the control word to set rounding mode to truncation (round toward
  // zero)
  new_control_word = control_word | 0x0C00;

  // Load the new control word
  asm volatile("fldcw %0" : : "m"(new_control_word));

  // Perform the conversion
  asm volatile(
      "fldl %1\n\t" // Load the double value onto the FPU stack
      "fistpl %0" // Convert the top of the FPU stack to int (truncation)
      : "=m"(result) // Output: store result in memory
      : "m"(x) // Input: the double value in memory
      : "st");

  // Restore the original control word
  asm volatile("fldcw %0" : : "m"(control_word));

  return result;
}

static inline unsigned _sh_trunc_f64_to_u32(double x) {
  unsigned long long result;
  unsigned short control_word, new_control_word;

  // Read the current FPU control word
  asm volatile("fnstcw %0" : "=m"(control_word));

  // Modify the control word to set rounding mode to truncation (round toward
  // zero)
  new_control_word = control_word | 0x0C00;

  // Load the new control word
  asm volatile("fldcw %0" : : "m"(new_control_word));

  // Perform the conversion
  asm volatile(
      "fldl %1\n\t" // Load the double value onto the FPU stack
      "fistpll %0" // Convert the top of the FPU stack to int (truncation)
      : "=m"(result) // Output: store result in memory
      : "m"(x) // Input: the double value in memory
      : "st");

  // Restore the original control word
  asm volatile("fldcw %0" : : "m"(control_word));

  return result;
}

#elif defined(_MSC_VER) && defined(_M_IX86)
// =============================================================================
//
// MSVC x86+FP
//

static inline int _sh_trunc_f64_to_i32(double x) {
  int result;
  unsigned short control_word, new_control_word;

  __asm {
    // Save the current FPU control word
        fnstcw control_word

        // Modify control word: set rounding mode to truncation (11b)
        mov ax, control_word
        or  ax, 0x0C00 // Set bits 10 and 11 to 11 (truncate)
        mov new_control_word, ax

        // Load the modified control word
        fldcw new_control_word

            // Perform the conversion
        fld qword ptr [x] // Load the double value onto the FPU stack
        fistp dword ptr [result] // Convert the top FPU value to int (truncation)

    // Restore the original FPU control word
        fldcw control_word
  }

  return result;
}

static inline unsigned _sh_trunc_f64_to_u32(double x) {
  unsigned __int64 result;
  unsigned short control_word, new_control_word;

  __asm {
    // Save the current FPU control word
        fnstcw control_word

        // Modify control word: set rounding mode to truncation (11b)
        mov ax, control_word
        or  ax, 0x0C00 // Set bits 10 and 11 to 11 (truncate)
        mov new_control_word, ax

        // Load the modified control word
        fldcw new_control_word

            // Perform the conversion
        fld qword ptr [x] // Load the double value onto the FPU stack
        fistp qword ptr [result] // Convert the top FPU value to int (truncation)

    // Restore the original FPU control word
        fldcw control_word
  }

  return result;
}

#elif (defined(__GNUC__) && defined(__aarch64__)) || \
    (defined(_MSC_VER) && defined(_M_ARM64))
// =============================================================================
//
// Clang/GCC/MSVC aarch64
//

#include <arm_neon.h>

#ifdef __clang__
/*
 NOTE: the intrinsics work in Clang, but generate an extra asm instruction.
        fcvtzs	d0, d0
        fmov	w0, s0

 GCC produces what we expect. Not sure if it makes a difference.
 */

static inline int _sh_trunc_f64_to_i32(double x) {
  int result;
  asm volatile("fcvtzs %w0, %d1" // Convert double to int32
               : "=r"(result) // Output: result in a general-purpose register
               : "w"(x) // Input: double in a floating-point register
  );
  return result;
}
static inline unsigned _sh_trunc_f64_to_u32(double x) {
  unsigned result;
  asm volatile("fcvtzu %w0, %d1" // Convert double to uint32
               : "=r"(result) // Output: result in a general-purpose register
               : "w"(x) // Input: double in a floating-point register
  );
  return result;
}
#else
static inline int _sh_trunc_f64_to_i32(double x) {
  return (int)vget_lane_s64(vcvt_s64_f64(vdup_n_f64(x)), 0);
}
static inline unsigned _sh_trunc_f64_to_u32(double x) {
  return (unsigned)vget_lane_u64(vcvt_u64_f64(vdup_n_f64(x)), 0);
}
#endif

#elif defined(__GNUC__) && defined(__ARM_ARCH_7A__) && defined(__VFP_FP__)
// =============================================================================
//
// Clang/GCC armv7-a
//

static inline int _sh_trunc_f64_to_i32(double x) {
  int result;
  asm volatile(
      "vcvt.s32.f64 s0, %P1\n\t" // Convert double to int32 with truncation
      "vmov %0, s0" // Move the lower part of d0 (s0) to the result
      : "=r"(result) // Output: result in a general-purpose register
      : "w"(x) // Input: the double value
      : "d0" // Clobbers: NEON register d0
  );
  return result;
}
static inline unsigned _sh_trunc_f64_to_u32(double x) {
  unsigned result;
  asm volatile(
      "vcvt.u32.f64 s0, %P1\n\t" // Convert double to uint32 with truncation
      "vmov %0, s0" // Move the lower part of d0 (s0) to the result
      : "=r"(result) // Output: result in a general-purpose register
      : "w"(x) // Input: the double value
      : "d0" // Clobbers: NEON register d0
  );
  return result;
}

#else
// =============================================================================
//
// Unsupported compiler/platform
//

#if HERMES_FP_TRUNC_ENABLE_UNSUPPORTED
#include <stdint.h>

#if HERMES_FP_TRUNC_ENABLE_UNSUPPORTED == 1
static inline int32_t _sh_trunc_f64_to_i32(double x) {
  return x < INT32_MIN ? 0 : x > INT32_MAX ? 0 : (int32_t)x;
}
static inline uint32_t _sh_trunc_f64_to_u32(double x) {
  return x < 0 ? 0 : x > UINT32_MAX ? 0 : (uint32_t)x;
}
#else
#include "llvh/Support/Compiler.h"

static inline int32_t _sh_trunc_f64_to_i32(double x)
    LLVM_NO_SANITIZE("float-cast-overflow");
static inline int32_t _sh_trunc_f64_to_i32(double x) {
  return (int32_t)x;
}
static inline uint32_t _sh_trunc_f64_to_u32(double x)
    LLVM_NO_SANITIZE("float-cast-overflow");
static inline uint32_t _sh_trunc_f64_to_u32(double x) {
  return (uint32_t)x;
}
#endif
#else
#error Unsupported compiler platform for _sh_trunc_f64
#endif

#endif
