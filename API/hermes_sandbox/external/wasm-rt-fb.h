/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef WASM_RT_FB_H
#define WASM_RT_FB_H

#include "wasm-rt-impl.h"

#if __STDC_VERSION__ < 201112L || __STDC_NO_ATOMICS__ == 1
typedef unsigned int seed_t;
#else
#include <stdatomic.h>
typedef atomic_uint seed_t;
#endif

#if defined(__linux__) || defined(__ANDROID__)
#include <sys/random.h>
#include <sys/syscall.h>
#include <unistd.h>
#elif defined(__APPLE__)
#include <time.h>
#include <unistd.h>
#endif

#if (defined(__linux__) || defined(__ANDROID__)) && defined(__aarch64__)
/* On Linux on ARM64 we most likely have at least 39 bits of virtual address
 * space https://github.com/torvalds/linux/blob/v6.7/arch/arm64/Kconfig#L1262 If
 * our mmap hint is above 2**39 it will likely fail. */
#define MAX_ADDR_HINT 0x37FFFFFFFF
#elif defined(__APPLE__) && defined(__aarch64__)
/* On ios/arm64 assume we have at least 39 bits of virtual address space  (
 * similar to linux on arm64). This should be true for all iOS versions >=14
 * (https://github.com/golang/go/issues/46860), older versions <14 are
 * unsupported. Note that the effective addressable space might vary, depending
 * on apps entitelmnets as well as various other factors, hence we go for a
 * conservative 39 bit address space limit, which is sufficient for most
 * applications and should be good enough for this purpose.
 */
#define MAX_ADDR_HINT 0x37FFFFFFFF
#elif (defined(__linux__) || defined(__ANDROID__)) && defined(__amd64__)
#define MAX_ADDR_HINT 0x3FFFFFFFFFFF
#elif defined(_WIN64)
/* On Windows use a 37 bit address space limit as this is the lowest
 * configuration for Windows Home
 * https://learn.microsoft.com/en-us/windows/win32/memory/memory-limits-for-windows-releases
 */
#define MAX_ADDR_HINT 0x1FFFFFFFFF
#else
/* For other non-explicitly listed configuration, be extra conservative and use
 * a 32 bit address space limit. */
#define MAX_ADDR_HINT 0xFFFFFFFF
#endif

/*
 * Generates a random 32-bit unsigned integer using the most appropriate method
 available on the current platform.
 *
 * On Linux and Android, it uses the getrandom() system call to generate
 * cryptographically secure random numbers. If getrandom() is not available or
 * fails, it falls back to rand_r().
 * On Windows, it uses the rand_s() function to generate cryptographically
 * secure random numbers.
 *
 * Returns a random 32-bit unsigned integer.
*/
uint32_t rand_u32() {
  uint32_t val;
  int ret;
#if defined(__ANDROID__) && __ANDROID_API__ < 28
  ret =
      syscall(__NR_getrandom, &val, sizeof(val), GRND_NONBLOCK) != sizeof(val);
#elif defined(__linux__) || defined(__ANDROID__)
  ret = getrandom(&val, sizeof(val), GRND_NONBLOCK) != sizeof(val);
#elif defined(_WIN32)
  {
    uint32_t val1, val2;
    rand_s(&val1);
    rand_s(&val2);
    val = (uint64_t)(((uint64_t)val2 << 31) ^ (uint64_t)val1);
  }
#else
  /* Fallback to rand_r() (thread-safe variant of rand) which is not
   * cryptographically secure but should be fine for this purpose.
   */
  static seed_t seed = 0;
  if (seed == 0) {
    seed = time(NULL) ^ clock();
  }
  val =
      ((((uint32_t)rand_r(&seed) & 0xFFFF) << 16) |
       ((uint32_t)rand_r(&seed) & 0xFFFF));
#endif
  (void)ret;
  return val;
}

/**
 * Generates a random memory address hint for mmap, masking off the lower bits
 * to align with the system's page size. Returnr a void pointer to the generated
 * memory address hint.
 */
void *get_mmap_hint() {
  uint64_t addr = (uint64_t)(rand_u32());
  if (sizeof(size_t) == 8) {
    addr = (addr << 32) | ((uint64_t)((rand_u32())));
    addr &= MAX_ADDR_HINT;
  }

#if defined(_WIN32)
  SYSTEM_INFO systemInfo;
  GetSystemInfo(&systemInfo);
  uintptr_t pageSize = systemInfo.dwPageSize;
#else
  uintptr_t pageSize = sysconf(_SC_PAGESIZE);
#endif

  void *hint = (void *)((addr) & ~(pageSize - 1));
  return hint;
}
#endif // WASM_RT_FB_H
