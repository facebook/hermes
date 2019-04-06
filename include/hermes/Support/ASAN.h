/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_SUPPORT_ASAN_H
#define HERMES_SUPPORT_ASAN_H

#include "hermes/Support/Compiler.h"

#if __has_feature(address_sanitizer) || defined(__SANITIZE_ADDRESS__)
#define ASAN_ENABLED
#endif

namespace hermes {

/// Poisons/unpoisons the memory region when run with ASAN on. This is a no-op
/// when ASAN is not enabled.
///
/// A poisoned region cannot be read from or written to, else it'll generate
/// an abort with the stack trace of the illegal read/write.
/// This should not be used as a replacement for ASAN's normal operations with
/// malloc/free, and should only be used to poison memory ranges that are not
/// managed by normal C memory management (for example, in a GC).
void asan_poison_if_enabled(void *start, void *end);
void asan_unpoison_if_enabled(void *start, void *end);

#ifndef ASAN_ENABLED
// If ASAN isn't enabled inline these as no-ops.
inline void asan_poison_if_enabled(void *start, void *end) {}
inline void asan_unpoison_if_enabled(void *start, void *end) {}
#endif

} // namespace hermes

#endif
