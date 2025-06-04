/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "hermes/AST/Context.h"
#include "hermes/AST/ESTree.h"

namespace hermes {

/// General purpose AST transformation which will be applied before running
/// semantic resolution in the compiler pipeline.
/// Allows adding functionality/transforms in a general way directly to the AST
/// in a way that works for lazy compilation, debugger eval, etc.
///
/// \return the transformed node, which should be used for the remainder of
///   compilation. On failure, report an error and return nullptr.
///   The returned Node must be the same kind as the original \p root.
ESTree::Node *transformASTForCompilation(Context &context, ESTree::Node *root);

} // namespace hermes
