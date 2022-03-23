/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_TEST_PARSER_DIAGCONTEXT_H
#define HERMES_TEST_PARSER_DIAGCONTEXT_H

#include "hermes/AST/Context.h"

namespace hermes {
namespace parser {

class DiagContext {
  int errCount_{0};
  int warnCount_{0};

 public:
  DiagContext(SourceErrorManager &mgr) {
    mgr.setDiagHandler(handler, this);
  }
  DiagContext(Context &astContext)
      : DiagContext(astContext.getSourceErrorManager()) {}

  void clear() {
    errCount_ = warnCount_ = 0;
  }
  int getErrCount() const {
    return errCount_;
  }
  int getWarnCount() const {
    return warnCount_;
  }

  int getErrCountClear() {
    unsigned res = errCount_;
    clear();
    return res;
  }
  int getWarnCountClear() {
    unsigned res = warnCount_;
    clear();
    return res;
  }

 private:
  static void handler(const llvh::SMDiagnostic &msg, void *ctx) {
    DiagContext *diag = static_cast<DiagContext *>(ctx);
    if (msg.getKind() == llvh::SourceMgr::DK_Error)
      ++diag->errCount_;
    else if (msg.getKind() == llvh::SourceMgr::DK_Warning)
      ++diag->warnCount_;
  }
};

} // namespace parser
} // namespace hermes

#endif // HERMES_TEST_PARSER_DIAGCONTEXT_H
