/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_JIT_DISCOVERBB_H
#define HERMES_VM_JIT_DISCOVERBB_H

#include "llvh/ADT/DenseMap.h"

#include <vector>

namespace hermes {
namespace vm {

class CodeBlock;

/// Disassemble a CodeBlock and discover all basic blocks.
/// \param the CodeBlock
/// \param[out] basicBlocks on output it will contain the starting offset of
///     every basic block in order. The last entry is the end of the bytecode.
/// \param[out] labels Map from a bytecode target label offset to a basic block
///     index.
void discoverBasicBlocks(
    CodeBlock *codeBlock,
    std::vector<uint32_t> &basicBlocks,
    llvh::DenseMap<uint32_t, unsigned> &labels);

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_JIT_DISCOVERBB_H
