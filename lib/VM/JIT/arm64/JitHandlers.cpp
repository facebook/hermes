/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JIT/Config.h"
#if HERMESVM_JIT
#include "JitHandlers.h"

#include "hermes/VM/Callable.h"
#include "hermes/VM/CodeBlock.h"
#include "hermes/VM/Interpreter.h"
#include "hermes/VM/RuntimeModule-inline.h"
#include "hermes/VM/RuntimeModule.h"
#include "hermes/VM/StaticHUtils.h"

#define DEBUG_TYPE "jit"

namespace hermes::vm {

SHLegacyValue _sh_ljs_create_bytecode_closure(
    SHRuntime *shr,
    const SHLegacyValue *env,
    SHRuntimeModule *shRuntimeModule,
    uint32_t functionID) {
  Runtime &runtime = getRuntime(shr);
  auto *runtimeModule = (RuntimeModule *)shRuntimeModule;
  GCScopeMarkerRAII marker{runtime};
  return JSFunction::createWithInferredParent(
             runtime,
             runtimeModule->getDomain(runtime),
             env ? Handle<Environment>::vmcast(toPHV(env))
                 : Runtime::makeNullHandle<Environment>(),
             runtimeModule->getCodeBlockMayAllocate(functionID))
      .getHermesValue();
}

SHLegacyValue _sh_ljs_get_bytecode_string(
    SHRuntime *shr,
    SHRuntimeModule *runtimeModule,
    uint32_t stringID) {
  return HermesValue::encodeStringValue(
      ((RuntimeModule *)runtimeModule)
          ->getStringPrimFromStringIDMayAllocate(stringID));
}

/// Wrapper for Interpreter::createObjectFromBuffer.
SHLegacyValue _interpreter_create_object_from_buffer(
    SHRuntime *shr,
    SHCodeBlock *codeBlock,
    uint32_t shapeTableIndex,
    uint32_t valBufferOffset) {
  Runtime &runtime = getRuntime(shr);
  CallResult<PseudoHandle<>> res = Interpreter::createObjectFromBuffer(
      runtime, (CodeBlock *)codeBlock, shapeTableIndex, valBufferOffset);
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    _sh_throw_current(shr);
  }
  return res->getHermesValue();
}

} // namespace hermes::vm

#endif // HERMESVM_JIT
