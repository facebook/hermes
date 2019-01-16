#define DEBUG_TYPE "vm"
#include "JSLib/JSLibInternal.h"
#include "hermes/VM/Interpreter.h"
#include "hermes/VM/StringPrimitive.h"

namespace hermes {
namespace vm {

ExecutionStatus Interpreter::directEval(
    Runtime *runtime,
    const hermes::inst::DirectEvalInst *inst) {
  auto *result = &runtime->getCurrentFrame()->getLocalVarRef(inst->op1);
  auto *input = &runtime->getCurrentFrame()->getLocalVarRef(inst->op2);

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
