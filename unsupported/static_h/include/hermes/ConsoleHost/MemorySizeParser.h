/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_MEMORYSIZEPARSER_H
#define HERMES_SUPPORT_MEMORYSIZEPARSER_H

#include "hermes/Public/GCConfig.h"

#include "llvh/ADT/StringRef.h"
#include "llvh/Support/CommandLine.h"

#include <string>

/// This file defines flags that are specific to the Hermes runtime,
/// independent of the Hermes compiler.  Such flags must be defined in the
/// hermes and hvm binaries, so this file is included in all of those.

namespace cl {

using llvh::cl::Option;
using llvh::cl::parser;

struct MemorySize {
  hermes::vm::gcheapsize_t bytes;
};

// Define a custom parser for memory size specifications.
struct MemorySizeParser : public parser<MemorySize> {
  MemorySizeParser(Option &O) : parser<MemorySize>(O) {}

  // parse - Return true on error.
  bool parse(
      cl::Option &O,
      llvh::StringRef ArgName,
      const std::string &ArgValue,
      MemorySize &Val);
};

} // namespace cl

#endif // HERMES_SUPPORT_MEMORYSIZEPARSER_H
