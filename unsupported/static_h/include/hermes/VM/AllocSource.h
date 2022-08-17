/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_ALLOC_SOURCE_H
#define HERMES_VM_ALLOC_SOURCE_H

namespace hermes {
namespace vm {

/// Tag indicating where to allocate backing storage.
enum class AllocSource {
  /// Allocate memory using \p oscompat::vm_allocate.
  VMAllocate,

  /// Allocate memory using \p malloc.
  Malloc
};

} // namespace vm
} // namespace hermes
#endif //  HERMES_VM_ALLOC_SOURCE_H
