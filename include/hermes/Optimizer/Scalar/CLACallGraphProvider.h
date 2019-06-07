/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_OPTIMIZER_SCALAR_CLA_CALLGRAPH_PROVIDER_H
#define HERMES_OPTIMIZER_SCALAR_CLA_CALLGRAPH_PROVIDER_H

#include "CallGraphProvider.h"
#include "ClosureAnalysis.h"
#include "SetConstraintAnalysisProblem.h"

namespace hermes {

/// This class provides an implementation of the call graph
/// based on closure analysis.  The constructor calls into
/// SetConstraintAnalysisProblem to read out the call graph
/// info.
class CLACallGraphProvider : public CallGraphProvider {
  SetConstraintAnalysisProblem *P_;

 public:
  CLACallGraphProvider(SetConstraintAnalysisProblem *P) {
    P_ = P;
    P_->getCallGraph(callees_, callsites_, receivers_, stores_);
  }
};
} // namespace hermes

#endif // HERMES_OPTIMIZER_SCALAR_CLA_CALLGRAPH_PROVIDER_H
