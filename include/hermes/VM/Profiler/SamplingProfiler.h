/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifdef _WINDOWS
#define HERMESVM_SAMPLING_PROFILER_USE_NO_OP
#include "hermes/VM/Profiler/SamplingProfilerNoOp.h"
#else
#define HERMESVM_SAMPLING_PROFILER_USE_POSIX
#include "hermes/VM/Profiler/SamplingProfilerPosix.h"
#endif
