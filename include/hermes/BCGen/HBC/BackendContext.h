#ifndef HERMES_BCGEN_HBC_BACKENDCONTEXT_H
#define HERMES_BCGEN_HBC_BACKENDCONTEXT_H

#include "hermes/AST/Context.h"

namespace hermes {
namespace hbc {

class LowerBuiltinCallsContext;

/// Context encapsulating data shared inside the HBC backend.
class BackendContext {
  BackendContext();

 public:
  ~BackendContext();

  static BackendContext &get(Context &ctx);

  std::shared_ptr<LowerBuiltinCallsContext> lowerBuiltinCallsContext;
};

} // namespace hbc
} // namespace hermes

#endif // HERMES_BCGEN_HBC_BACKENDCONTEXT_H
