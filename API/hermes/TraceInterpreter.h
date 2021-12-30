/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <hermes/Public/RuntimeConfig.h>
#include <hermes/Support/OptValue.h>
#include <hermes/Support/SHA1.h>
#include <hermes/SynthTrace.h>

#include <jsi/jsi.h>
#include <llvh/Support/MemoryBuffer.h>
#include <llvh/Support/raw_ostream.h>

#include <map>
#include <unordered_map>
#include <vector>

namespace facebook {
namespace hermes {

namespace tracing {

class TraceInterpreter final {
 public:
  /// A DefAndUse details the location of a definition of an object id, and its
  /// use. It is an index into the global record table.
  struct DefAndUse {
    /// If an object was not used or not defined, its DefAndUse can store this
    /// value.
    static constexpr uint64_t kUnused = std::numeric_limits<uint64_t>::max();

    uint64_t lastDefBeforeFirstUse{kUnused};
    uint64_t lastUse{kUnused};
  };

  /// A Call is a list of Pieces that represent the entire single call
  /// frame, even if it spans multiple control transfers between JS and native.
  /// It also contains a map from ObjectIDs to their last definition before a
  /// first use, and a last use.
  struct Call {
    /// A Piece is a series of contiguous records that are part of the same
    /// native call, and have no transitions to JS in the middle of them.
    struct Piece {
      /// The index of the start of the piece in the global record vector.
      uint64_t start;
      std::vector<const SynthTrace::Record *> records;

      explicit Piece() : start(0) {}
      explicit Piece(int64_t start) : start(start) {}
    };

    /// A list of pieces, where each piece stops when a transition occurs
    /// between JS and Native. Pieces are guaranteed to be sorted according to
    /// their start record (ascending).
    std::vector<Piece> pieces;
    std::unordered_map<SynthTrace::ObjectID, DefAndUse> locals;

    explicit Call() = delete;
    explicit Call(const Piece &piece) {
      pieces.emplace_back(piece);
    }
    explicit Call(Piece &&piece) {
      pieces.emplace_back(std::move(piece));
    }
  };

  /// A HostFunctionToCalls is a mapping from a host function id to the list of
  /// calls associated with that host function's execution. The calls are
  /// ordered by invocation (the 0th element is the 1st call).
  using HostFunctionToCalls =
      std::unordered_map<SynthTrace::ObjectID, std::vector<Call>>;

  /// A PropNameToCalls is a mapping from property names to a list of
  /// calls on that property. The calls are ordered by invocation (the 0th
  /// element is the 1st call).
  using PropNameToCalls = std::unordered_map<std::string, std::vector<Call>>;

  struct HostObjectInfo final {
    explicit HostObjectInfo() = default;

    PropNameToCalls propNameToCalls;
    std::vector<Call> callsToGetPropertyNames;
    std::vector<std::vector<std::string>> resultsOfGetPropertyNames;
  };

  /// A HostObjectToCalls is a mapping from a host object id to the
  /// mapping of property names to calls associated with accessing properties of
  /// that host object and the list of calls associated with getPropertyNames.
  using HostObjectToCalls =
      std::unordered_map<SynthTrace::ObjectID, HostObjectInfo>;

  /// Options for executing the trace.
  struct ExecuteOptions {
    /// Customizes the GCConfig of the Runtime.
    ::hermes::vm::GCConfig::Builder gcConfigBuilder;

    /// If true, trace again while replaying. After normalization (see
    /// hermes/tools/synth/trace_normalize.py) the output trace should be
    /// identical to the input trace. If they're not, there was a bug in replay.
    mutable bool traceEnabled{false};

    /// If true, command-line options override the config options recorded in
    /// the trace.  If false, start from the default config.
    bool useTraceConfig{false};

    /// Number of initial executions whose stats are discarded.
    int warmupReps{0};

    /// Number of repetitions of execution. Stats returned are those for the rep
    /// with the median totalTime.
    int reps{1};

    /// If true, run a complete collection before printing stats. Useful for
    /// guaranteeing there's no garbage in heap size numbers.
    bool forceGCBeforeStats{false};

    /// If true, make attempts to make the instruction count more stable. Useful
    /// for using a tool like PIN to count instructions and compare runs.
    bool stabilizeInstructionCount{false};

    /// If true, remove the requirement that the input bytecode was compiled
    /// from the same source used to record the trace. There must only be one
    /// input bytecode file in this case. If its observable behavior deviates
    /// from the trace, the results are undefined.
    bool disableSourceHashCheck{false};

    /// A trace contains many MarkerRecords which have a name used to identify
    /// them. If the replay encounters this given marker, perform an action
    /// described by MarkerAction. All actions will stop the trace early and
    /// collect stats at the marker point, unless the marker is set to the
    /// special marker "end". In that case the trace will run to completion.
    std::string marker{"end"};

    enum class MarkerAction {
      NONE,
      /// Take a snapshot at marker.
      SNAPSHOT,
      /// Take a heap timeline that ends at marker.
      TIMELINE,
      /// Take a sampling heap profile that ends at marker.
      SAMPLE_MEMORY,
      /// Take a sampling time profile that ends at marker.
      SAMPLE_TIME,
    };

    /// Sets the action to take upon encountering the marker. The action will
    /// write results into the \p profileFileName.
    MarkerAction action{MarkerAction::NONE};

    /// Output file name for any profiling information.
    std::string profileFileName;

    // These are the config parameters.  We wrap them in llvh::Optional
    // to indicate whether the corresponding command line flag was set
    // explicitly.  We override the trace's config only when that is true.

    /// If true, track all disk I/O done by the runtime and print a report at
    /// the end to stdout.
    llvh::Optional<bool> shouldTrackIO;

    /// If present, do a bytecode warmup run that touches a percentage of the
    /// bytecode. A value of 50 here means 50% of the bytecode should be warmed.
    llvh::Optional<unsigned> bytecodeWarmupPercent;
  };

 private:
  jsi::Runtime &rt_;
  ExecuteOptions options_;
  llvh::raw_ostream *traceStream_;
  // Map from source hash to source file to run.
  std::map<::hermes::SHA1, std::shared_ptr<const jsi::Buffer>> bundles_;
  const SynthTrace &trace_;
  const std::unordered_map<SynthTrace::ObjectID, DefAndUse> &globalDefsAndUses_;
  const HostFunctionToCalls &hostFunctionCalls_;
  const HostObjectToCalls &hostObjectCalls_;
  std::unordered_map<SynthTrace::ObjectID, jsi::Function> hostFunctions_;
  std::unordered_map<SynthTrace::ObjectID, uint64_t> hostFunctionsCallCount_;
  // NOTE: Theoretically a host object property can have both a getter and a
  // setter. Since this doesn't occur in practice currently, this
  // implementation will ignore it. If it does happen, the value of the
  // interior map should turn into a pair of functions, and a pair of function
  // counts.
  std::unordered_map<SynthTrace::ObjectID, jsi::Object> hostObjects_;
  std::unordered_map<
      SynthTrace::ObjectID,
      std::unordered_map<std::string, uint64_t>>
      hostObjectsCallCount_;
  std::unordered_map<SynthTrace::ObjectID, uint64_t>
      hostObjectsPropertyNamesCallCount_;

  // Invariant: the value is either jsi::Object or jsi::String.
  std::unordered_map<SynthTrace::ObjectID, jsi::Value> gom_;
  // For the PropNameIDs, which are not representable as jsi::Value.
  std::unordered_map<SynthTrace::ObjectID, jsi::PropNameID> gpnm_;

  std::string stats_;
  /// Whether the marker was reached.
  bool markerFound_{false};
  /// Depth in the execution stack. Zero is the outermost function.
  uint64_t depth_{0};

 public:
  /// Execute the trace given by \p traceFile, that was the trace of executing
  /// the bundle given by \p bytecodeFile.
  static void exec(
      const std::string &traceFile,
      const std::vector<std::string> &bytecodeFiles,
      const ExecuteOptions &options);

  /// Same as exec, except it prints out the stats of a run.
  /// \return The stats collected by the runtime about times and memory usage.
  static std::string execAndGetStats(
      const std::string &traceFile,
      const std::vector<std::string> &bytecodeFiles,
      const ExecuteOptions &options);

  /// Same as exec, except it additionally traces the execution of the
  /// interpreter, to \p *traceStream.  (Requires \p traceStream to be
  /// non-null.)  This trace can be compared to the original to detect
  /// correctness issues.
  static void execAndTrace(
      const std::string &traceFile,
      const std::vector<std::string> &bytecodeFiles,
      const ExecuteOptions &options,
      std::unique_ptr<llvh::raw_ostream> traceStream);

  static ::hermes::vm::RuntimeConfig merge(
      ::hermes::vm::RuntimeConfig::Builder &,
      const ::hermes::vm::GCConfig::Builder &,
      const ExecuteOptions &,
      bool,
      bool);

  /// \param traceStream If non-null, write a trace of the execution into this
  /// stream.
  static std::string execFromMemoryBuffer(
      std::unique_ptr<llvh::MemoryBuffer> &&traceBuf,
      std::vector<std::unique_ptr<llvh::MemoryBuffer>> &&codeBufs,
      const ExecuteOptions &options,
      std::unique_ptr<llvh::raw_ostream> traceStream);

  /// For test purposes, use the given runtime, execute once.
  /// Otherwise like execFromMemoryBuffer above.
  static std::string execFromMemoryBuffer(
      std::unique_ptr<llvh::MemoryBuffer> &&traceBuf,
      std::vector<std::unique_ptr<llvh::MemoryBuffer>> &&codeBufs,
      jsi::Runtime &runtime,
      const ExecuteOptions &options);

 private:
  TraceInterpreter(
      jsi::Runtime &rt,
      const ExecuteOptions &options,
      const SynthTrace &trace,
      std::map<::hermes::SHA1, std::shared_ptr<const jsi::Buffer>> bundles,
      const std::unordered_map<SynthTrace::ObjectID, DefAndUse>
          &globalDefsAndUses,
      const HostFunctionToCalls &hostFunctionCalls,
      const HostObjectToCalls &hostObjectCalls);

  static std::string execFromFileNames(
      const std::string &traceFile,
      const std::vector<std::string> &bytecodeFiles,
      const ExecuteOptions &options,
      std::unique_ptr<llvh::raw_ostream> traceStream);

  static std::string exec(
      jsi::Runtime &rt,
      const ExecuteOptions &options,
      const SynthTrace &trace,
      std::map<::hermes::SHA1, std::shared_ptr<const jsi::Buffer>> bundles);

  /// Requires \p codeBufs to be the memory buffers containing the code
  /// referenced (via source hash) by the given \p trace.  Returns a map from
  /// the source hash to the memory buffer.  In addition, if \p codeIsMmapped is
  /// non-null, sets \p *codeIsMmapped to indicate whether all the code is
  /// mmapped, and, if \p isBytecode is non-null, sets \p *isBytecode
  /// to indicate whether all the code is bytecode.
  static std::map<::hermes::SHA1, std::shared_ptr<const jsi::Buffer>>
  getSourceHashToBundleMap(
      std::vector<std::unique_ptr<llvh::MemoryBuffer>> &&codeBufs,
      const SynthTrace &trace,
      const ExecuteOptions &options,
      bool *codeIsMmapped = nullptr,
      bool *isBytecode = nullptr);

  jsi::Function createHostFunction(
      const SynthTrace::CreateHostFunctionRecord &rec,
      const jsi::PropNameID &propNameID);

  jsi::Object createHostObject(SynthTrace::ObjectID objID);

  std::string execEntryFunction(const Call &entryFunc);

  // Execute \p entryFunc on the given \p thisVal and the \p count
  // arguments \p args.  If the first record should be treated as a
  // definition of a propNameID used in the function, \p
  // nativePropNameToConsumeAsDef will be non-null, and will point to
  // the jsi::PropNameID that is the runtime value for the prop name.
  jsi::Value execFunction(
      const Call &entryFunc,
      const jsi::Value &thisVal,
      const jsi::Value *args,
      uint64_t count,
      const jsi::PropNameID *nativePropNameToConsumeAsDef = nullptr);

  /// Requires that \p valID is the proper id for \p val, and that a
  /// defining occurence of \p valID occurs at the given \p
  /// globalRecordNum.  Decides whether the definition should be
  /// recorded, locally in \p call, or globally, and, if so, adds the
  /// association between \p valID and \p val to \p locals or \p
  /// globals, as appropriate.
  template <typename ValueType>
  void addValueToDefs(
      const Call &call,
      SynthTrace::ObjectID valID,
      uint64_t globalRecordNum,
      const ValueType &val,
      std::unordered_map<SynthTrace::ObjectID, ValueType> &locals,
      std::unordered_map<SynthTrace::ObjectID, ValueType> &globals);

  /// Same as above, except it avoids copies on temporary objects.
  template <typename ValueType>
  void addValueToDefs(
      const Call &call,
      SynthTrace::ObjectID valID,
      uint64_t globalRecordNum,
      ValueType &&val,
      std::unordered_map<SynthTrace::ObjectID, ValueType> &locals,
      std::unordered_map<SynthTrace::ObjectID, ValueType> &globals);

  /// Requires that \p valID is the proper id for \p val, and that a
  /// defining occurence of \p key occurs at the given \p
  /// globalRecordNum.  Decides whether the definition should be
  /// recorded, locally in \p call, or globally, and, if so, adds the
  /// association between \p key and \p val to \p locals or \p
  /// globals, as appropriate.
  void addJSIValueToDefs(
      const Call &call,
      SynthTrace::ObjectID valID,
      uint64_t globalRecordNum,
      const jsi::Value &val,
      std::unordered_map<SynthTrace::ObjectID, jsi::Value> &locals) {
    addValueToDefs<jsi::Value>(call, valID, globalRecordNum, val, locals, gom_);
  }

  /// Same as above, except it avoids copies on temporary objects.
  void addJSIValueToDefs(
      const Call &call,
      SynthTrace::ObjectID valID,
      uint64_t globalRecordNum,
      jsi::Value &&val,
      std::unordered_map<SynthTrace::ObjectID, jsi::Value> &locals) {
    addValueToDefs<jsi::Value>(call, valID, globalRecordNum, val, locals, gom_);
  }

  /// Requires that \p valID is the proper id for \p propNameID, and
  /// that a defining occurence of \p propNameID occurs at the given
  /// \p globalRecordNum.  Decides whether the definition should be
  /// recorded, locally in \p call, or globally, and, if so, adds the
  /// association between \p propNameID and \p val to \p locals or \p
  /// globals, as appropriate.
  void addPropNameIDToDefs(
      const Call &call,
      SynthTrace::ObjectID valID,
      uint64_t globalRecordNum,
      const jsi::PropNameID &propNameID,
      std::unordered_map<SynthTrace::ObjectID, jsi::PropNameID> &locals) {
    addValueToDefs<jsi::PropNameID>(
        call, valID, globalRecordNum, propNameID, locals, gpnm_);
  }

  /// Same as above, except it avoids copies on temporary objects.
  void addPropNameIDToDefs(
      const Call &call,
      SynthTrace::ObjectID valID,
      uint64_t globalRecordNum,
      jsi::PropNameID &&propNameID,
      std::unordered_map<SynthTrace::ObjectID, jsi::PropNameID> &locals) {
    addValueToDefs<jsi::PropNameID>(
        call, valID, globalRecordNum, propNameID, locals, gpnm_);
  }

  /// If \p traceValue specifies an Object or String, requires \p
  /// val to be of the corresponding runtime type.  Adds this
  /// occurrence at \p globalRecordNum as a local or global definition
  /// in \p locals or the global object map, respectively.
  ///
  /// \p isThis should be true if and only if the value is a 'this' in a call
  /// (only used for validation). TODO(T84791675): Remove this parameter.
  ///
  /// N.B. This method should be called even if you happen to know that the
  /// value cannot be an Object or String, since it performs useful validation.
  void ifObjectAddToDefs(
      const SynthTrace::TraceValue &traceValue,
      const jsi::Value &val,
      const Call &call,
      uint64_t globalRecordNum,
      std::unordered_map<SynthTrace::ObjectID, jsi::Value> &locals,
      bool isThis = false);

  /// Same as above, except it avoids copies on temporary objects.
  void ifObjectAddToDefs(
      const SynthTrace::TraceValue &traceValue,
      jsi::Value &&val,
      const Call &call,
      uint64_t globalRecordNum,
      std::unordered_map<SynthTrace::ObjectID, jsi::Value> &locals,
      bool isThis = false);

  /// Check if the \p marker is the one that is being searched for. If this is
  /// the first time encountering the matching marker, perform the actions set
  /// up for that marker.
  void checkMarker(const std::string &marker);

  std::string printStats();

  LLVM_ATTRIBUTE_NORETURN void crashOnException(
      const std::exception &e,
      ::hermes::OptValue<uint64_t> globalRecordNum);
};

} // namespace tracing
} // namespace hermes
} // namespace facebook
