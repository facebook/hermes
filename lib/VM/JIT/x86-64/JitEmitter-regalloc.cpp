/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JIT/Config.h"
#if HERMESVM_JIT_X86_64
#include "JitEmitter-internal.h"
#include "JitEmitter.h"

// Task 2: no register allocator declaration in JitEmitter.h currently needs
// a body to link — the per-opcode Emitter methods are all unsupported()
// stubs in JitEmitter-stubs.cpp and never reach register allocation. Task 3
// ports the arm64 register allocator (getOrAllocFRInGpX/VecD/AnyReg,
// freeReg, syncToFrame, etc.) into this file.

#endif // HERMESVM_JIT_X86_64
