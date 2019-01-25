/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_PUBLIC_RUNTIMECONFIG_H
#define HERMES_PUBLIC_RUNTIMECONFIG_H

#include "hermes/Public/CtorConfig.h"
#include "hermes/Public/GCConfig.h"

namespace hermes {
namespace vm {

class PinnedHermesValue;

// Parameters for Runtime initialisation.  Check documentation in README.md
#define RUNTIME_FIELDS(F)                         \
  /* Parameters to be passed on to the GC. */     \
  F(vm::GCConfig, GCConfig)                       \
                                                  \
  /* Pre-allocated Register Stack */              \
  F(PinnedHermesValue *, RegisterStack, nullptr)  \
                                                  \
  /* Register Stack Size */                       \
  F(unsigned, MaxNumRegisters, 1024 * 1024)       \
                                                  \
  /* Whether or not the JIT is enabled */         \
  F(bool, EnableJIT, false)                       \
                                                  \
  /* Whether to allow eval and Function ctor */   \
  F(bool, EnableEval, true)                       \
                                                  \
  /* Support for ES6 Symbol. */                   \
  F(bool, ES6Symbol, true)                        \
                                                  \
  /* Enable sampling certain statistics. */       \
  F(bool, EnableSampledStats, false)              \
                                                  \
  /* Whether to enable sampling profiler */       \
  F(bool, EnableSampleProfiling, false)           \
                                                  \
  /* Whether to randomize stack placement etc. */ \
  F(bool, RandomizeMemoryLayout, false)           \
                                                  \
  /* Eagerly read bytecode into page cache. */    \
  F(unsigned, BytecodeWarmupPercent, 0)           \
  /* RUNTIME_FIELDS END */

_HERMES_CTORCONFIG_STRUCT(RuntimeConfig, RUNTIME_FIELDS, {});

#undef RUNTIME_FIELDS

} // namespace vm
} // namespace hermes

#endif // HERMES_PUBLIC_RUNTIMECONFIG_H
