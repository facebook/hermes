/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#define DEBUG_TYPE "vm"
#include "JSLib/JSLibInternal.h"
#include "hermes/VM/Interpreter.h"
#include "hermes/VM/StringPrimitive.h"

#include "Interpreter-internal.h"

using namespace hermes::inst;

namespace hermes {
namespace vm {

ExecutionStatus Interpreter::caseDirectEval(
    Runtime *runtime,
    PinnedHermesValue *frameRegs,
    const Inst *ip) {
  auto *result = &O1REG(DirectEval);
  auto *input = &O2REG(DirectEval);

  if (!input->isString()) {
    *result = *input;
    return ExecutionStatus::RETURNED;
  }

  GCScopeMarkerRAII gcMarker{runtime};

  // Create a dummy scope, so that the local eval executes in its own scope
  // (as per the spec for strict callers, which is the only thing we support).

  ScopeChain scopeChain{};
  scopeChain.functions.emplace_back();

  auto cr = vm::directEval(
      runtime, Handle<StringPrimitive>::vmcast(input), scopeChain, false);
  if (cr == ExecutionStatus::EXCEPTION)
    return ExecutionStatus::EXCEPTION;

  *result = *cr;
  return ExecutionStatus::RETURNED;
}

} // namespace vm
} // namespace hermes
