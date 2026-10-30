/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_JIT_JITCURARCH_H
#define HERMES_VM_JIT_JITCURARCH_H

#include "hermes/VM/JIT/Config.h"

#if HERMESVM_JIT

#if defined(__aarch64__) || defined(_M_ARM64)

#include "hermes/VM/JIT/arm64/JIT.h"

#include "arm64/JitEmitter.h"
#include "arm64/JitImpl.h"

/// The namespace containing the current architecture's JIT backend. The
/// shared compiler driver (JitCompiler.cpp) is compiled inside this
/// namespace, so its unqualified references to Emitter, JITContext, FR,
/// FRType, etc. resolve to the current backend's types.
#define HERMESVM_JIT_ARCH_NS arm64

#else
#error "JitCurArch.h: unsupported JIT architecture"
#endif

#endif // HERMESVM_JIT
#endif // HERMES_VM_JIT_JITCURARCH_H
