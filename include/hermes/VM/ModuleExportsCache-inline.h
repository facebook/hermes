/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_MODULEEXPORTSCACHE_INLINE_H
#define HERMES_MODULEEXPORTSCACHE_INLINE_H

#include "hermes/VM/ModuleExportsCache.h"

namespace hermes::vm {

HermesValue module_export_cache::get(
    const ArrayStorage *exports,
    uint32_t modIndex) {
  // If \p exports is allocated, and \p modIndex is in range, return the
  // previously cached value.  Otherwise return empty to indicate a cache miss.
  // If moduleExports_ is null, it will be allocated in the cache-miss slow
  // path.  If is allocated, but we fail to expand the moduleExports_ cache,
  // indices out of range will take the slow path.
  if (LLVM_LIKELY(exports && modIndex < exports->size())) {
    return exports->at(modIndex);
  } else {
    return HermesValue::encodeEmptyValue();
  }
}

} // namespace hermes::vm

#endif // HERMES_MODULEEXPORTSCACHE_INLINE_H
