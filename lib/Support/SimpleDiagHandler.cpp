/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/SimpleDiagHandler.h"

#include "llvh/ADT/Twine.h"

namespace hermes {

void SimpleDiagHandler::installInto(hermes::SourceErrorManager &sm) {
  sm.setDiagHandler(handler, this);
}

void SimpleDiagHandler::handler(const llvh::SMDiagnostic &msg, void *ctx) {
  auto *mgr = static_cast<SimpleDiagHandler *>(ctx);
  if (msg.getKind() == llvh::SourceMgr::DK_Error) {
    if (!mgr->haveErrors()) {
      mgr->firstMessage_ = msg;
    }
  }
}

std::string SimpleDiagHandler::getErrorString() const {
  const auto &msg = getFirstMessage();
  return (Twine(msg.getLineNo()) + ":" + Twine(msg.getColumnNo() + 1) + ":" +
          msg.getMessage())
      .str();
}

SimpleDiagHandlerRAII::SimpleDiagHandlerRAII(
    SourceErrorManager &sourceErrorManager)
    : sourceErrorManager_(sourceErrorManager),
      oldHandler_(sourceErrorManager.getDiagHandler()),
      oldContext_(sourceErrorManager.getDiagContext()),
      oldErrorLimit_(sourceErrorManager.getErrorLimit()) {
  installInto(sourceErrorManager);
  sourceErrorManager.clearErrorLimitReached();
  sourceErrorManager.setErrorLimit(1);
}

SimpleDiagHandlerRAII::~SimpleDiagHandlerRAII() {
  sourceErrorManager_.clearErrorLimitReached();
  sourceErrorManager_.setErrorLimit(oldErrorLimit_);
  sourceErrorManager_.setDiagHandler(oldHandler_, oldContext_);
}

} // namespace hermes
