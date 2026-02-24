/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_SCOPECHAIN_H
#define HERMES_SUPPORT_SCOPECHAIN_H

#include "llvh/ADT/StringRef.h"

#include <vector>

namespace hermes {
/// A ScopeChainItem represents variables available in a scope.
struct ScopeChainItem {
  /// List of variables in this function.
  std::vector<llvh::StringRef> variables;
};
/// A ScopeChain is a sequence of nested ScopeChainItems, from innermost to
/// outermost scopes.
struct ScopeChain {
  /// Functions on the stack. Innermost (direct parent) is 0.
  std::vector<ScopeChainItem> functions;
};

} // namespace hermes

#endif // HERMES_SUPPORT_SCOPECHAIN_H
