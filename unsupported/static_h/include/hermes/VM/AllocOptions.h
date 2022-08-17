/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_ALLOCOPTIONS_H
#define HERMES_VM_ALLOCOPTIONS_H

namespace hermes {
namespace vm {

/// Template parameter passed in during allocation, that signifies whether or
/// not the cell being allocated has a finalizer. Some GC implementations can
/// use this information to speed up finalizer handling.
enum class HasFinalizer { No = 0, Yes };

/// Template parameter passed in during allocation, that signifies whether or
/// not the cell being allocated should be allocated directly in OG.
enum class LongLived { No = 0, Yes };

} // namespace vm
} // namespace hermes

#endif
