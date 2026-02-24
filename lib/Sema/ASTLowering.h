/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SEMA_ASTLOWERING_H
#define HERMES_SEMA_ASTLOWERING_H

#include "hermes/AST/ESTree.h"

namespace hermes::flow {
class FlowContext;
}

namespace hermes::sema {
class SemContext;

/// Perform type-aware lowering of the AST.
/// \return true on success, false on unexpected error like nesting overflow,
///    in which case the error will have been reported.
bool lowerAST(
    Context &astContext,
    SemContext &semContext,
    flow::FlowContext &flowContext,
    ESTree::ProgramNode *root);

} // namespace hermes::sema

#endif
