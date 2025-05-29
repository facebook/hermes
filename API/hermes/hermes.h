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
#include <hermes/Public/SamplingProfiler.h>
#include <jsi/jsi.h>
#include <unordered_map>

struct HermesTestHelper;
struct SHUnit;
struct SHRuntime;

namespace hermes {
namespace vm {
class GCExecTrace;
class Runtime;
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

class HermesRuntime;
/// The Hermes Root API interface. This is the entry point to create the Hermes
/// runtime and to access Hermes-specific methods that do not rely on a runtime
/// instance.
class HERMES_EXPORT IHermesRootAPI : public jsi::ICast {
 public:
  static constexpr jsi::UUID uuid{
      0xb654d898,
      0xdfad,
      0x11ef,
      0x859a,
      0x325096b39f47};

  // Returns an instance of Hermes Runtime.
  virtual std::unique_ptr<HermesRuntime> makeHermesRuntime(
      const ::hermes::vm::RuntimeConfig &runtimeConfig) = 0;

  virtual bool isHermesBytecode(const uint8_t *data, size_t len) = 0;

  // Returns the supported bytecode version.
  virtual uint32_t getBytecodeVersion() = 0;

  // (EXPERIMENTAL) Issues madvise calls for portions of the given
  // bytecode file that will likely be used when loading the bytecode
  // file and running its global function.
  virtual void prefetchHermesBytecode(const uint8_t *data, size_t len) = 0;

  // Returns whether the data is valid HBC with more extensive checks than
  // isHermesBytecode and returns why it isn't in errorMessage (if nonnull)
  // if not.
  virtual bool hermesBytecodeSanityCheck(
      const uint8_t *data,
      size_t len,
      std::string *errorMessage = nullptr) = 0;

  /// Sets a global fatal handler that is shared across all active Hermes
  /// runtimes. Setting fatal handler in multiple places will override the
  /// previous fatal handler set by this functionality.
  /// The fatal handler must not throw exceptions, as Hermes is compiled without
  /// exceptions.
  virtual void setFatalHandler(void (*handler)(const std::string &)) = 0;

  // Assuming that \p data is valid HBC bytecode data, returns a pointer to the
  // first element of the epilogue, data append to the end of the bytecode
  // stream. Return pair contain ptr to data and header.
  virtual std::pair<const uint8_t *, size_t> getBytecodeEpilogue(
      const uint8_t *data,
      size_t len) = 0;

  /// Enable sampling profiler.
  /// Starts a separate thread that polls VM state with \p meanHzFreq frequency.
  /// Any subsequent call to \c enableSamplingProfiler() is ignored until
  /// next call to \c disableSamplingProfiler()
  virtual void enableSamplingProfiler(double meanHzFreq = 100) = 0;

  /// Disable the sampling profiler
  virtual void disableSamplingProfiler() = 0;

  /// Dump sampled stack trace to the given file name.
  virtual void dumpSampledTraceToFile(const std::string &fileName) = 0;

  /// Dump sampled stack trace to the given stream.
  virtual void dumpSampledTraceToStream(std::ostream &stream) = 0;

  /// Return the executed JavaScript function info.
  /// This information holds the segmentID, Virtualoffset and sourceURL.
  /// This information is needed specifically to be able to symbolicate non-CJS
  /// bundles correctly. This API will be simplified later to simply return a
  /// segmentID and virtualOffset, when we are able to only support CJS bundles.
  virtual std::unordered_map<std::string, std::vector<std::string>>
  getExecutedFunctions() = 0;

  /// \return whether code coverage profiler is enabled or not.
  virtual bool isCodeCoverageProfilerEnabled() = 0;

  /// Enable code coverage profiler.
  virtual void enableCodeCoverageProfiler() = 0;

  /// Disable code coverage profiler.
  virtual void disableCodeCoverageProfiler() = 0;

 protected:
  /// The destructor is protected as delete calls on interfaces must not occur.
  /// It is also non-virtual to simplify the v-table.
  ~IHermesRootAPI() {}
};

/// The setFatalHandler functionality has global effects, which may cause
/// unintended or surprising behavior for users of this API. For this reason, it
/// is not recommended and the functionality is provided by the optional
/// interface ISetFatalHandler.
class HERMES_EXPORT ISetFatalHandler : public jsi::ICast {
 public:
  static constexpr jsi::UUID uuid{
      0xda98a610,
      0x09cb,
      0x11f0,
      0x87bf,
      0x325096b39f47};
  /// Sets a global fatal handler that is shared across all active Hermes
  /// runtimes. Setting fatal handler in multiple places will override the
  /// previous fatal handler set by this functionality.
  /// The fatal handler must not throw exceptions, as Hermes is compiled without
  /// exceptions.
  virtual void setFatalHandler(void (*handler)(const std::string &)) = 0;

 protected:
  ~ISetFatalHandler() = default;
};

/// Interface for Hermes-specific runtime methods.The actual implementations of
/// the pure virtual methods are provided by a class internal to the .cpp file,
/// which is created by the factory.
class HERMES_EXPORT IHermes : public jsi::ICast {
 public:
  static constexpr jsi::UUID uuid{
      0xe85cfa22,
      0xdfae,
      0x11ef,
      0xa6f7,
      0x325096b39f47};

  /// Return a ICast pointer to an object that be cast into the interface
  /// IHermesRootAPI. This root API object has static lifetime.
  virtual ICast *getHermesRootAPI() = 0;

  /// Serialize the sampled stack to the format expected by DevTools'
  /// Profiler.stop return type.
  virtual void sampledTraceToStreamInDevToolsFormat(std::ostream &stream) = 0;

  /// Dump sampled stack trace for a given runtime to a data structure that can
  /// be used by third parties.
  virtual sampling_profiler::Profile dumpSampledTraceToProfile() = 0;

  /// Resets the timezone offset cache used by Hermes for performance
  /// optimization. Hermes maintains a cached timezone offset to accelerate date
  /// and time calculations. However, this cache does not automatically detect
  /// changes to the system timezone. When the system timezone changes, the
  /// integration layer (e.g., React Native) must call this method to invalidate
  /// the cache and ensure correct time calculations.
  ///
  /// \note Call this method immediately after detecting any timezone change in
  /// the integrator.
  virtual void resetTimezoneCache() = 0;

  /// Load a new segment into the Runtime.
  /// The \param context must be a valid RequireContext retrieved from JS
  /// using `require.context`.
  virtual void loadSegment(
      std::unique_ptr<const jsi::Buffer> buffer,
      const jsi::Value &context) = 0;

  /// Gets a guaranteed unique id for an Object (or, respectively, String
  /// or PropNameId), which is assigned at allocation time and is
  /// static throughout that object's (or string's, or PropNameID's)
  /// lifetime.
  virtual uint64_t getUniqueID(const jsi::Object &o) const = 0;
  virtual uint64_t getUniqueID(const jsi::BigInt &s) const = 0;
  virtual uint64_t getUniqueID(const jsi::String &s) const = 0;
  virtual uint64_t getUniqueID(const jsi::PropNameID &pni) const = 0;
  virtual uint64_t getUniqueID(const jsi::Symbol &sym) const = 0;

  /// Same as the other \c getUniqueID, except it can return 0 for some values.
  /// 0 means there is no ID associated with the value.
  virtual uint64_t getUniqueID(const jsi::Value &val) const = 0;

  /// From an ID retrieved from \p getUniqueID, go back to the object.
  /// NOTE: This is much slower in general than the reverse operation, and takes
  /// up more memory. Don't use this unless it's absolutely necessary.
  /// \return a jsi::Object if a matching object is found, else returns null.
  virtual jsi::Value getObjectForID(uint64_t id) = 0;

  /// Get a structure representing the execution history (currently just of
  /// GC, but will be generalized as necessary), to aid in debugging
  /// non-deterministic execution.
  virtual const ::hermes::vm::GCExecTrace &getGCExecTrace() const = 0;

  /// Get IO tracking (aka HBC page access) info as a JSON string.
  /// See hermes::vm::Runtime::getIOTrackingInfoJSON() for conditions
  /// needed for there to be useful output.
  virtual std::string getIOTrackingInfoJSON() = 0;

#ifdef HERMESVM_PROFILER_BB
  /// Write the trace to the given stream.
  virtual void dumpBasicBlockProfileTrace(std::ostream &os) const = 0;
#endif

#ifdef HERMESVM_PROFILER_OPCODE
  /// Write the opcode stats to the given stream.
  virtual void dumpOpcodeStats(std::ostream &os) const = 0;
#endif

  /// \return a reference to the Debugger for this Runtime.
  virtual debugger::Debugger &getDebugger() = 0;

#ifdef HERMES_ENABLE_DEBUGGER

  struct DebugFlags {
    // Looking for the .lazy flag? It's no longer necessary.
    // Source is evaluated lazily by default. See
    // RuntimeConfig::CompilationMode.
  };

  /// Evaluate the given code in an unoptimized form,
  /// used for debugging.
  virtual void debugJavaScript(
      const std::string &src,
      const std::string &sourceURL,
      const DebugFlags &debugFlags) = 0;
#endif

  /// Register this runtime and thread for sampling profiler. Before using the
  /// runtime on another thread, invoke this function again from the new thread
  /// to make the sampling profiler target the new thread (and forget the old
  /// thread).
  virtual void registerForProfiling() = 0;
  /// Unregister this runtime for sampling profiler.
  virtual void unregisterForProfiling() = 0;

  /// Define methods to interrupt JS execution and set time limits.
  /// All JS compiled to bytecode via prepareJS, or evaluateJS, will support
  /// interruption and time limit monitoring if the runtime is configured with
  /// AsyncBreakCheckInEval. If JS prepared in other ways is executed, care must
  /// be taken to ensure that it is compiled in a mode that supports it (i.e.,
  /// the emitted code contains async break checks).

  /// Asynchronously terminates the current execution. This can be called on
  /// any thread.
  virtual void asyncTriggerTimeout() = 0;

  /// Register this runtime for execution time limit monitoring, with a time
  /// limit of \p timeoutInMs milliseconds.
  /// See compilation notes above.
  virtual void watchTimeLimit(uint32_t timeoutInMs) = 0;
  /// Unregister this runtime for execution time limit monitoring.
  virtual void unwatchTimeLimit() = 0;

  /// Same as \c evaluate JavaScript but with a source map, which will be
  /// applied to exception traces and debug information.
  ///
  /// This is an experimental Hermes-specific API. In the future it may be
  /// renamed, moved or combined with another API, but the provided
  /// functionality will continue to be available in some form.
  virtual jsi::Value evaluateJavaScriptWithSourceMap(
      const std::shared_ptr<const jsi::Buffer> &buffer,
      const std::shared_ptr<const jsi::Buffer> &sourceMapBuf,
      const std::string &sourceURL) = 0;

  /// Provided for compatibility with Static Hermes, but should not be called.
  virtual jsi::Value evaluateSHUnit(SHUnit *(*shUnitCreator)()) = 0;
  virtual SHRuntime *getSHRuntime() noexcept = 0;

  /// Returns the underlying low level Hermes VM runtime instance.
  /// This function is considered unsafe and unstable.
  /// Direct use of a vm::Runtime should be avoided as the lower level APIs are
  /// unsafe and they can change without notice.
  virtual void *getVMRuntimeUnsafe() const = 0;

 protected:
  ~IHermes() = default;
};

/// Interface for methods that are exposed for test purposes.
class HERMES_EXPORT IHermesTestHelpers : public jsi::ICast {
 public:
  static constexpr jsi::UUID uuid{
      0x664e489a,
      0xf941,
      0x11ef,
      0xa44c,
      0x325096b39f47};

  virtual size_t rootsListLengthForTests() const = 0;

 protected:
  ~IHermesTestHelpers() = default;
};

class HermesRuntime : public jsi::Runtime, public IHermes {
 public:
  /// Similar to jsi::Runtime, HermesRuntime is treated as an object, rather
  /// than a pure interface. This is to prevent breaking usages of
  /// HermesRuntime prior to the introduction of jsi::IRuntime, IHermes, and
  /// other interfaces.
  ~HermesRuntime() override = default;

  using jsi::Runtime::castInterface;
};

/// Returns a pointer to an object that can be cast into IHermesRootAPI, which
/// can be used to create a Hermes runtime and to access global Hermes-specific
/// methods. This object has static lifetime.
HERMES_EXPORT jsi::ICast *makeHermesRootAPI();

/// Return a RuntimeConfig that is more suited for running untrusted JS than
/// the default config. Disables some language features and may trade off some
/// performance for security.
///
/// Can serve as a starting point with tweaks to re-enable needed features:
///   auto conf = hardenedHermesRuntimeConfig().rebuild();
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
