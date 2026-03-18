/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_AST_ASTUTILS_H
#define HERMES_AST_ASTUTILS_H

#include "hermes/AST/ESTree.h"

#include "llvh/ADT/ArrayRef.h"

#include <memory>

namespace hermes {
class Context;

/// Wrap the given program in a IIFE, e.g.
/// (function(exports){ ...program body })({});
/// This function cannot fail.
ESTree::ProgramNode *wrapInIIFE(
    std::shared_ptr<Context> &context,
    ESTree::ProgramNode *program);

/// \return true if the given list of Decorators contains the given Decorator
/// member expression, provided as a list of identifiers.
/// \pre \p names is not empty.
/// \param names the list of identifiers to match.
///   If {"a", "b", "c"}, then we are looking for @a.b.c.
ESTree::DecoratorNode *findDecorator(
    ESTree::NodeList &decorators,
    llvh::ArrayRef<ESTree::NodeLabel> names);

} // namespace hermes

#endif
