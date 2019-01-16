#ifndef HERMES_AST_SEMANTICVALIDATOR_H
#define HERMES_AST_SEMANTICVALIDATOR_H

#include "hermes/AST/Context.h"
#include "hermes/AST/ESTree.h"
#include "hermes/Support/SourceErrorManager.h"

namespace hermes {

/// Perform semantic validation of the entire AST, starting from the specified
/// root, which should be ProgramNode.
bool validateAST(Context &astContext, ESTree::NodePtr root);

/// Perform semantic validation of an individual function in the given context
/// \param function must be a function node
/// \param strict specifies parent strictness.
bool validateFunctionAST(
    Context &astContext,
    ESTree::NodePtr function,
    bool strict);

} // namespace hermes

#endif // HERMES_AST_SEMANTICVALIDATOR_H
