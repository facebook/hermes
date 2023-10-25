/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_AST_ES6CLASS_H
#define HERMES_AST_ES6CLASS_H

#include "hermes/AST/ESTree.h"

#include "llvh/ADT/StringRef.h"

#include <memory>

namespace hermes {

void transformES6Classes(
    Context &context,
    ESTree::Node *node);

} // namespace hermes

#endif
