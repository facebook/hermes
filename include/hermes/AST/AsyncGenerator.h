#ifndef HERMES_AST_ASYNCGENERATORS_H
#define HERMES_AST_ASYNCGENERATORS_H

#include "hermes/AST/ESTree.h"

namespace hermes {

/// Recursively transforms the ESTree Node tree such that async generators
/// are converted into generators
void transformAsyncGenerators(Context &context, ESTree::Node *node);

} // namespace hermes

#endif
