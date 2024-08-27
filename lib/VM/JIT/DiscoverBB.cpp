/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JIT/DiscoverBB.h"

#define DEBUG_TYPE "jit"

#include "hermes/Inst/InstDecode.h"
#include "hermes/VM/CodeBlock.h"

#include "llvh/Support/Debug.h"

namespace hermes {
namespace vm {
using hermes::inst::Inst;
using hermes::inst::OpCode;
using hermes::inst::OperandType;

void discoverBasicBlocks(
    CodeBlock *codeBlock,
    std::vector<uint32_t> &basicBlocks,
    llvh::DenseMap<uint32_t, unsigned> &labels) {
  auto const begin = codeBlock->begin();
  auto const end = codeBlock->end();

  llvh::DenseSet<uint32_t> labelSet{};

  auto addLabel = [begin, &labelSet](const uint8_t *label) {
    labelSet.insert((uint32_t)(label - begin));
  };

  auto ip = begin;
  // Add the start of the bytecode.
  addLabel(ip);

  while (ip != end) {
    auto decoded = decodeInstruction((const Inst *)ip);
    bool branch = false;
    // FIXME: implement SwitchImm.
    assert(
        decoded.meta.opCode != OpCode::SwitchImm &&
        "SwitchImm not implemented yet");
    if (decoded.meta.opCode == OpCode::Catch) {
      addLabel(ip);
      ip += decoded.meta.size;
      continue;
    }
    for (unsigned i = 0; i < decoded.meta.numOperands; ++i) {
      int32_t offset;
      if (decoded.meta.operandType[i] == OperandType::Addr8 ||
          decoded.meta.operandType[i] == OperandType::Addr32) {
        offset = decoded.operandValue[i].integer;
        // Add the branch destination as a label.
        addLabel(ip + offset);
        branch = true;
      }
    }
    ip += decoded.meta.size;
    // If this was a branch, add the next instruction as a label.
    if (branch)
      addLabel(ip);
  }

  // Add the end of the bytecode
  addLabel(ip);

  // Sort all labels into a sequence of basic blocks.
  basicBlocks.clear();
  basicBlocks.reserve(labelSet.size());
  basicBlocks.insert(basicBlocks.begin(), labelSet.begin(), labelSet.end());
  std::sort(basicBlocks.begin(), basicBlocks.end());

  // Create a mapping from a label to a basic block number.
  LLVM_DEBUG(llvh::outs() << "Discovered Basic Blocks:\n");
  labels.clear();
  labels.reserve(labelSet.size());
  for (unsigned i = 0, size = basicBlocks.size(); i != size; ++i) {
    labels.try_emplace(basicBlocks[i], i);
    LLVM_DEBUG(llvh::outs() << "  BB" << i << " at " << basicBlocks[i] << "\n");
  }
}

} // namespace vm
} // namespace hermes
