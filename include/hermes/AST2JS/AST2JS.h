/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_AST2JS_AST2JS_H
#define HERMES_AST2JS_AST2JS_H

namespace llvh {
class raw_ostream;
}

namespace hermes {
namespace ESTree {
class Node;
}

/// Output the supplied AST as JS.
/// \param OS the output stream
/// \param root the root AST node
/// \param pretty pretty print or not.
void generateJS(llvh::raw_ostream &OS, ESTree::Node *root, bool pretty);

} // namespace hermes

#endif
