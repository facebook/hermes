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

namespace llvh {
class raw_ostream;
}

namespace hermes {

class Context;

namespace ESTree {
class Node;
class ProgramNode;
class FunctionExpressionNode;
} // namespace ESTree

using DeclarationFileListTy = std::vector<ESTree::ProgramNode *>;

namespace sema {

class SemContext;

/// Perform semantic resolution of the entire AST, starting from the specified
/// root, which should be ProgramNode.
///
/// \param flowContext if nonnull, perform Flow type checking using that type
///     container.
/// \return true on success, false if errors were reported.
bool resolveAST(
    Context &astContext,
    SemContext &semCtx,
    flow::FlowContext *flowContext,
    ESTree::ProgramNode *root,
    const DeclarationFileListTy &ambientDecls = {});

/// Perform semantic resolution of the entire AST, starting from the specified
/// root, which should be ProgramNode.
/// \return true on success, false if errors were reported.
inline bool resolveAST(
    Context &astContext,
    SemContext &semCtx,
    ESTree::ProgramNode *root,
    const DeclarationFileListTy &ambientDecls = {}) {
  return resolveAST(astContext, semCtx, nullptr, root, ambientDecls);
}

/// Run semantic resolution for a lazy function and store the result in \c
/// semCtx_.
/// \param root the top-level function node to run resolution on.
/// \param semInfo the original FunctionInfo for the root node,
///   which was created on the first pass and will be populated with real
///   scopes now.
/// \return false on error.
bool resolveASTLazy(
    Context &astContext,
    SemContext &semCtx,
    ESTree::FunctionLikeNode *root,
    FunctionInfo *semInfo,
    bool parentHadSuperBinding);

/// Run semantic resolution for 'eval' within a given scope.
/// This is distinct from resolveASTLazy because it operates on ProgramNode.
/// \param root the top-level function node to run resolution on.
/// \param semInfo the FunctionInfo for the enclosing function.
/// \return false on error.
bool resolveASTInScope(
    Context &astContext,
    SemContext &semCtx,
    ESTree::ProgramNode *root,
    FunctionInfo *semInfo,
    bool parentHadSuperBinding);

/// Perform semantic resolution of a CommonJS module.
bool resolveCommonJSAST(
    Context &astContext,
    SemContext &semCtx,
    flow::FlowContext *flowContext,
    ESTree::FunctionExpressionNode *root);

/// Perform semantic resolution of a CommonJS module.
inline bool resolveCommonJSAST(
    Context &astContext,
    SemContext &semCtx,
    ESTree::FunctionExpressionNode *root) {
  return resolveCommonJSAST(astContext, semCtx, nullptr, root);
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
    Context &astContext,
    SemContext &semCtx,
    flow::FlowContext *flowContext,
    ESTree::Node *root);

} // namespace sema
} // namespace hermes

#endif
