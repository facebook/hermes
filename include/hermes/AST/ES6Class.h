/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_AST_ES6CLASS_H
#define HERMES_AST_ES6CLASS_H

#include "hermes/AST/ESTree.h"

namespace hermes {

/// Recursively transforms the ESTree Node tree such that ES6 classes
/// are converted into ES5 functions, to make the ESTree compatible
/// with Hermes IR's gen pipeline, which currently does not support
/// ES6 classes.
void transformES6Classes(Context &context, ESTree::Node *node);

} // namespace hermes

#endif
