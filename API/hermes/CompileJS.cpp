/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "CompileJS.h"

#include "hermes/BCGen/HBC/BytecodeProviderFromSrc.h"
#include "hermes/Support/Algorithms.h"

#include "llvm/Support/SHA1.h"

namespace hermes {

bool compileJS(
    const std::string &str,
    const std::string &sourceURL,
    std::string &bytecode,
    bool optimize) {
  hbc::CompileFlags flags{};
  flags.optimize = optimize;

  // Note that we are relying the zero termination provided by str.data(),
  // because the parser requires it.
  auto res = hbc::BCProviderFromSrc::createBCProviderFromSrc(
      hermes::make_unique<hermes::Buffer>(
          (const uint8_t *)str.data(), str.size()),
      sourceURL,
      flags);
  if (!res.first)
    return false;

  llvm::raw_string_ostream bcstream(bytecode);

  BytecodeGenerationOptions opts(::hermes::EmitBundle);
  opts.optimizationEnabled = optimize;

  hbc::BytecodeSerializer BS{bcstream, opts};
  BS.serialize(
      *res.first->getBytecodeModule(),
      llvm::SHA1::hash(llvm::makeArrayRef(
          reinterpret_cast<const uint8_t *>(str.data()), str.size())));

  // Flush to string.
  bcstream.flush();
  return true;
}

bool compileJS(const std::string &str, std::string &bytecode, bool optimize) {
  return compileJS(str, "", bytecode, optimize);
}

} // namespace hermes
