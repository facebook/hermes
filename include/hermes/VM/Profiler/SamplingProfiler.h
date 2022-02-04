/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// If nothing is defined then pick a sensible default
#if !defined(HERMESVM_SAMPLING_PROFILER_STUB) && \
    !defined(HERMESVM_SAMPLING_PROFILER_POSIX)
#if defined(_WINDOWS) || defined(__EMSCRIPTEN__)
#define HERMESVM_SAMPLING_PROFILER_STUB
#else
#define HERMESVM_SAMPLING_PROFILER_POSIX
#endif
#endif

#if defined(HERMESVM_SAMPLING_PROFILER_STUB)
#include "hermes/VM/Profiler/SamplingProfilerStub.h"
#elif defined(HERMESVM_SAMPLING_PROFILER_POSIX)
#include "hermes/VM/Profiler/SamplingProfilerPosix.h"
#else
#error "No sampling profiler implementation defined"
#endif
