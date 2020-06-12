/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/AST/Context.h"
#include "hermes/AST/SemValidate.h"
#include "hermes/BCGen/HBC/HBC.h"
#include "hermes/IRGen/IRGen.h"
#include "hermes/Parser/JSParser.h"

#include <stdint.h>
#include <vector>
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/SHA1.h"

namespace hermes {

struct TestCompileFlags {
  bool staticBuiltins{false};
};

/// Compile source code \p source into Hermes bytecode, asserting that it can be
/// compiled successfully. \return the bytecode as a vector of bytes.
/// Compile source code \p source into Hermes bytecode.
/// \return the bytecode as a vector of bytes.
inline std::vector<uint8_t> bytecodeForSource(
    const char *source,
    TestCompileFlags flags = {}) {
  using namespace hbc;
  /* Parse source */
  SourceErrorManager sm;
  CodeGenerationSettings codeGenOpts;
  codeGenOpts.unlimitedRegisters = false;
  OptimizationSettings optSettings;
  optSettings.staticBuiltins = flags.staticBuiltins;
  auto context = std::make_shared<Context>(sm, codeGenOpts, optSettings);
  parser::JSParser jsParser(*context, source);
  auto parsed = jsParser.parse();
  assert(parsed.hasValue() && "Failed to parse source");
  sem::SemContext semCtx{};
  auto validated = validateAST(*context, semCtx, *parsed);
  (void)validated;
  assert(validated && "Failed to validate source");
  auto *ast = parsed.getValue();

  /* Generate IR */
  Module M(context);
  DeclarationFileListTy declFileList;
  hermes::generateIRFromESTree(ast, &M, declFileList, {});

  /* Generate bytecode module */
  auto bytecodeGenOpts = BytecodeGenerationOptions::defaults();
  bytecodeGenOpts.staticBuiltinsEnabled = flags.staticBuiltins;
  auto BM =
      generateBytecodeModule(&M, M.getTopLevelFunction(), bytecodeGenOpts);
  assert(BM != nullptr && "Failed to generate bytecode module");

  /* Serialize it */
  llvm::SmallVector<char, 0> bytecodeVector;
  llvm::raw_svector_ostream OS(bytecodeVector);
  BytecodeSerializer BS{OS, bytecodeGenOpts};
  BS.serialize(
      *BM,
      llvm::SHA1::hash(llvm::ArrayRef<uint8_t>{
          reinterpret_cast<const uint8_t *>(source), strlen(source)}));
  return std::vector<uint8_t>{bytecodeVector.begin(), bytecodeVector.end()};
}

} // namespace hermes
