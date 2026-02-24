/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/MemorySizeParser.h"

namespace hermes::cli {

/// Validate that \p size is always within the range of gcheapsize_t. And when
/// compressed pointers are enabled, it must be less than 2^32.
static bool validateSize([[maybe_unused]] uint64_t size) {
#ifdef HERMESVM_COMPRESSED_POINTERS
  if (size >= (1ULL << 32))
    return false;
#endif
  // Memory size can't go beyond the maximum of gcheapsize_t.
  return size <= std::numeric_limits<size_t>::max();
}

bool MemorySizeParser::parse(
    llvh::cl::Option &O,
    llvh::StringRef ArgName,
    const std::string &Arg,
    MemorySize &Val) {
  const char *ArgStart = Arg.c_str();
  char *End;

  // Parse integer part, leaving 'End' pointing to the first non-integer char
  Val.bytes = strtoull(ArgStart, &End, 0);

  if (End == ArgStart) {
    return O.error("'" + Arg + "' value invalid for file size argument!");
  }

  if (!validateSize(Val.bytes))
    return O.error("Memory size is too large: " + std::to_string(Val.bytes));

  enum ParserState {
    SawNum,
    SawPrefix,
    SawPrefixPlusI,
    SawWholeSpec,
    Error,
  };

  ParserState state = SawNum;

  while (1) {
    if (state == Error) {
      return O.error("'" + Arg + "' value invalid for file size argument!");
    }
    char c = *End++;
    switch (c) {
      case 0:
        if (state == SawPrefixPlusI) {
          state = Error;
        } else {
          if (!validateSize(Val.bytes))
            return O.error(
                "Memory size is too large: " + std::to_string(Val.bytes));
          return false; // No error
        }
        break;
      case 'i': // Ignore the 'i' in KiB if people use that
        if (state == SawPrefix) {
          state = SawPrefixPlusI;
        } else {
          state = Error;
        }
        break;
      case 'b':
      case 'B': // Ignore B suffix
        if (state == SawWholeSpec) {
          state = Error;
        } else {
          state = SawWholeSpec;
        }
        break;

      case 'g':
      case 'G':
      case 'm':
      case 'M':
      case 'k':
      case 'K':
        if (state != SawNum) {
          state = Error;
        } else {
          switch (c) {
            case 'g':
            case 'G':
              Val.bytes *= 1024 * 1024 * 1024;
              break;
            case 'm':
            case 'M':
              Val.bytes *= 1024 * 1024;
              break;
            case 'k':
            case 'K':
              Val.bytes *= 1024;
              break;
          }
          state = SawPrefix;
        }
        break;

      default:
        // Print an error message if unrecognized character!
        return O.error("'" + Arg + "' value invalid for file size argument!");
    }
  }
}

} // namespace hermes::cli
