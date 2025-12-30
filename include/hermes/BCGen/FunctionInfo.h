/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_FUNCTIONINFO_H
#define HERMES_BCGEN_FUNCTIONINFO_H

#include "hermes/IR/IR.h"
namespace hermes {

/// Represents which kinds of calls on a function are prohibited.
/// Note: if more entries are added here, the backends may need to be updated to
/// reflect the larger enum size.
enum ProhibitInvoke {
  Call = 0,
  Construct = 1,
  None = 2,
};

/// Represents the kind of a function.
/// Note: if more entries are added here, the backends may need to be updated to
/// reflect the larger enum size.
enum FuncKind {
  Normal = 0,
  Generator = 1,
  Async = 2,
};

/// \return the ProhibitInvoke value corresponding to the function's \p defKind
/// and \p valKind.
constexpr ProhibitInvoke computeProhibitInvoke(
    Function::ProhibitInvoke defKind) {
  switch (defKind) {
    case Function::ProhibitInvoke::ProhibitNone:
      return ProhibitInvoke::None;
    case Function::ProhibitInvoke::ProhibitConstruct:
      return ProhibitInvoke::Construct;
    case Function::ProhibitInvoke::ProhibitCall:
      return ProhibitInvoke::Call;
  }
  llvm_unreachable("all cases handled");
}

/// \return the FuncKind value corresponding to the function's \p valKind.
constexpr FuncKind computeFuncKind(ValueKind valKind) {
  switch (valKind) {
    case ValueKind::GeneratorFunctionKind:
      return FuncKind::Generator;
    case ValueKind::AsyncFunctionKind:
      return FuncKind::Async;
    default:
      return FuncKind::Normal;
  }
}

} // namespace hermes

#endif
