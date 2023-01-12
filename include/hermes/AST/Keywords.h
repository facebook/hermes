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
  const UniqueString *const identArguments;
  /// Identifier for "eval".
  const UniqueString *const identEval;
  /// Identifier for "delete".
  const UniqueString *const identDelete;
  /// Identifier for "this".
  const UniqueString *const identThis;
  /// Identifier for "use strict".
  const UniqueString *const identUseStrict;
  /// Identifier for "show source ".
  const UniqueString *const identShowSource;
  /// Identifier for "hide source ".
  const UniqueString *const identHideSource;
  /// Identifier for "sensitive".
  const UniqueString *const identSensitive;
  /// Identifier for "var".
  const UniqueString *const identVar;
  /// Identifier for "let".
  const UniqueString *const identLet;
  /// Identifier for "const".
  const UniqueString *const identConst;
  /// "+".
  const UniqueString *const identPlus;
  /// "-".
  const UniqueString *const identMinus;
  /// "=".
  const UniqueString *const identAssign;
  /// "new"
  const UniqueString *const identNew;
  /// "target"
  const UniqueString *const identTarget;
  /// "typeof".
  const UniqueString *const identTypeof;
  /// "constructor".
  const UniqueString *const identConstructor;
  /// "length".
  const UniqueString *const identLength;

  Keywords(Context &astContext);
};

} // namespace sem
} // namespace hermes

#endif
