/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_IRGEN_IRGEN_H
#define HERMES_IRGEN_IRGEN_H

#include "hermes/AST/Context.h"
#include "hermes/IR/IR.h"
#include "hermes/IR/IRBuilder.h"

#include <vector>

namespace hermes {

namespace ESTree {
class Node;
using NodePtr = Node *;
} // namespace ESTree

using DeclarationFileListTy = std::vector<ESTree::ProgramNode *>;

namespace hbc {

class BytecodeFunction;

struct LazyCompilationData {
  /// The variables in scope at the point where the function is defined.
  std::shared_ptr<SerializedScope> parentScope;

  /// The original name of the function, as found in the source.
  Identifier originalName;
  /// The generated name of the variable holding the function in the parent's
  /// frame, which is what we need to look up to reference ourselves. It is only
  /// set if there is an alias binding from \c originalName (which must be
  /// valid) and said variable, which must have a different name (since it is
  /// generated). Function::lazyClosureAlias_.
  Identifier closureAlias;

  /// The source buffer ID in which we can find the function source.
  uint32_t bufferId;

  /// The type of function, e.g. statement or expression.
  ESTree::NodeKind nodeKind;

  /// Whether or not the function is strict.
  bool strictMode;
};
} // namespace hbc

/// Lowers an ESTree program into Hermes IR in \p M.
/// \param declFileList a list of parsed global property definition files.
/// \param scopeChain identifiers in the environment, if compiling for local
/// eval. \returns True if an error occured and a message was emitted.
bool generateIRFromESTree(
    ESTree::NodePtr node,
    Module *M,
    const DeclarationFileListTy &declFileList,
    const ScopeChain &scopeChain);

/// Lowers an ESTree program into Hermes IR in \p M without a top-level
/// function, so that it can be used as a CommonJS module.
/// \param id the ID assigned to the CommonJS module when added to the IR
///           (index when reading filenames for the first time)
/// \param filename the relative filename to the CommonJS module.
void generateIRForCJSModule(
    ESTree::FunctionExpressionNode *node,
    uint32_t id,
    llvm::StringRef filename,
    Module *M,
    Function *topLevelFunction,
    const DeclarationFileListTy &declFileList);

/// Generate IR from the AST of a previously pre-parsed "lazy" function by
/// parsing it again and validating it. On error, a stub function which throws
/// a SyntaxError will be emitted instead.
/// \return the newly generated function IR and lexical scope root
std::pair<Function *, Function *> generateLazyFunctionIR(
    hbc::BytecodeFunction *bcFunction,
    Module *M,
    llvm::SMRange sourceRange);

} // namespace hermes

#endif
