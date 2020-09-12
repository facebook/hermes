/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "HermesParserDiagHandler.h"

#include "hermes/Support/UTF8.h"
#include "llvh/ADT/Twine.h"

using llvh::Twine;

namespace hermes {

HermesParserDiagHandler::HermesParserDiagHandler(SourceErrorManager &sm)
    : sm_(sm) {
  sm_.setDiagHandler(handler, this);
  sm_.setErrorLimit(1);
}

void HermesParserDiagHandler::handler(
    const llvh::SMDiagnostic &diag,
    void *ctx) {
  auto *mgr = static_cast<HermesParserDiagHandler *>(ctx);
  if (diag.getKind() == llvh::SourceMgr::DK_Error) {
    if (!mgr->hasError()) {
      mgr->firstError_ = diag;
    }
  }
}

std::string HermesParserDiagHandler::getErrorString() const {
  int lineNo = firstError_.getLineNo();
  int columnNo = firstError_.getColumnNo();
  if (lineNo == -1 || columnNo == -1) {
    return (firstError_.getMessage() + "\n").str();
  }

  // Build lines to be used in error message
  std::string messageLine = (Twine(lineNo) + ":" + Twine(columnNo) + ": " +
                             firstError_.getMessage() + "\n")
                                .str();

  std::string sourceLine;
  std::string caretLine;
  std::tie(sourceLine, caretLine) = SourceErrorManager::buildSourceAndCaretLine(
      firstError_, sm_.getOutputOptions());

  // Do not include caret line for non-ASCII characters which may have width > 1
  bool showCaret = isAllASCII(sourceLine.begin(), sourceLine.end());
  if (!showCaret) {
    return (Twine(messageLine) + sourceLine + "\n").str();
  }

  return (Twine(messageLine) + sourceLine + "\n" + caretLine + "\n").str();
}

} // namespace hermes
