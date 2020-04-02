/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_MEMORYSIZEPARSER_H
#define HERMES_SUPPORT_MEMORYSIZEPARSER_H

#include "hermes/Public/GCConfig.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/CommandLine.h"

#include <string>

/// This file defines flags that are specific to the Hermes runtime,
/// independent of the Hermes compiler.  Such flags must be defined in the
/// hermes, hvm, and repl binaries, so this file is included in all of those.

namespace cl {

using llvm::cl::Option;
using llvm::cl::parser;

struct MemorySize {
  hermes::vm::gcheapsize_t bytes;
};

// Define a custom parser for memory size specifications.
struct MemorySizeParser : public parser<MemorySize> {
  MemorySizeParser(Option &O) : parser<MemorySize>(O) {}

  // parse - Return true on error.
  bool parse(
      cl::Option &O,
      llvm::StringRef ArgName,
      const std::string &ArgValue,
      MemorySize &Val);
};

} // namespace cl

#endif // HERMES_SUPPORT_MEMORYSIZEPARSER_H
