/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_AST_ASTUTILS_H
#define HERMES_AST_ASTUTILS_H

#include <memory>

namespace hermes {
class Context;
namespace ESTree {
class ProgramNode;
} // namespace ESTree

/// Wrap the given program in a IIFE, e.g.
/// (function(exports){ ...program body })({});
/// This function cannot fail.
ESTree::ProgramNode *wrapInIIFE(
    std::shared_ptr<Context> &context,
    ESTree::ProgramNode *program);

} // namespace hermes

#endif
