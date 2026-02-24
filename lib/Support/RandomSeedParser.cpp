/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/RandomSeedParser.h"

namespace hermes::cli {

bool RandomSeedParser::parse(
    llvh::cl::Option &O,
    llvh::StringRef ArgName,
    const std::string &Arg,
    int64_t &Val) {
  const char *ArgStart = Arg.c_str();
  char *End;

  Val = strtol(ArgStart, &End, 0);

  if (End == ArgStart) {
    return O.error("'" + Arg + "' value invalid for random seed argument!");
  }
  // No error is false.
  return false;
}

} // namespace hermes::cli
