/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <dlfcn.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <new>

static void *origMalloc(unsigned sz) {
  static void *(*orig)(size_t) = 0;
  if (!orig) {
    orig = (void *(*)(size_t))dlsym(RTLD_NEXT, "malloc");
  }
  void *ret = orig(sz);
  return ret;
}

extern "C" void *malloc(size_t sz) {
  static char *level = 0;
  static char *end = 0;

  // Align the allocation to 16 bytes.
  sz = (sz + 15) & ~15;

  for (;;) {
    if (end - level > sz) {
      char *ret = level;
      level += sz;
      return ret;
    }

    static constexpr size_t blockSize = 1024 * 1024;
    if (sz > blockSize)
      return origMalloc(sz);

    level = (char *)origMalloc(blockSize);
    end = level + blockSize;
  }
}

extern "C" void *realloc(void *p, size_t sz) {
  return malloc(sz);
}

extern "C" void *calloc(size_t n, size_t sz) {
  void *p = malloc(n * sz);
  return memset(p, 0, sz);
}

extern "C" void free(void *f) {}

void *operator new(std::size_t size) {
  return malloc(size);
}

void *operator new[](std::size_t size) {
  return malloc(size);
}

void *operator new(std::size_t size, const std::nothrow_t &) noexcept {
  return malloc(size);
}

void *operator new[](std::size_t size, const std::nothrow_t &) noexcept {
  return malloc(size);
}

void operator delete(void *ptr) noexcept {}

void operator delete[](void *ptr) noexcept {}

void operator delete(void *ptr, const std::nothrow_t &) noexcept {}

void operator delete[](void *ptr, const std::nothrow_t &) noexcept {}
