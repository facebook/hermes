#ifndef HERMES_OPTIMIZER_SCALAR_SIMPLE_CALLGRAPH_PROVIDER_H
#define HERMES_OPTIMIZER_SCALAR_SIMPLE_CALLGRAPH_PROVIDER_H

#include "hermes/Optimizer/Scalar/CallGraphProvider.h"

namespace hermes {

class Function;

/// This class provides an (one) implementation of the CallGraphProvider
/// interface.  This implementation uses only local information based
/// on def-use. No closure analysis is used.
class SimpleCallGraphProvider : public CallGraphProvider {
  void initCallRelationships(Function *F);

 public:
  SimpleCallGraphProvider(Function *F) {
    initCallRelationships(F);
  }
};
} // namespace hermes

#endif // HERMES_OPTIMIZER_SCALAR_SIMPLE_CALLGRAPH_PROVIDER_H
