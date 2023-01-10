/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Sema/SemResolve.h"

#include "SemanticResolver.h"
#include "hermes/AST/ESTree.h"
#include "hermes/Support/PerfSection.h"

namespace hermes {
namespace sema {

bool resolveAST(Context &astContext, SemContext &semCtx, ESTree::Node *root) {
  PerfSection validation("Resolving JavaScript function AST");
  // Resolve the entire AST.
  SemanticResolver resolver{astContext, semCtx, true};
  return resolver.run(root);
}

bool resolveASTForParser(
    Context &astContext,
    SemContext &semCtx,
    ESTree::Node *root) {
  SemanticResolver resolver{astContext, semCtx, false};
  return resolver.run(root);
}

} // namespace sema
} // namespace hermes
