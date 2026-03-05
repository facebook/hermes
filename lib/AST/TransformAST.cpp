/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/AST/TransformAST.h"
#include "hermes/AST/AsyncGenerator.h"
#if HERMES_PARSE_TS
#include "hermes/AST/StripTS.h"
#endif

namespace hermes {

ESTree::Node *transformASTForCompilation(Context &context, ESTree::Node *root) {
#if HERMES_PARSE_TS
  if (context.getTransformTS()) {
    root = stripTS(context, root);
    if (!root)
      return nullptr;
  }
#endif
  if (context.getEnableAsyncGenerators()) {
    root = transformAsyncGenerators(context, root);
  }
  return root;
}

} // namespace hermes
