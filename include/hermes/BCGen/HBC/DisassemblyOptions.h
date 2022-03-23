/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_HBC_DISASSEMBLYOPTIONS_H
#define HERMES_BCGEN_HBC_DISASSEMBLYOPTIONS_H

namespace hermes {
namespace hbc {

enum class DisassemblyOptions : uint32_t {
  None = 0,
  // Output in pretty mode. See PrettyDisassembleVisitor for what
  // pretty mode does.
  Pretty = 1,
  IncludeSource = 2, // Include source lines in disassembly output.
  IncludeVirtualOffsets = 4, // Include virtual offsets in disassembly output.
  IncludeFunctionIds = 8,
  Objdump = 16, // Output in objdump mode.
  IncludeOpCodeList = 32, // In objdump mode, also list all opcodes.
};

inline constexpr DisassemblyOptions operator|(
    DisassemblyOptions x,
    DisassemblyOptions y) {
  return static_cast<DisassemblyOptions>(
      static_cast<uint32_t>(x) | static_cast<uint32_t>(y));
}

inline constexpr DisassemblyOptions operator&(
    DisassemblyOptions x,
    DisassemblyOptions y) {
  return static_cast<DisassemblyOptions>(
      static_cast<uint32_t>(x) & static_cast<uint32_t>(y));
}

} // namespace hbc
} // namespace hermes

#endif // DISASSEMBLYOPTIONS
