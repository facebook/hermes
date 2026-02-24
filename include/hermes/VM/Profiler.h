/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_PROFILER_H
#define HERMES_VM_PROFILER_H

#include <cstdint>

#if defined(__i386__) || defined(__x86_64__)
#include <x86intrin.h>
#endif

namespace hermes {

#if defined(HERMESVM_PROFILER_JSFUNCTION) || \
    defined(HERMESVM_PROFILER_OPCODE) || defined(HERMESVM_PROFILER_NATIVECALL)

inline uint64_t rdtsc() {
#if defined(__i386__) || defined(__x86_64__)
  // Use the rdtsc intrinsic to get the cycle count if it's available.
  return __rdtsc();
#elif defined(__aarch64__)
  uint64_t t;
  // Use CNTVCT_EL0 (Counter-timer virtual count register) for AArch64.
  // The virtual timer is the physical timer minus an offset,
  // which appears to be used for compensating for running in a VM.
  // From the ARMv8 manual:
  //   The virtual count allows a hypervisor to show virtual time to a Virtual
  //   Machine (VM). For example, a hypervisor could use the
  //   offset to hide the passage of time when the VM was not scheduled. This
  //   means that the virtual count can represent time experienced by the VM,
  //   rather than wall clock time.
  // MRS: Move to general-purpose register from system register.
  asm volatile("mrs %0, cntvct_el0" : "=r"(t));
  return t;
#else
#error "Unsupported architecture for profiler"
#endif
}

#endif

} // namespace hermes

#ifdef HERMESVM_PROFILER_OPCODE

#define INIT_OPCODE_PROFILER            \
  uint64_t startTime = hermes::rdtsc(); \
  unsigned curOpcode = (unsigned)OpCode::Call;

#define RECORD_OPCODE_START_TIME               \
  curOpcode = (unsigned)ip->opCode;            \
  runtime.opcodeExecuteFrequency[curOpcode]++; \
  startTime = hermes::rdtsc();

#define UPDATE_OPCODE_TIME_SPENT \
  runtime.timeSpent[curOpcode] += hermes::rdtsc() - startTime

#else

#define INIT_OPCODE_PROFILER
#define RECORD_OPCODE_START_TIME
#define UPDATE_OPCODE_TIME_SPENT

#endif

#if defined(HERMESVM_PROFILER_JSFUNCTION)
#include "hermes/BCGen/HBC/BCProvider.h"
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

#define PROFILER_ENTER_FUNCTION(block)                                 \
  do {                                                                 \
    runtime.maxStackLevel =                                            \
        std::max(runtime.maxStackLevel, runtime.getStackLevel());      \
    auto id = runtime.getProfilerID(block);                            \
    auto o = runtime.opcodeCount;                                      \
    runtime.functionEvents.emplace_back(id, hermes::rdtsc(), o, true); \
  } while (false)

#define PROFILER_EXIT_FUNCTION(block)                                   \
  do {                                                                  \
    auto id = runtime.getProfilerID(block);                             \
    auto o = runtime.opcodeCount;                                       \
    runtime.functionEvents.emplace_back(id, hermes::rdtsc(), o, false); \
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
#define HERMESVM_RDTSC() hermes::rdtsc()
#else
#define HERMESVM_RDTSC() 0
#endif

#endif // HERMESVM_PROFILER_NATIVECALL

#endif // HERMES_VM_PROFILER_H
