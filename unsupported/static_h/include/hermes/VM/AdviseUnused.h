/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_MARKUNUSED_H
#define HERMES_VM_MARKUNUSED_H

namespace hermes {
namespace vm {

/// Indicates whether or not operations that free up pages should return them to
/// the OS.
enum class AdviseUnused { No = 0, Yes };

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_MARKUNUSED_H
