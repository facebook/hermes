/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/AST/Keywords.h"

#include "hermes/AST/Context.h"
#include "hermes/Support/RegExpSerialization.h"

namespace hermes {
namespace sem {

Keywords::Keywords(Context &astContext)
    : identArguments(
          astContext.getIdentifier("arguments").getUnderlyingPointer()),
      identEval(astContext.getIdentifier("eval").getUnderlyingPointer()),
      identDelete(astContext.getIdentifier("delete").getUnderlyingPointer()),
      identThis(astContext.getIdentifier("this").getUnderlyingPointer()),
      identUseStrict(
          astContext.getIdentifier("use strict").getUnderlyingPointer()),
      identShowSource(
          astContext.getIdentifier("show source").getUnderlyingPointer()),
      identHideSource(
          astContext.getIdentifier("hide source").getUnderlyingPointer()),
      identSensitive(
          astContext.getIdentifier("sensitive").getUnderlyingPointer()),
      identVar(astContext.getIdentifier("var").getUnderlyingPointer()),
      identLet(astContext.getIdentifier("let").getUnderlyingPointer()),
      identConst(astContext.getIdentifier("const").getUnderlyingPointer()),
      identPlus(astContext.getIdentifier("+").getUnderlyingPointer()),
      identMinus(astContext.getIdentifier("-").getUnderlyingPointer()),
      identAssign(astContext.getIdentifier("=").getUnderlyingPointer()),
      identNew(astContext.getIdentifier("new").getUnderlyingPointer()),
      identTarget(astContext.getIdentifier("target").getUnderlyingPointer()),
      identTypeof(astContext.getIdentifier("typeof").getUnderlyingPointer()),
      identConstructor(
          astContext.getIdentifier("constructor").getUnderlyingPointer()),
      identLength(astContext.getIdentifier("length").getUnderlyingPointer()) {}

} // namespace sem
} // namespace hermes
