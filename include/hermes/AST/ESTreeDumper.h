#ifndef HERMES_AST_ESTREEDUMPER_H
#define HERMES_AST_ESTREEDUMPER_H

#include "hermes/AST/ESTree.h"

#include "llvm/Support/raw_ostream.h"

namespace hermes {

/// Print out the contents of the given tree to \p os.
void dumpESTree(llvm::raw_ostream &os, ESTree::NodePtr rootNode);

} // namespace hermes

#endif
