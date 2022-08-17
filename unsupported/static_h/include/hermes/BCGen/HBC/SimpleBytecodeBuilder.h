/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_HBC_SIMPLEBYTECODEBUILDER_H
#define HERMES_BCGEN_HBC_SIMPLEBYTECODEBUILDER_H

#include "hermes/BCGen/HBC/BytecodeFileFormat.h"
#include "hermes/BCGen/HBC/BytecodeInstructionGenerator.h"
#include "hermes/Public/Buffer.h"

#include <memory>
#include <vector>

namespace hermes {
namespace hbc {
/// A builder that is able to generate bytecode for very simple functions.
/// This can be used to generate a few special functions needed by the Runtime.
class SimpleBytecodeBuilder {
  /// A wrapper for a few crutial fields needed to represent a function.
  /// We can add more fields to it if things get more complicated, but let's
  /// try to keep it as simple as we can.
  struct SimpleFunction {
    /// Offset of the bytecode. This will be populated later during buffer
    /// generation.
    uint32_t offset{};
    /// Number of parameters including "this".
    uint32_t paramCount;
    /// Size of the frame.
    uint32_t frameSize;
    /// The opcodes.
    std::vector<opcode_atom_t> opcodes;

    SimpleFunction(
        uint32_t paramCount,
        uint32_t frameSize,
        std::vector<opcode_atom_t> &&opcodes)
        : paramCount(paramCount),
          frameSize(frameSize),
          opcodes(std::move(opcodes)) {
      assert(paramCount > 0 && "paramCount must include 'this'");
    }
  };

  /// List of functions in the builder to be concatenated into the final
  /// bytecode buffer.
  std::vector<SimpleFunction> functions_{};

 public:
  /// Add a function to the builder. We only need the \p frameSize and
  /// \p opcodes.
  void addFunction(
      uint32_t paramCount,
      uint32_t frameSize,
      std::vector<opcode_atom_t> &&opcodes) {
    functions_.emplace_back(paramCount, frameSize, std::move(opcodes));
  }

  /// Generate the bytecode buffer given the list of functions in the builder.
  std::unique_ptr<Buffer> generateBytecodeBuffer();
};
} // namespace hbc
} // namespace hermes

#endif // HERMES_BCGEN_HBC_SIMPLEBYTECODEBUILDER_H
