/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_AST_ESTREEJSONDUMPER_H
#define HERMES_AST_ESTREEJSONDUMPER_H

#include "hermes/AST/ESTree.h"

#include "llvh/Support/raw_ostream.h"

namespace hermes {

class JSONEmitter;

/// Print out the contents of the given tree to \p os.
/// \p pretty for pretty print the JSON.
/// When \p sm is not null, print the source locations for the AST nodes.
void dumpESTreeJSON(
    llvh::raw_ostream &os,
    ESTree::NodePtr rootNode,
    bool pretty,
    SourceErrorManager *sm = nullptr);

/// Print out the contents of \p rootNode to \p json.
/// Does not call json.endJSONL(), caller should do that if necessary.
/// When \p sm is not null, print the source locations for the AST nodes.
void dumpESTreeJSON(
    JSONEmitter &json,
    ESTree::NodePtr rootNode,
    SourceErrorManager *sm = nullptr);

} // namespace hermes

#endif
