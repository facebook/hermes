/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_AST_ASYNCGENERATORS_H
#define HERMES_AST_ASYNCGENERATORS_H

#include "hermes/AST/ESTree.h"

namespace hermes {

/// Recursively transforms the ESTree Node tree such that async generators
/// are converted into generators
ESTree::Node *transformAsyncGenerators(Context &context, ESTree::Node *node);

} // namespace hermes

#endif
