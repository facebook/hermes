/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "SegmentTestCompile.h"

#include "hermes/AST/CommonJS.h"
#include "hermes/AST/Context.h"
#include "hermes/AST/SemValidate.h"
#include "hermes/BCGen/HBC/BytecodeDisassembler.h"
#include "hermes/BCGen/HBC/HBC.h"
#include "hermes/IR/IR.h"
#include "hermes/IRGen/IRGen.h"
#include "hermes/Parser/JSParser.h"
#include "hermes/Support/Algorithms.h"

#include "llvh/ADT/StringRef.h"
#include "llvh/Support/MemoryBuffer.h"

namespace hermes {

std::pair<std::string, std::string> genSplitCode(
    std::string &mainCode,
    std::string &segmentCode) {
  std::vector<uint32_t> segments{
      0,
      1,
  };

  hermes::CodeGenerationSettings codeGenOpts;
  hermes::OptimizationSettings optimizationOpts;

  auto context = std::make_shared<hermes::Context>(
      codeGenOpts, optimizationOpts, nullptr, segments);
  context->setUseCJSModules(true);

  hermes::Module M(context);
  hermes::sem::SemContext semCtx{};

  auto parseJS = [&](std::unique_ptr<llvh::MemoryBuffer> fileBuf,
                     bool wrapCJSModule = false) -> hermes::ESTree::NodePtr {
    ::hermes::parser::JSParser jsParser(*context, std::move(fileBuf));
    auto parsedJs = jsParser.parse();
    hermes::ESTree::NodePtr parsedAST = parsedJs.getValue();
    if (wrapCJSModule) {
      parsedAST = hermes::wrapCJSModule(
          context, cast<hermes::ESTree::ProgramNode>(parsedAST));
    }
    validateAST(*context, semCtx, parsedAST);
    return parsedAST;
  };

  ::hermes::DeclarationFileListTy declFileList;

  auto globalMemBuffer = llvh::MemoryBuffer::getMemBufferCopy("", "<global>");
  auto *globalAST = parseJS(std::move(globalMemBuffer));
  generateIRFromESTree(globalAST, &M, declFileList, {});
  auto *topLevelFunction = M.getTopLevelFunction();

  auto genModule = [&](uint32_t segmentID,
                       uint32_t id,
                       std::unique_ptr<llvh::MemoryBuffer> fileBuf) {
    llvh::StringRef filename = fileBuf->getBufferIdentifier();
    auto *ast = parseJS(std::move(fileBuf), true);
    generateIRForCJSModule(
        cast<hermes::ESTree::FunctionExpressionNode>(ast),
        segmentID,
        id,
        filename,
        &M,
        topLevelFunction,
        declFileList);
  };

  genModule(
      segments[0],
      0,
      llvh::MemoryBuffer::getMemBufferCopy(mainCode, "main.js"));
  genModule(
      segments[1],
      1,
      llvh::MemoryBuffer::getMemBufferCopy(segmentCode, "foo.js"));

  hermes::BytecodeGenerationOptions genOpts{hermes::EmitBundle};
  hermes::SHA1 sourceHash;

  auto genBC = [&](uint32_t segment) {
    std::string bc{};
    llvh::raw_string_ostream os{bc};
    hermes::hbc::generateBytecode(
        &M, os, genOpts, sourceHash, segment, nullptr, nullptr);
    return os.str();
  };

  return std::make_pair(genBC(segments[0]), genBC(segments[1]));
}

} // namespace hermes
