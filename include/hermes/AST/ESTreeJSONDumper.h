/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_AST_ESTREEJSONDUMPER_H
#define HERMES_AST_ESTREEJSONDUMPER_H

#include "hermes/AST/ESTree.h"

#include "llvm/Support/raw_ostream.h"

namespace hermes {

/// Print out the contents of the given tree to \p os.
/// \p pretty for pretty print the JSON.
/// When \p sm is not null, print the source locations for the AST nodes.
void dumpESTreeJSON(
    llvm::raw_ostream &os,
    ESTree::NodePtr rootNode,
    bool pretty,
    SourceErrorManager *sm = nullptr);

} // namespace hermes

#endif
