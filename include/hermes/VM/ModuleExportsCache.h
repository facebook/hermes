/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_MODULEEXPORTSCACHE_H
#define HERMES_MODULEEXPORTSCACHE_H

#include "hermes/VM/HermesValue.h"
#include "hermes/VM/Runtime.h"

#include <stdint.h>

// Forward declaration.  (ArrayStorage = ArrayStorageBase<HermesValue>, but
// we don't want to do typedefs or using declarations in header files.)
template <typename HVType>
class ArrayStorageBase;

namespace hermes::vm {

namespace module_export_cache {

/// Attempts to set the module export for module \p modIndex to \p modExport.
/// This may fail if \p modIndex is outside current capacity
/// of the cache, and attempts to reallocate it fail.  If that occurs,
/// the array size will remain unchanged.
void set(
    Runtime &runtime,
    ArrayStorageBase<HermesValue> *&exports,
    uint32_t modIndex,
    HermesValue modExport);

/// Returns the module export for module \p modIndex.  This will be
/// empty if that module has not yet been initialized.
inline HermesValue get(
    const ArrayStorageBase<HermesValue> *exports,
    uint32_t modIndex);

} // namespace module_export_cache

} // namespace hermes::vm

#endif // HERMES_MODULEEXPORTSCACHE_H
