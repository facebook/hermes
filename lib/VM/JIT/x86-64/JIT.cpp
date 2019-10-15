/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JIT/x86-64/JIT.h"

#include "FastJIT.h"

namespace hermes {
namespace vm {
namespace x86_64 {

JITContext::JITContext(bool enable, size_t blockSize, size_t maxMemory)
    : enabled_(enable), heap_(blockSize / 2, blockSize / 2, maxMemory) {}

JITContext::~JITContext() = default;

JITCompiledFunctionPtr JITContext::compileImpl(
    Runtime *runtime,
    CodeBlock *codeBlock) {
  FastJIT impl{this, codeBlock};
  impl.compile();
  return codeBlock->getJITCompiled();
}

} // namespace x86_64
} // namespace vm
} // namespace hermes
