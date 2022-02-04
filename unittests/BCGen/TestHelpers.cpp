/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "TestHelpers.h"
#include "hermes/AST/SemValidate.h"
#include "hermes/BCGen/HBC/HBC.h"
#include "hermes/BCGen/HBC/TraverseLiteralStrings.h"
#include "hermes/BCGen/HBC/UniquingStringLiteralTable.h"
#include "hermes/IRGen/IRGen.h"
#include "hermes/Parser/JSParser.h"
#include "hermes/Utils/Options.h"
#include "llvh/Support/SHA1.h"

#include "gtest/gtest.h"

using namespace hermes;
using namespace hermes::hbc;

/// Compile source code \p source into Hermes bytecode.
/// \return the bytecode as a vector of bytes.
std::vector<uint8_t> hermes::bytecodeForSource(
    const char *source,
    TestCompileFlags flags) {
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
  llvh::SmallVector<char, 0> bytecodeVector;
  llvh::raw_svector_ostream OS(bytecodeVector);
  BytecodeSerializer BS{OS, bytecodeGenOpts};
  BS.serialize(
      *BM,
      llvh::SHA1::hash(llvh::ArrayRef<uint8_t>{
          reinterpret_cast<const uint8_t *>(source), strlen(source)}));
  return std::vector<uint8_t>{bytecodeVector.begin(), bytecodeVector.end()};
}
