/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_HERMES_H
#define HERMES_HERMES_H

#include <exception>
#include <list>
#include <map>
#include <memory>
#include <ostream>
#include <string>

#include <hermes/Public/HermesExport.h>
#include <hermes/Public/RuntimeConfig.h>
#include <jsi/jsi.h>
#include <unordered_map>

struct HermesTestHelper;

namespace hermes {
namespace vm {
class GCExecTrace;
struct MockedEnvironment;
} // namespace vm
} // namespace hermes

namespace facebook {
namespace jsi {

class ThreadSafeRuntime;

}

namespace hermes {

namespace debugger {
class Debugger;
}

class HermesRuntimeImpl;

/// Represents a Hermes JS runtime.
class HERMES_EXPORT HermesRuntime : public jsi::Runtime {
 public:
  static bool isHermesBytecode(const uint8_t *data, size_t len);
  // Returns the supported bytecode version.
  static uint32_t getBytecodeVersion();
  // (EXPERIMENTAL) Issues madvise calls for portions of the given
  // bytecode file that will likely be used when loading the bytecode
  // file and running its global function.
  static void prefetchHermesBytecode(const uint8_t *data, size_t len);
  // Returns whether the data is valid HBC with more extensive checks than
  // isHermesBytecode and returns why it isn't in errorMessage (if nonnull)
  // if not.
  static bool hermesBytecodeSanityCheck(
      const uint8_t *data,
      size_t len,
      std::string *errorMessage = nullptr);
  static void setFatalHandler(void (*handler)(const std::string &));

  // Assuming that \p data is valid HBC bytecode data, returns a pointer to the
  // first element of the epilogue, data append to the end of the bytecode
  // stream. Return pair contain ptr to data and header.
  static std::pair<const uint8_t *, size_t> getBytecodeEpilogue(
      const uint8_t *data,
      size_t len);

  /// Enable sampling profiler.
  static void enableSamplingProfiler();

  /// Disable the sampling profiler
  static void disableSamplingProfiler();

  /// Dump sampled stack trace to the given file name.
  static void dumpSampledTraceToFile(const std::string &fileName);

  /// Dump sampled stack trace to the given stream.
  static void dumpSampledTraceToStream(std::ostream &stream);

  /// Serialize the sampled stack to the format expected by DevTools'
  /// Profiler.stop return type.
  void sampledTraceToStreamInDevToolsFormat(std::ostream &stream);

  /// Return the executed JavaScript function info.
  /// This information holds the segmentID, Virtualoffset and sourceURL.
  /// This information is needed specifically to be able to symbolicate non-CJS
  /// bundles correctly. This API will be simplified later to simply return a
  /// segmentID and virtualOffset, when we are able to only support CJS bundles.
  static std::unordered_map<std::string, std::vector<std::string>>
  getExecutedFunctions();

  /// \return whether code coverage profiler is enabled or not.
  static bool isCodeCoverageProfilerEnabled();

  /// Enable code coverage profiler.
  static void enableCodeCoverageProfiler();

  /// Disable code coverage profiler.
  static void disableCodeCoverageProfiler();

  // The base class declares most of the interesting methods.  This
  // just declares new methods which are specific to HermesRuntime.
  // The actual implementations of the pure virtual methods are
  // provided by a class internal to the .cpp file, which is created
  // by the factory.

  /// Load a new segment into the Runtime.
  /// The \param context must be a valid RequireContext retrieved from JS
  /// using `require.context`.
  void loadSegment(
      std::unique_ptr<const jsi::Buffer> buffer,
      const jsi::Value &context);

  /// Gets a guaranteed unique id for an Object (or, respectively, String
  /// or PropNameId), which is assigned at allocation time and is
  /// static throughout that object's (or string's, or PropNameID's)
  /// lifetime.
  uint64_t getUniqueID(const jsi::Object &o) const;
  uint64_t getUniqueID(const jsi::BigInt &s) const;
  uint64_t getUniqueID(const jsi::String &s) const;
  uint64_t getUniqueID(const jsi::PropNameID &pni) const;
  uint64_t getUniqueID(const jsi::Symbol &sym) const;

  /// Same as the other \c getUniqueID, except it can return 0 for some values.
  /// 0 means there is no ID associated with the value.
  uint64_t getUniqueID(const jsi::Value &val) const;

  /// From an ID retrieved from \p getUniqueID, go back to the object.
  /// NOTE: This is much slower in general than the reverse operation, and takes
  /// up more memory. Don't use this unless it's absolutely necessary.
  /// \return a jsi::Object if a matching object is found, else returns null.
  jsi::Value getObjectForID(uint64_t id);

  /// Get a structure representing the environment-dependent behavior, so
  /// it can be written into the trace for later replay.
  const ::hermes::vm::MockedEnvironment &getMockedEnvironment() const;

  /// Get a structure representing the execution history (currently just of
  /// GC, but will be generalized as necessary), to aid in debugging
  /// non-deterministic execution.
  const ::hermes::vm::GCExecTrace &getGCExecTrace() const;

  /// Make the runtime read from \p env to replay its environment-dependent
  /// behavior.
  void setMockedEnvironment(const ::hermes::vm::MockedEnvironment &env);

  /// Get IO tracking (aka HBC page access) info as a JSON string.
  /// See hermes::vm::Runtime::getIOTrackingInfoJSON() for conditions
  /// needed for there to be useful output.
  std::string getIOTrackingInfoJSON();

#ifdef HERMESVM_PROFILER_BB
  /// Write the trace to the given stream.
  void dumpBasicBlockProfileTrace(std::ostream &os) const;
#endif

#ifdef HERMESVM_PROFILER_OPCODE
  /// Write the opcode stats to the given stream.
  void dumpOpcodeStats(std::ostream &os) const;
#endif

  /// \return a reference to the Debugger for this Runtime.
  debugger::Debugger &getDebugger();

#ifdef HERMES_ENABLE_DEBUGGER

  struct DebugFlags {
    // Looking for the .lazy flag? It's no longer necessary.
    // Source is evaluated lazily by default. See
    // RuntimeConfig::CompilationMode.
  };

  /// Evaluate the given code in an unoptimized form,
  /// used for debugging.
  void debugJavaScript(
      const std::string &src,
      const std::string &sourceURL,
      const DebugFlags &debugFlags);
#endif

  /// Register this runtime for sampling profiler.
  void registerForProfiling();
  /// Unregister this runtime for sampling profiler.
  void unregisterForProfiling();

  /// Define methods to interrupt JS execution and set time limits.
  /// All JS compiled to bytecode via prepareJS, or evaluateJS, will support
  /// interruption and time limit monitoring if the runtime is configured with
  /// AsyncBreakCheckInEval. If JS prepared in other ways is executed, care must
  /// be taken to ensure that it is compiled in a mode that supports it (i.e.,
  /// the emitted code contains async break checks).

  /// Asynchronously terminates the current execution. This can be called on
  /// any thread.
  void asyncTriggerTimeout();

  /// Register this runtime for execution time limit monitoring, with a time
  /// limit of \p timeoutInMs milliseconds.
  /// See compilation notes above.
  void watchTimeLimit(uint32_t timeoutInMs);
  /// Unregister this runtime for execution time limit monitoring.
  void unwatchTimeLimit();

  /// Same as \c evaluate JavaScript but with a source map, which will be
  /// applied to exception traces and debug information.
  ///
  /// This is an experimental Hermes-specific API. In the future it may be
  /// renamed, moved or combined with another API, but the provided
  /// functionality will continue to be available in some form.
  jsi::Value evaluateJavaScriptWithSourceMap(
      const std::shared_ptr<const jsi::Buffer> &buffer,
      const std::shared_ptr<const jsi::Buffer> &sourceMapBuf,
      const std::string &sourceURL);

 private:
  // Only HermesRuntimeImpl can subclass this.
  HermesRuntime() = default;
  friend class HermesRuntimeImpl;

  friend struct ::HermesTestHelper;
  size_t rootsListLengthForTests() const;

  // Do not add any members here.  This ensures that there are no
  // object size inconsistencies.  All data should be in the impl
  // class in the .cpp file.
};

/// Return a RuntimeConfig that is more suited for running untrusted JS than
/// the default config. Disables some language features and may trade off some
/// performance for security.
///
/// Can serve as a starting point with tweaks to re-enable needed features:
///   auto conf = hardenedHermesRuntimeConfig().rebuild();
///   conf.withArrayBuffer(true);
///   ...
///   auto runtime = makeHermesRuntime(conf.build());
HERMES_EXPORT ::hermes::vm::RuntimeConfig hardenedHermesRuntimeConfig();

HERMES_EXPORT std::unique_ptr<HermesRuntime> makeHermesRuntime(
    const ::hermes::vm::RuntimeConfig &runtimeConfig =
        ::hermes::vm::RuntimeConfig());
HERMES_EXPORT std::unique_ptr<jsi::ThreadSafeRuntime>
makeThreadSafeHermesRuntime(
    const ::hermes::vm::RuntimeConfig &runtimeConfig =
        ::hermes::vm::RuntimeConfig());
} // namespace hermes
} // namespace facebook

#endif
