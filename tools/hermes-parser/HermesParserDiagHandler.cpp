/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
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
}

void HermesParserDiagHandler::handler(
    const llvh::SMDiagnostic &diag,
    void *ctx) {
  auto *mgr = static_cast<HermesParserDiagHandler *>(ctx);

  if (!mgr->hasError()) {
    // Only store the first error that is produced
    if (diag.getKind() == llvh::SourceMgr::DK_Error) {
      mgr->firstError_ = diag;
      return;
    }
  } else {
    // Notes that appear immediately after the error are stored
    if (diag.getKind() == llvh::SourceMgr::DK_Note && mgr->collectNotes_) {
      mgr->firstErrorNotes_.push_back(diag);
      return;
    }

    // No more notes can appear after the next non-note diagnostic is produced
    mgr->collectNotes_ = false;
  }
}

std::string HermesParserDiagHandler::getErrorString() const {
  std::string message = formatDiagnostic(firstError_);
  for (auto note : firstErrorNotes_) {
    message += "\n" + formatDiagnostic(note);
  }

  return message;
}

std::string HermesParserDiagHandler::formatDiagnostic(
    const llvh::SMDiagnostic &diag) const {
  std::string message = diag.getKind() == llvh::SourceMgr::DK_Note
      ? ("note: " + diag.getMessage()).str()
      : diag.getMessage().str();
  int lineNo = diag.getLineNo();
  int columnNo = diag.getColumnNo();
  if (lineNo == -1 || columnNo == -1) {
    return message;
  }

  // Build lines to be used in error message
  std::string messageLine =
      (message + " (" + Twine(lineNo) + ":" + Twine(columnNo) + ")\n").str();

  std::string sourceLine;
  std::string caretLine;
  std::tie(sourceLine, caretLine) =
      SourceErrorManager::buildSourceAndCaretLine(diag, sm_.getOutputOptions());

  // Do not include caret line for non-ASCII characters which may have width > 1
  bool showCaret = isAllASCII(sourceLine.begin(), sourceLine.end());
  if (!showCaret) {
    return (Twine(messageLine) + sourceLine).str();
  }

  return (Twine(messageLine) + sourceLine + "\n" + caretLine).str();
}

} // namespace hermes
