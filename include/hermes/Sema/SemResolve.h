/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SEMA_SEMRESOLVE_H
#define HERMES_SEMA_SEMRESOLVE_H

#include "hermes/Sema/FlowContext.h"

#include <vector>

namespace hermes {

class Context;

namespace ESTree {
class Node;
class ProgramNode;
} // namespace ESTree

using DeclarationFileListTy = std::vector<ESTree::ProgramNode *>;

namespace sema {

class SemContext;

/// Perform semantic resolution of the entire AST, starting from the specified
/// root, which should be ProgramNode.
/// \return true on success, false if errors were reported.
bool resolveAST(
    Context &astContext,
    SemContext &semCtx,
    flow::FlowContext *flowContext,
    ESTree::Node *root,
    const DeclarationFileListTy &ambientDecls = {});

/// Perform semantic resolution of the entire AST, starting from the specified
/// root, which should be ProgramNode.
/// \return true on success, false if errors were reported.
inline bool resolveAST(
    Context &astContext,
    SemContext &semCtx,
    ESTree::Node *root,
    const DeclarationFileListTy &ambientDecls = {}) {
  return resolveAST(astContext, semCtx, nullptr, root, ambientDecls);
}

/// Perform semantic resolution of the entire AST, without preparing the AST for
/// compilation. This will not error on features we can parse but not compile,
/// transform the AST, or perform compilation specific validation.
/// \return true on success, false if errors were reported.
bool resolveASTForParser(
    Context &astContext,
    SemContext &semCtx,
    ESTree::Node *root);

/// Dump the state of the SemContext and optionally the Flow context, and print
/// the annotated AST.
void semDump(
    llvh::raw_ostream &os,
    SemContext &semCtx,
    flow::FlowContext *flowContext,
    ESTree::Node *root);

} // namespace sema
} // namespace hermes

#endif
