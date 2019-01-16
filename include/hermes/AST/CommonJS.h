#ifndef HERMES_AST_COMMONJS_H
#define HERMES_AST_COMMONJS_H

#include "hermes/AST/ESTree.h"

#include "llvm/ADT/StringRef.h"

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
    ESTree::FileNode *file);

} // namespace hermes

#endif
