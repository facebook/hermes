/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SEMA_ESTREECLONE_H
#define HERMES_SEMA_ESTREECLONE_H

#include "DeclCollector.h"

namespace hermes {

class Context;

namespace sema {

class SemContext;
class FunctionInfo;
class LexicalScope;

/// Clone \p node.
/// Clone the semantic information in \p semContext and associate it recursively
/// with the newly created node and its children.
/// Clone the decls in \p declCollectorMap as well.
/// The node can then be inserted into the AST wherever the caller chooses.
/// May report recursion depth errors through the SourceErrorManager.
/// \param node the FunctionDeclaration or ClassDeclaration node to clone.
/// \param declCollectorMap the map to add the cloned decls to.
/// \param functionInfo the FunctionInfo to which \p node belongs.
/// \param scope the LexicalScope to which \p node belongs.
/// \return the cloned node, nullptr if there were errors.
ESTree::Node *cloneNode(
    Context &context,
    SemContext &semContext,
    DeclCollectorMapTy &declCollectorMap,
    ESTree::Node *node,
    FunctionInfo *functionInfo,
    LexicalScope *scope);

} // namespace sema
} // namespace hermes

#endif
