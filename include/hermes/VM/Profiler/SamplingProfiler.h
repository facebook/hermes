/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifdef _WINDOWS
#include "hermes/VM/Profiler/SamplingProfilerWindows.h"
#else
#include "hermes/VM/Profiler/SamplingProfilerPosix.h"
#endif
