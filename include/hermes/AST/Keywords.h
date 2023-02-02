/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_AST_KEYWORDS_H
#define HERMES_AST_KEYWORDS_H

#include "hermes/AST/RecursiveVisitor.h"

namespace hermes {

class Context;

namespace sem {

class Keywords {
 public:
  /// Identifier for "arguments".
  UniqueString *const identArguments;
  /// Identifier for "eval".
  UniqueString *const identEval;
  /// Identifier for "delete".
  UniqueString *const identDelete;
  /// Identifier for "this".
  UniqueString *const identThis;
  /// Identifier for "use strict".
  UniqueString *const identUseStrict;
  /// Identifier for "show source ".
  UniqueString *const identShowSource;
  /// Identifier for "hide source ".
  UniqueString *const identHideSource;
  /// Identifier for "sensitive".
  UniqueString *const identSensitive;
  /// Identifier for "var".
  UniqueString *const identVar;
  /// Identifier for "let".
  UniqueString *const identLet;
  /// Identifier for "const".
  UniqueString *const identConst;
  /// "+".
  UniqueString *const identPlus;
  /// "-".
  UniqueString *const identMinus;
  /// "=".
  UniqueString *const identAssign;
  /// "new"
  UniqueString *const identNew;
  /// "target"
  UniqueString *const identTarget;
  /// "typeof".
  UniqueString *const identTypeof;

  Keywords(Context &astContext);
};

} // namespace sem
} // namespace hermes

#endif
