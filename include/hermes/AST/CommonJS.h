/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_AST_COMMONJS_H
#define HERMES_AST_COMMONJS_H

#include "hermes/AST/ESTree.h"

#include "llvh/ADT/StringRef.h"

#include <memory>

namespace hermes {

/// Turns the given \p file into a CommonJS module.
/// Extracts the code from the file and wraps it in a function expression,
/// so the result looks something like:
/// (function(exports, require, module) {
///   <input file>
/// });
/// Ensures the result is non-null.
ESTree::FunctionExpressionNode *wrapCJSModule(
    std::shared_ptr<Context> &context,
    ESTree::ProgramNode *program);

} // namespace hermes

#endif
