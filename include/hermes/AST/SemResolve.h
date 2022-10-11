/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_AST_SEMRESOLVE_H
#define HERMES_AST_SEMRESOLVE_H

namespace hermes {

class Context;

namespace ESTree {
class Node;
}

namespace sema {

class SemContext;

/// Perform semantic resolution of the entire AST, starting from the specified
/// root, which should be ProgramNode.
/// \return true on success, false if errors were reported.
bool resolveAST(Context &astContext, SemContext &semCtx, ESTree::Node *root);

/// Perform semantic resolution of the entire AST, without preparing the AST for
/// compilation. This will not error on features we can parse but not compile,
/// transform the AST, or perform compilation specific validation.
/// \return true on success, false if errors were reported.
bool resolveASTForParser(
    Context &astContext,
    SemContext &semCtx,
    ESTree::Node *root);

} // namespace sema
} // namespace hermes

#endif
