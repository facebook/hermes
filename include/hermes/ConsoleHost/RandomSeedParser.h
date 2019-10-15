/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_CONSOLEHOST_MEMORYSIZEPARSER_H
#define HERMES_CONSOLEHOST_MEMORYSIZEPARSER_H

#include "hermes/Public/GCConfig.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/CommandLine.h"

#include <string>

namespace cl {

using llvm::cl::Option;
using llvm::cl::parser;

// Define a custom parser for large integers. LLVM does not support parsing
// large ints.
struct RandomSeedParser : public parser<int64_t> {
  RandomSeedParser(Option &O) : parser<int64_t>(O) {}

  // parse - Return true on error.
  bool parse(
      cl::Option &O,
      llvm::StringRef ArgName,
      const std::string &ArgValue,
      int64_t &Val);
};

} // namespace cl

#endif // HERMES_CONSOLEHOST_MEMORYSIZEPARSER_H
