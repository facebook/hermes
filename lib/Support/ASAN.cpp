/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/Support/ASAN.h"

#ifdef ASAN_ENABLED
#include <sanitizer/asan_interface.h>
#endif

namespace hermes {

#ifdef ASAN_ENABLED
void asan_poison_if_enabled(void *start, void *end) {
  ASAN_POISON_MEMORY_REGION(
      start,
      reinterpret_cast<uintptr_t>(end) - reinterpret_cast<uintptr_t>(start));
}

void asan_unpoison_if_enabled(void *start, void *end) {
  ASAN_UNPOISON_MEMORY_REGION(
      start,
      reinterpret_cast<uintptr_t>(end) - reinterpret_cast<uintptr_t>(start));
}
#endif

} // namespace hermes
