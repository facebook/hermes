/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_AST_TS2FLOW_H
#define HERMES_AST_TS2FLOW_H

#include "hermes/AST/ESTree.h"

namespace hermes {

/// Converts the given TypeScript AST to Flow AST.
/// TypeScript type annotations are interpreted in Flow semantics.
/// \param context the AST context.
/// \param program the top-level program node that may contain TypeScript nodes.
/// \return the converted program node. nullptr on error.
ESTree::ProgramNode *convertTSToFlow(
    Context &context,
    ESTree::ProgramNode *program);

} // namespace hermes

#endif
