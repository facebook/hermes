/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_PROFILER_SAMPLINGPROFILERDEFS_H
#define HERMES_VM_PROFILER_SAMPLINGPROFILERDEFS_H

#if defined(__EMSCRIPTEN__)
#define HERMESVM_SAMPLING_PROFILER_AVAILABLE 0
#else // !defined(__EMSCRIPTEN__)

#define HERMESVM_SAMPLING_PROFILER_AVAILABLE 1

#if defined(_WINDOWS)
#define HERMESVM_SAMPLING_PROFILER_WINDOWS
#else // !defined(_WINDOWS)
#define HERMESVM_SAMPLING_PROFILER_POSIX
#endif // !defined(_WINDOWS)

#endif // !defined(__EMSCRIPTEN__)

#endif // HERMES_VM_PROFILER_SAMPLINGPROFILERDEFS_H
