/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/AST/TransformAST.h"
#include "hermes/AST/AsyncGenerator.h"

namespace hermes {

ESTree::Node *transformASTForCompilation(Context &context, ESTree::Node *root) {
  if (context.getEnableAsyncGenerators()) {
    transformAsyncGenerators(context, root);
  }
  return root;
}

} // namespace hermes
