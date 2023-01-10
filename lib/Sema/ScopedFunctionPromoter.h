/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SEMA_SCOPEDFUNCTIONPROMOTER_H
#define HERMES_SEMA_SCOPEDFUNCTIONPROMOTER_H

#include "hermes/AST/ESTree.h"

namespace hermes {

namespace sem {
class Keywords;
}

namespace sema {

class SemanticResolver;

/// This function checks whether it is safe to promote block-scoped function
/// declarations to function scope. i.e. whether it is safe to replace one with
/// "var" without creating a conflict.
///
/// A conflict exists if a let-like declaration is visible in the declaration
/// scope. The checker starts with a list of all block scoped function
/// declarations. Then it visits all scopes recursively, maintaining a scoped
/// table of let-like declarations with matching names. When it encounters a
/// block-scoped function declaration, it checks whether a matching let-like
/// declaration is visible. If not, it is safe to promote.
///
/// The input is a list of block-scoped function function declarations. The
/// ones that can be promoted are deleted from their own scope and added to the
/// function scope.
void promoteScopedFuncDecls(
    SemanticResolver &resolver,
    ESTree::FunctionLikeNode *funcNode);

} // namespace sema
} // namespace hermes

#endif
