/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "JSLibInternal.h"

#include "hermes/AST/SemValidate.h"
#include "hermes/BCGen/HBC/BytecodeStream.h"
#include "hermes/BCGen/HBC/HBC.h"
#include "hermes/IR/IR.h"
#include "hermes/IRGen/IRGen.h"
#include "hermes/Parser/JSParser.h"
#include "hermes/Support/SimpleDiagHandler.h"
#include "hermes/Utils/Options.h"
#include "hermes/VM/JSLib.h"
#include "hermes/VM/Operations.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/StringPrimitive.h"
#include "hermes/VM/StringRefUtils.h"
#include "hermes/VM/StringView.h"
#include "llvm/Support/ConvertUTF.h"
#include "llvm/Support/raw_ostream.h"

namespace hermes {
namespace vm {

namespace {
#ifndef HERMESVM_LEAN
bool isSingleFunctionExpression(ESTree::NodePtr ast) {
  auto *prog = dyn_cast<ESTree::ProgramNode>(ast);
  if (!prog) {
    return false;
  }
  ESTree::NodeList &body = prog->_body;
  if (body.size() != 1) {
    return false;
  }
  auto *exprStatement =
      dyn_cast<ESTree::ExpressionStatementNode>(&body.front());
  if (!exprStatement) {
    return false;
  }
  return isa<ESTree::FunctionExpressionNode>(exprStatement->_expression) ||
      isa<ESTree::ArrowFunctionExpressionNode>(exprStatement->_expression);
}
#endif
} // namespace

CallResult<HermesValue> evalInEnvironment(
    Runtime *runtime,
    llvm::StringRef utf8code,
    Handle<Environment> environment,
    const ScopeChain &scopeChain,
    Handle<> thisArg,
    bool singleFunction) {
#ifdef HERMESVM_LEAN
  return runtime->raiseEvalUnsupported(utf8code);
#else
  if (!runtime->enableEval) {
    return runtime->raiseEvalUnsupported(utf8code);
  }
  CodeGenerationSettings codeGenOpts;
  codeGenOpts.unlimitedRegisters = false;
  auto context = std::make_shared<Context>(codeGenOpts);

  SimpleDiagHandlerRAII evalOutputManager{context->getSourceErrorManager()};
  // The spec requires that global eval always start in non-strict mode.
  context->setStrictMode(false);
  context->setEnableEval(true);

  auto reportError = [&, runtime]() {
    const auto &msg = evalOutputManager.getFirstMessage();
    return runtime->raiseSyntaxError(
        TwineChar16(msg.getLineNo()) + ":" + (msg.getColumnNo() + 1) + ":" +
        msg.getMessage());
  };

// Generate full debug info if the debugger is present, otherwise generate
// enough for backtraces.
#ifdef HERMES_ENABLE_DEBUGGER
  context->setDebugInfoSetting(DebugInfoSetting::ALL);
#else
  context->setDebugInfoSetting(DebugInfoSetting::THROWING);
#endif
  sem::SemContext semCtx{};
  hermes::parser::JSParser jsParser(*context, utf8code);
  auto parsed = jsParser.parse();
  if (!parsed || !validateAST(*context, semCtx, *parsed))
    return reportError();

  ESTree::Node *ast = parsed.getValue();

  // Check to see if we're only allowed to have a single function.
  if (singleFunction && !isSingleFunctionExpression(ast))
    return runtime->raiseSyntaxError("Invalid function expression");

  Module M(context);

  DeclarationFileListTy declFileList;
  hermes::generateIRFromESTree(ast, &M, declFileList, scopeChain);

  if (evalOutputManager.haveErrors())
    return reportError();

  auto bytecodeOptions = BytecodeGenerationOptions::defaults();
  bytecodeOptions.verifyIR = runtime->verifyEvalIR;
  auto module =
      hbc::generateBytecodeModule(&M, M.getTopLevelFunction(), bytecodeOptions);
  if (evalOutputManager.haveErrors())
    return reportError();

  auto bytecode =
      hbc::BCProviderFromSrc::createBCProviderFromSrc(std::move(module));

  if (evalOutputManager.haveErrors())
    return reportError();

  // TODO: pass a sourceURL derived from a '//# sourceURL' comment.
  llvm::StringRef sourceURL{};
  return runtime->runBytecode(
      std::move(bytecode),
      RuntimeModuleFlags{},
      sourceURL,
      environment,
      thisArg);
#endif
}

CallResult<HermesValue> directEval(
    Runtime *runtime,
    Handle<StringPrimitive> str,
    const ScopeChain &scopeChain,
    bool singleFunction) {
  // Convert the code into UTF8.
  std::string code;
  auto view = StringPrimitive::createStringView(runtime, str);
  if (view.isASCII()) {
    code = std::string(view.begin(), view.end());
  } else {
    SmallU16String<4> allocator;
    convertUTF16ToUTF8WithReplacements(code, view.getUTF16Ref(allocator));
  }

  return evalInEnvironment(
      runtime,
      code,
      Runtime::makeNullHandle<Environment>(),
      scopeChain,
      runtime->getGlobal(),
      singleFunction);
}

CallResult<HermesValue> eval(void *, Runtime *runtime, NativeArgs args) {
  GCScope gcScope(runtime);

  if (!args.getArg(0).isString()) {
    return args.getArg(0);
  }

  return directEval(runtime, args.dyncastArg<StringPrimitive>(0), {}, false);
}

} // namespace vm
} // namespace hermes
