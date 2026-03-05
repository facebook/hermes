/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_AST_STRIPTS_H
#define HERMES_AST_STRIPTS_H

#include "hermes/AST/Config.h"
#include "hermes/AST/ESTree.h"

#if HERMES_PARSE_TS

namespace hermes {

/// Strip erasable TypeScript syntax from the AST.
/// Reports errors for non-erasable constructs (enum, namespace, parameter
/// properties).
/// \return the transformed node, or nullptr on error.
ESTree::Node *stripTS(Context &context, ESTree::Node *node);

} // namespace hermes

#endif // HERMES_PARSE_TS

#endif // HERMES_AST_STRIPTS_H
