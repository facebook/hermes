/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_PROFILER_H
#define HERMES_VM_PROFILER_H

#ifdef HERMESVM_PROFILER_OPCODE
#include <x86intrin.h>

#define INIT_OPCODE_PROFILER      \
  uint64_t startTime = __rdtsc(); \
  unsigned curOpcode = (unsigned)OpCode::Call;

#define RECORD_OPCODE_START_TIME               \
  curOpcode = (unsigned)ip->opCode;            \
  runtime.opcodeExecuteFrequency[curOpcode]++; \
  startTime = __rdtsc();

#define UPDATE_OPCODE_TIME_SPENT \
  runtime.timeSpent[curOpcode] += __rdtsc() - startTime

#else

#define INIT_OPCODE_PROFILER
#define RECORD_OPCODE_START_TIME
#define UPDATE_OPCODE_TIME_SPENT

#endif

#if defined(HERMESVM_PROFILER_JSFUNCTION)
#include "hermes/BCGen/HBC/BytecodeDataProvider.h"
#include "hermes/VM/SymbolID.h"

namespace hermes {
namespace vm {

/// ID assigned to each function the profiler encounters.
using ProfilerID = uint32_t;

constexpr ProfilerID NO_PROFILER_ID = UINT32_MAX;

/// Metadata for a function that the profiler has seen.
/// Getting the function offset from functionID is very expensive,
/// thus we store the ID to calculate the offset at symbol dump time.
struct ProfilerFunctionInfo {
  ProfilerID profilerId;
  std::string functionName;
  uint32_t functionID;
  std::shared_ptr<hbc::BCProvider> bytecode;
  ProfilerFunctionInfo(
      ProfilerID id,
      std::string name,
      uint32_t fid,
      std::shared_ptr<hbc::BCProvider> bc)
      : profilerId(id),
        functionName(name),
        functionID(fid),
        bytecode(std::move(bc)) {}
};

} // namespace vm
} // namespace hermes
#endif

#if defined(HERMESVM_PROFILER_JSFUNCTION)
#include <x86intrin.h>
namespace hermes {
namespace vm {
/// Structure to capture the event of either entering a function or
/// exiting a function. Currently we only cover normal functions,
/// and exclude native and bound functions.
struct ProfilerFunctionEvent {
  ProfilerID functionID;
  uint64_t timeStamp;
  uint64_t opcodeStamp;
  bool isEnter;
  ProfilerFunctionEvent(ProfilerID s, uint64_t t, uint64_t o, bool e)
      : functionID(s), timeStamp(t), opcodeStamp(o), isEnter(e) {}
};

#define INC_OPCODE_COUNT ++runtime.opcodeCount

#define PROFILER_ENTER_FUNCTION(block)                            \
  do {                                                            \
    runtime.maxStackLevel =                                       \
        std::max(runtime.maxStackLevel, runtime.getStackLevel()); \
    auto id = runtime.getProfilerID(block);                       \
    auto o = runtime.opcodeCount;                                 \
    runtime.functionEvents.emplace_back(id, __rdtsc(), o, true);  \
  } while (false)

#define PROFILER_EXIT_FUNCTION(block)                             \
  do {                                                            \
    auto id = runtime.getProfilerID(block);                       \
    auto o = runtime.opcodeCount;                                 \
    runtime.functionEvents.emplace_back(id, __rdtsc(), o, false); \
  } while (false)

} // namespace vm
} // namespace hermes

#else

#define INC_OPCODE_COUNT
#define PROFILER_ENTER_FUNCTION(block)
#define PROFILER_EXIT_FUNCTION(block)

#endif

// Profile frequency and duration of native calls.
#ifdef HERMESVM_PROFILER_NATIVECALL

#if defined(__i386__) || defined(__x86_64__)
#include <x86intrin.h>
#define HERMESVM_RDTSC() __rdtsc()
#else
#define HERMESVM_RDTSC() 0
#endif

#endif // HERMESVM_PROFILER_NATIVECALL

#endif // HERMES_VM_PROFILER_H
