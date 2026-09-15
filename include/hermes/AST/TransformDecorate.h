/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "hermes/AST/ESTree.h"

namespace hermes {

/// Rewrite `Hermes.decorate(name, Hermes.<dec>);` calls in the AST into
/// decoration entries on the preceding FunctionDeclaration's
/// FunctionLikeDecoration::decorations list, and remove the call statement.
///
/// A `Hermes.decorate(...)` call statement is only valid when it appears
/// immediately following a FunctionDeclaration in the same statement list,
/// and the first argument is the identifier of that FunctionDeclaration.
/// Any `Hermes.decorate(...)` call appearing anywhere else (in an
/// expression, not after a function declaration, after a non-matching
/// function, with the wrong shape) is reported as a compile error.
///
/// This pass is intended to run only in typed mode; the caller is
/// responsible for that gating.
///
/// \return \p root on success, or nullptr if any error was reported.
ESTree::Node *transformDecorate(Context &context, ESTree::Node *root);

} // namespace hermes
