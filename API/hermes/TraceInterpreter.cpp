/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <hermes/TraceInterpreter.h>

#include <hermes/BCGen/HBC/BytecodeDataProvider.h>
#include <hermes/SynthTraceParser.h>
#include <hermes/TracingRuntime.h>
#include <hermes/VM/instrumentation/PerfEvents.h>
#include <jsi/instrumentation.h>
#include <llvh/Support/SHA1.h>
#include <llvh/Support/SaveAndRestore.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>

#define TRACE_EXPECT_EQ(expected, actual)                                 \
  do {                                                                    \
    if (options_.verificationEnabled && (expected) != (actual)) {         \
      std::cerr << "Expected: " << (expected) << ", Actual: " << (actual) \
                << std::endl;                                             \
      std::cerr << "Synth Trace Verification failed at record "           \
                << nextExecIndex_ - 1 << std::endl;                       \
      abort();                                                            \
    }                                                                     \
  } while (0)

#define TRACE_EXPECT_TRUE(actual) TRACE_EXPECT_EQ(true, actual)

using namespace hermes::parser;
using namespace facebook::jsi;

namespace facebook {
namespace hermes {
namespace tracing {

using RecordType = SynthTrace::RecordType;
using ObjectID = SynthTrace::ObjectID;

namespace {

/// Cast a record from type \p FromType to type \p ToType, and assert that it
/// has the correct tag. Note that this can only be used for "final" types,
/// since it just compares the type tags.
template <typename ToType, typename FromType>
ToType &record_cast(FromType &rec) {
  assert(ToType::type == rec.getType());
  return static_cast<ToType &>(rec);
}

std::pair<
    std::unordered_map<ObjectID, uint64_t>,
    std::vector<std::pair<uint64_t, ObjectID>>>
createLastUseMaps(
    const std::vector<std::unique_ptr<SynthTrace::Record>> &records) {
  std::unordered_map<ObjectID, uint64_t> lastUsePerObj;
  std::vector<std::pair<uint64_t, ObjectID>> lastUses;
  for (int64_t i = records.size() - 1; i >= 0; i--) {
    const SynthTrace::Record *rec = records[i].get();
    for (const auto &objID : rec->uses()) {
      if (lastUsePerObj.find(objID) == lastUsePerObj.end()) {
        lastUsePerObj.insert({objID, i});
        lastUses.push_back({i, objID});
      }
    }
  }

  std::reverse(lastUses.begin(), lastUses.end());
  return {lastUsePerObj, lastUses};
}

std::unique_ptr<const jsi::Buffer> bufConvert(
    std::unique_ptr<llvh::MemoryBuffer> buf) {
  // A jsi::Buffer adapter that owns a llvh::MemoryBuffer.
  class OwnedMemoryBuffer : public jsi::Buffer {
   public:
    OwnedMemoryBuffer(std::unique_ptr<llvh::MemoryBuffer> buffer)
        : data_(std::move(buffer)) {}
    size_t size() const override {
      return data_->getBufferSize();
    }
    const uint8_t *data() const override {
      return reinterpret_cast<const uint8_t *>(data_->getBufferStart());
    }

   private:
    std::unique_ptr<llvh::MemoryBuffer> data_;
  };

  return std::make_unique<const OwnedMemoryBuffer>(std::move(buf));
}

static bool isAllZeroSourceHash(const ::hermes::SHA1 sourceHash) {
  for (auto byte : sourceHash) {
    if (byte) {
      return false;
    }
  }
  return true;
}

static void verifyBundlesExist(
    const std::map<::hermes::SHA1, std::shared_ptr<const jsi::Buffer>> &bundles,
    const SynthTrace &trace) {
  std::vector<::hermes::SHA1> missingSourceHashes;
  for (const auto &rec : trace.records()) {
    if (rec->getType() == SynthTrace::RecordType::BeginExecJS) {
      const auto &bejsr =
          record_cast<const SynthTrace::BeginExecJSRecord>(*rec);

      if (bundles.count(bejsr.sourceHash()) == 0 &&
          !isAllZeroSourceHash(bejsr.sourceHash())) {
        missingSourceHashes.emplace_back(bejsr.sourceHash());
      }
    }
  }
  if (!missingSourceHashes.empty()) {
    std::string msg = "Missing bundles with the following source hashes: ";
    bool first = true;
    for (const auto &hash : missingSourceHashes) {
      if (!first) {
        msg += ", ";
      }
      msg += ::hermes::hashAsString(hash);
      first = false;
    }
    msg += "\nProvided source hashes: ";
    if (bundles.empty()) {
      msg += "(no sources provided)";
    } else {
      first = true;
      for (const auto &p : bundles) {
        if (!first) {
          msg += ", ";
        }
        msg += ::hermes::hashAsString(p.first);
        first = false;
      }
    }
    throw std::invalid_argument(msg);
  }
}

/// Returns the element of \p repGCStats with the median "totalTime" stat.
static std::string mergeGCStats(const std::vector<std::string> &repGCStats) {
  if (repGCStats.empty())
    throw std::invalid_argument("Empty GC stats.");
  std::vector<std::pair<double, const std::string *>> valueAndStats;
  for (auto &stats : repGCStats) {
    // Find "totalTime" by string search, to avoid JSONParser boilerplate
    // and issues with the "GC Stats" header.
    std::string target = "\"totalTime\":";
    auto pos = stats.find(target);
    if (pos == std::string::npos)
      throw std::invalid_argument("Malformed GC stats.");
    double value = std::atof(&stats[pos + target.size()]);
    valueAndStats.emplace_back(value, &stats);
  }
  unsigned median = (valueAndStats.size() - 1) / 2;
  std::nth_element(
      valueAndStats.begin(),
      valueAndStats.begin() + median,
      valueAndStats.end());
  return *valueAndStats[median].second;
}

bool isDoubleEqual(double a, double b) {
  // Bitwise comparison to avoid issues with infinity and NaN.
  if (memcmp(&a, &b, sizeof(double)) == 0)
    return true;
  // If the sign bit is different, then it is definitely not equal. This handles
  // cases like -0 != 0.
  if (std::signbit(a) != std::signbit(b))
    return false;
  // There can be subtle rounding differences between platforms, so allow for
  // some error. For example, GCC uses 80 bit registers for doubles on
  // non-Darwin x86 platforms. A change in when values are written to memory
  // (when it is shortened to 64 bits), or comparing values between Darwin and
  // non-Darwin platforms could lead to different results.
  const double ep =
      (std::abs(a) + std::abs(b)) * std::numeric_limits<double>::epsilon();
  return std::abs(a - b) <= ep;
}

} // namespace

/// Assert that \p val seen at replay matches the recorded \p traceValue
void TraceInterpreter::assertMatch(
    const SynthTrace::TraceValue &traceValue,
    const Value &val) const {
  if (traceValue.isUndefined()) {
    TRACE_EXPECT_TRUE(val.isUndefined());
  } else if (traceValue.isNull()) {
    TRACE_EXPECT_TRUE(val.isNull());
  } else if (traceValue.isBool()) {
    TRACE_EXPECT_TRUE(val.isBool());
    TRACE_EXPECT_EQ(traceValue.getBool(), val.getBool());
  } else if (traceValue.isNumber()) {
    TRACE_EXPECT_TRUE(val.isNumber());
    double valNum = val.getNumber();
    double traceValueNum = traceValue.getNumber();
    TRACE_EXPECT_TRUE(isDoubleEqual(valNum, traceValueNum));
  } else if (traceValue.isString()) {
    TRACE_EXPECT_TRUE(val.isString());
  } else if (traceValue.isObject()) {
    TRACE_EXPECT_TRUE(val.isObject());
  } else if (traceValue.isSymbol()) {
    TRACE_EXPECT_TRUE(val.isSymbol());
  } else if (traceValue.isBigInt()) {
    TRACE_EXPECT_TRUE(val.isBigInt());
  }
}

TraceInterpreter::TraceInterpreter(
    jsi::Runtime &rt,
    const ExecuteOptions &options,
    const SynthTrace &trace,
    std::map<::hermes::SHA1, std::shared_ptr<const Buffer>> bundles)
    : rt_(rt),
      options_(options),
      bundles_(std::move(bundles)),
      trace_(trace),
      gom_() {
  // Add the global object to the global object map
  if (trace.globalObjID()) {
    gom_.emplace(*trace.globalObjID(), rt.global());
  }

  auto [lastUsePerObj, lastUses] = createLastUseMaps(trace.records());
  lastUsePerObj_ = std::move(lastUsePerObj);
  lastUses_ = std::move(lastUses);
}

/* static */
std::string TraceInterpreter::execAndGetStats(
    const std::string &traceFile,
    const std::vector<std::string> &bytecodeFiles,
    const ExecuteOptions &options) {
  // If there is a trace, don't write it out, not used here.
  return execWithRuntime(traceFile, bytecodeFiles, options, makeHermesRuntime);
}

/* static */
std::string TraceInterpreter::execWithRuntime(
    const std::string &traceFile,
    const std::vector<std::string> &bytecodeFiles,
    const ExecuteOptions &options,
    const std::function<std::unique_ptr<jsi::Runtime>(
        const ::hermes::vm::RuntimeConfig &runtimeConfig)> &createRuntime) {
  auto errorOrFile = llvh::MemoryBuffer::getFile(traceFile);
  if (!errorOrFile) {
    throw std::system_error(errorOrFile.getError());
  }
  std::unique_ptr<llvh::MemoryBuffer> traceBuf = std::move(errorOrFile.get());
  std::vector<std::unique_ptr<llvh::MemoryBuffer>> bytecodeBuffers;
  for (const std::string &bytecode : bytecodeFiles) {
    errorOrFile = llvh::MemoryBuffer::getFile(bytecode);
    if (!errorOrFile) {
      throw std::system_error(errorOrFile.getError());
    }
    bytecodeBuffers.emplace_back(std::move(errorOrFile.get()));
  }
  return std::get<0>(execFromMemoryBuffer(
      std::move(traceBuf), std::move(bytecodeBuffers), options, createRuntime));
}

/* static */
std::map<::hermes::SHA1, std::shared_ptr<const jsi::Buffer>>
TraceInterpreter::getSourceHashToBundleMap(
    std::vector<std::unique_ptr<llvh::MemoryBuffer>> &&codeBufs,
    const SynthTrace &trace,
    const ExecuteOptions &options,
    bool *codeIsMmapped,
    bool *isBytecode) {
  if (codeIsMmapped) {
    *codeIsMmapped = true;
    for (const auto &buf : codeBufs) {
      if (buf->getBufferKind() != llvh::MemoryBuffer::MemoryBuffer_MMap) {
        // If any of the buffers aren't mmapped, don't turn on I/O tracking.
        *codeIsMmapped = false;
        break;
      }
    }
  }
  std::vector<std::unique_ptr<const jsi::Buffer>> bundles;
  for (auto &buf : codeBufs) {
    bundles.emplace_back(bufConvert(std::move(buf)));
  }

  if (isBytecode) {
    *isBytecode = true;
    for (const auto &bundle : bundles) {
      if (!HermesRuntime::isHermesBytecode(bundle->data(), bundle->size())) {
        // If any of the buffers are source code, don't turn on I/O tracking.
        *isBytecode = false;
        break;
      }
    }
  }

  // Map source hashes of files to their memory buffer.
  std::map<::hermes::SHA1, std::shared_ptr<const jsi::Buffer>>
      sourceHashToBundle;
  for (auto &bundle : bundles) {
    ::hermes::SHA1 sourceHash{};
    if (HermesRuntime::isHermesBytecode(bundle->data(), bundle->size())) {
      sourceHash =
          ::hermes::hbc::BCProviderFromBuffer::getSourceHashFromBytecode(
              llvh::makeArrayRef(bundle->data(), bundle->size()));
    } else {
      sourceHash =
          llvh::SHA1::hash(llvh::makeArrayRef(bundle->data(), bundle->size()));
    }
    auto inserted = sourceHashToBundle.insert({sourceHash, std::move(bundle)});
    assert(
        inserted.second &&
        "Duplicate source hash detected, files only need to be supplied once");
    (void)inserted;
  }

  if (!options.disableSourceHashCheck) {
    verifyBundlesExist(sourceHashToBundle, trace);
  }

  return sourceHashToBundle;
}

/* static */
::hermes::vm::RuntimeConfig TraceInterpreter::merge(
    ::hermes::vm::RuntimeConfig::Builder &rtConfigBuilderIn,
    const ::hermes::vm::GCConfig::Builder &gcConfigBuilderIn,
    const ExecuteOptions &options,
    bool codeIsMmapped,
    bool isBytecode) {
  ::hermes::vm::RuntimeConfig::Builder rtConfigBuilder;
  ::hermes::vm::GCConfig::Builder gcConfigBuilder;

  if (options.useTraceConfig) {
    rtConfigBuilder.update(rtConfigBuilderIn);
  } else {
    // Some portions of even the default config must agree with the trace
    // config, because the contents of the trace assume a given shape for
    // runtime data structures during replay.  So far, we know of only one such
    // parameter: EnableSampledStats.
    rtConfigBuilder.withEnableSampledStats(
        rtConfigBuilderIn.build().getEnableSampledStats());
  }

  if (options.bytecodeWarmupPercent) {
    rtConfigBuilder.withBytecodeWarmupPercent(*options.bytecodeWarmupPercent);
  }
  if (options.shouldTrackIO) {
    rtConfigBuilder.withTrackIO(
        *options.shouldTrackIO && isBytecode && codeIsMmapped);
  }

  // If (and only if) an out trace is requested, turn on tracing in the VM
  // as well.
  if (options.traceEnabled) {
    rtConfigBuilder.withSynthTraceMode(
        ::hermes::vm::SynthTraceMode::TracingAndReplaying);
  } else {
    rtConfigBuilder.withSynthTraceMode(::hermes::vm::SynthTraceMode::Replaying);
  }

  if (options.action == ExecuteOptions::MarkerAction::SAMPLE_TIME) {
    // If time sampling is requested, the RuntimeConfig has to have the sampling
    // profiler enabled.
    rtConfigBuilder.withEnableSampleProfiling(true);
  }

  // If aggregating multiple reps, randomize the placement of some data
  // structures in each rep, for a more robust time metric.
  if (options.reps > 1) {
    rtConfigBuilder.withRandomizeMemoryLayout(true);
  }

  if (options.useTraceConfig) {
    gcConfigBuilder.update(gcConfigBuilderIn);
  }
  gcConfigBuilder.update(options.gcConfigBuilder);
  return rtConfigBuilder.withGCConfig(gcConfigBuilder.build()).build();
}

/* static */
std::tuple<std::string, std::unique_ptr<jsi::Runtime>>
TraceInterpreter::execFromMemoryBuffer(
    std::unique_ptr<llvh::MemoryBuffer> &&traceBuf,
    std::vector<std::unique_ptr<llvh::MemoryBuffer>> &&codeBufs,
    const ExecuteOptions &options,
    const std::function<std::unique_ptr<jsi::Runtime>(
        const ::hermes::vm::RuntimeConfig &runtimeConfig)> &createRuntime) {
  auto [trace, rtConfigBuilder, gcConfigBuilder] =
      parseSynthTrace(std::move(traceBuf));

  bool codeIsMmapped;
  bool isBytecode;
  const std::map<::hermes::SHA1, std::shared_ptr<const jsi::Buffer>>
      sourceHashToBundle = getSourceHashToBundleMap(
          std::move(codeBufs), trace, options, &codeIsMmapped, &isBytecode);

  const auto &rtConfig = merge(
      rtConfigBuilder, gcConfigBuilder, options, codeIsMmapped, isBytecode);

  std::vector<std::string> repGCStats(options.reps);
  std::unique_ptr<jsi::Runtime> rt;
  for (int rep = -options.warmupReps; rep < options.reps; ++rep) {
    ::hermes::vm::instrumentation::PerfEvents::begin();
    rt = createRuntime(rtConfig);

    auto stats = exec(*rt, options, trace, sourceHashToBundle);
    // If we're not warming up, save the stats.
    if (rep >= 0) {
      repGCStats[rep] = stats;
    }
  }

  return std::make_tuple(
      // Merged GC stats
      rtConfig.getGCConfig().getShouldRecordStats() ? mergeGCStats(repGCStats)
                                                    : "",
      // The last runtime used for replay
      std::move(rt));
}

/* static */
std::string TraceInterpreter::exec(
    jsi::Runtime &rt,
    const ExecuteOptions &options,
    const SynthTrace &trace,
    std::map<::hermes::SHA1, std::shared_ptr<const jsi::Buffer>> bundles) {
  TraceInterpreter interpreter(rt, options, trace, std::move(bundles));
  return interpreter.executeRecordsWithMarkerOptions();
}

Function TraceInterpreter::createHostFunction(
    const SynthTrace::CreateHostFunctionRecord &rec,
    const PropNameID &propNameID) {
#ifdef HERMESVM_API_TRACE_DEBUG
  assert(propNameID.utf8(rt_) == rec.functionName_);
#endif
  return Function::createFromHostFunction(
      rt_,
      propNameID,
      rec.paramCount_,
      [this](
          Runtime &rt,
          const Value &thisVal,
          const Value *args,
          size_t count) mutable -> Value {
        try {
          const auto &rec = trace_.records()[nextExecIndex_];
          const auto &ctnr =
              record_cast<const SynthTrace::CallToNativeRecord>(*rec);
          // Associate the this arg with its object id.
          ifObjectAddToObjectMap(
              ctnr.thisArg_, thisVal, nextExecIndex_, /* isThis = */ true);
          // Associate each argument with its object id.
          assert(
              ctnr.args_.size() == count &&
              "Called at runtime with a different number of args than "
              "the trace expected");
          for (uint64_t i = 0; i < ctnr.args_.size(); ++i) {
            ifObjectAddToObjectMap(
                ctnr.args_[i], Value{rt, args[i]}, nextExecIndex_);
          }

          executeRecords();

          const auto &rfnr =
              record_cast<const SynthTrace::ReturnFromNativeRecord>(
                  *trace_.records()[nextExecIndex_ - 1]);
          return traceValueToJSIValue(rfnr.retVal_);
        } catch (const std::exception &e) {
          crashOnException(e, llvh::None);
        }
      });
}

Object TraceInterpreter::createHostObject(ObjectID objID) {
  class FakeHostObject : public HostObject {
    TraceInterpreter &interpreter_;

   public:
    explicit FakeHostObject(TraceInterpreter &interpreter)
        : interpreter_(interpreter) {}

    Value get(Runtime &rt, const PropNameID &name) override {
      try {
        const auto &rec =
            interpreter_.trace_.records()[interpreter_.nextExecIndex_];
        const auto &gpnr =
            record_cast<const SynthTrace::GetPropertyNativeRecord>(*rec);
        interpreter_.addToPropNameIDMap(
            gpnr.propNameID_,
            jsi::PropNameID{interpreter_.rt_, name},
            interpreter_.nextExecIndex_);

        interpreter_.executeRecords();

        const auto &gpnrr =
            record_cast<const SynthTrace::GetPropertyNativeReturnRecord>(
                *interpreter_.trace_
                     .records()[interpreter_.nextExecIndex_ - 1]);
        return interpreter_.traceValueToJSIValue(gpnrr.retVal_);
      } catch (const std::exception &e) {
        interpreter_.crashOnException(e, llvh::None);
      }
    }

    void set(Runtime &rt, const PropNameID &name, const Value &value) override {
      try {
        const auto &rec =
            interpreter_.trace_.records()[interpreter_.nextExecIndex_];
        const auto &spnr =
            record_cast<const SynthTrace::SetPropertyNativeRecord>(*rec);
        interpreter_.addToPropNameIDMap(
            spnr.propNameID_,
            jsi::PropNameID{interpreter_.rt_, name},
            interpreter_.nextExecIndex_);
        interpreter_.ifObjectAddToObjectMap(
            spnr.value_, Value(rt, value), interpreter_.nextExecIndex_);

        // There is exactly one argument to pass to a set call.
        interpreter_.executeRecords();
      } catch (const std::exception &e) {
        interpreter_.crashOnException(e, llvh::None);
      }
    }

    std::vector<PropNameID> getPropertyNames(Runtime &rt) override {
      try {
        interpreter_.executeRecords();

        const auto &rec =
            interpreter_.trace_.records()[interpreter_.nextExecIndex_ - 1];
        assert(rec->getType() == RecordType::GetNativePropertyNamesReturn);
        const auto &record =
            record_cast<const SynthTrace::GetNativePropertyNamesReturnRecord>(
                *rec);

        std::vector<PropNameID> propNameIDs;
        for (const SynthTrace::TraceValue &name : record.propNameIDs_) {
          propNameIDs.emplace_back(
              interpreter_.getPropNameIDForUse(name.getUID()));
        }

        return propNameIDs;
      } catch (const std::exception &e) {
        interpreter_.crashOnException(e, llvh::None);
      }
    }
  };

  return Object::createFromHostObject(
      rt_, std::make_shared<FakeHostObject>(*this));
}

std::string TraceInterpreter::executeRecordsWithMarkerOptions() {
  assert(
      (options_.action == ExecuteOptions::MarkerAction::NONE ||
       !options_.profileFileName.empty()) &&
      "If the action isn't none, need a profile output file");
  switch (options_.action) {
    case ExecuteOptions::MarkerAction::TIMELINE:
      if (auto *hermesRuntime = dynamic_cast<HermesRuntime *>(&rt_)) {
        // Start tracking heap objects right before interpreting the trace.
        // No need to handle fragment callbacks, as this is not live profiling
        // being given to Chrome, it's just going to a file.
        hermesRuntime->instrumentation().startTrackingHeapObjectStackTraces(
            nullptr);
      }
      break;
    case ExecuteOptions::MarkerAction::SAMPLE_MEMORY:
      if (auto *hermesRuntime = dynamic_cast<HermesRuntime *>(&rt_)) {
        hermesRuntime->instrumentation().startHeapSampling(1 << 15);
      }
      break;
    case ExecuteOptions::MarkerAction::SAMPLE_TIME:
      if (dynamic_cast<HermesRuntime *>(&rt_)) {
        HermesRuntime::enableSamplingProfiler();
      }
      break;
    default:
      // Do nothing.
      break;
  }
  executeRecords();

#ifdef HERMESVM_PROFILER_BB
  if (auto *hermesRuntime = dynamic_cast<HermesRuntime *>(&rt_)) {
    hermesRuntime->dumpBasicBlockProfileTrace(std::cerr);
  }
#endif

  checkMarker(std::string("end"));
  if (!markerFound_) {
    // An action was requested at a marker but that marker wasn't found.
    throw std::runtime_error(
        std::string("Marker \"") + options_.marker +
        "\" specified but not found in trace");
  }

  return stats_;
}

jsi::Value TraceInterpreter::traceValueToJSIValue(
    SynthTrace::TraceValue value) {
  if (value.isUndefined()) {
    return Value::undefined();
  }
  if (value.isNull()) {
    return Value::null();
  }
  if (value.isNumber()) {
    return Value(value.getNumber());
  }
  if (value.isBool()) {
    return Value(value.getBool());
  }
  if (value.isUID()) {
    return getJSIValueForUse(value.getUID());
  }
  llvm_unreachable("Unrecognized value type encountered");
}

jsi::Value TraceInterpreter::getJSIValueForUse(SynthTrace::ObjectID id) {
  auto it = gom_.find(id);
  assert(it != gom_.end() && "Value not found");
  return jsi::Value{rt_, it->second};
};

jsi::PropNameID TraceInterpreter::getPropNameIDForUse(SynthTrace::ObjectID id) {
  auto it = gpnm_.find(id);
  assert(it != gpnm_.end() && "ID not found");
  return PropNameID{rt_, it->second};
};

void TraceInterpreter::executeRecords() {
  llvh::SaveAndRestore<uint64_t> depthGuard(depth_, depth_ + 1);
  const std::vector<std::unique_ptr<SynthTrace::Record>> &records =
      trace_.records();

  // Save a value so that Call can set it, and Return can access it.
  Value retval;
  // Carry the return value from BeginJSExec to EndJSExec.
  Value overallRetval;

#ifndef NDEBUG
  // We'll want the first record, to verify that it's the one that consumes
  // nativePropNameToConsumeAsDef if that is non-null.
  const SynthTrace::Record *firstRec = records[nextExecIndex_].get();
  if (depth_ != 1) {
    RecordType firstRecType = firstRec->getType();
    assert(
        (firstRecType == RecordType::CallToNative ||
         firstRecType == RecordType::GetNativePropertyNames ||
         firstRecType == RecordType::GetPropertyNative ||
         firstRecType == RecordType::SetPropertyNative) &&
        "Illegal starting record");
  }
#endif

  const auto endIndex = records.size();
  while (nextExecIndex_ < endIndex) {
    const auto currentExecIndex = nextExecIndex_++;
    eraseRefsBefore(currentExecIndex);
    try {
      const SynthTrace::Record *rec = records[currentExecIndex].get();
      switch (rec->getType()) {
        case RecordType::BeginExecJS: {
          const auto &bejsr =
              record_cast<const SynthTrace::BeginExecJSRecord>(*rec);
          auto it = bundles_.find(bejsr.sourceHash());
          if (it == bundles_.end()) {
            if ((options_.disableSourceHashCheck ||
                 isAllZeroSourceHash(bejsr.sourceHash())) &&
                bundles_.size() == 1) {
              // Normally, if a bundle's source hash doesn't match, it would
              // be an error. However, for convenience and backwards
              // compatibility, allow an all-zero hash to automatically assume
              // a bundle if that was the only bundle supplied.
              it = bundles_.begin();
            } else {
              throw std::invalid_argument(
                  "Trace expected the source hash " +
                  ::hermes::hashAsString(bejsr.sourceHash()) +
                  ", that wasn't provided as a file");
            }
          }

          // Copy the shared pointer to the buffer in case this file is
          // executed multiple times.
          auto bundle = it->second;
          if (!HermesRuntime::isHermesBytecode(
                  bundle->data(), bundle->size())) {
            llvh::errs()
                << "Note: You are running from source code, not HBC bytecode.\n"
                << "      This run will reflect dev performance, not production.\n";
          }
          // overallRetval is to be consumed when we get an EndExecJS record.
          overallRetval =
              rt_.evaluateJavaScript(std::move(bundle), bejsr.sourceURL());
          break;
        }
        case RecordType::EndExecJS: {
          const auto &eejsr =
              record_cast<const SynthTrace::EndExecJSRecord>(*rec);
          ifObjectAddToObjectMap(
              eejsr.retVal_, std::move(overallRetval), currentExecIndex);
          [[fallthrough]];
        }
        case RecordType::Marker: {
          const auto &mr = static_cast<const SynthTrace::MarkerRecord &>(*rec);
          // If the tag is the requested tag, and the stats have not already
          // been collected, collect them.
          checkMarker(mr.tag_);
          if (auto *tracingRT = dynamic_cast<TracingRuntime *>(&rt_)) {
            if (rec->getType() != RecordType::EndExecJS) {
              // If tracing is on, re-emit the marker into the result stream.
              // This way, the trace emitted from replay will be the same as
              // the trace input. Don't do this for EndExecJSRecord, since
              // that falls through to here, and it already emits a marker
              // automatically.
              tracingRT->addMarker(mr.tag_);
            }
          }
          break;
        }
        case RecordType::CreateObject: {
          const auto &cor =
              static_cast<const SynthTrace::CreateObjectRecord &>(*rec);
          // Make an empty object to be used.
          addToObjectMap(cor.objID_, Object(rt_), currentExecIndex);
          break;
        }
        case RecordType::CreateBigInt: {
          const auto &cbr =
              static_cast<const SynthTrace::CreateBigIntRecord &>(*rec);
          Value bigint;
          switch (cbr.method_) {
            case SynthTrace::CreateBigIntRecord::Method::FromInt64:
              bigint = BigInt::fromInt64(rt_, static_cast<int64_t>(cbr.bits_));
              TRACE_EXPECT_EQ(
                  static_cast<int64_t>(cbr.bits_),
                  bigint.asBigInt(rt_).getInt64(rt_));
              break;
            case SynthTrace::CreateBigIntRecord::Method::FromUint64:
              bigint = BigInt::fromUint64(rt_, cbr.bits_);
              TRACE_EXPECT_EQ(cbr.bits_, bigint.asBigInt(rt_).getUint64(rt_));
              break;
          }
          addToObjectMap(cbr.objID_, std::move(bigint), currentExecIndex);
          break;
        }
        case RecordType::BigIntToString: {
          const auto &bts =
              static_cast<const SynthTrace::BigIntToStringRecord &>(*rec);
          BigInt obj = getJSIValueForUse(bts.bigintID_).asBigInt(rt_);
          addToObjectMap(
              bts.strID_, obj.toString(rt_, bts.radix_), currentExecIndex);
          break;
        }
        case RecordType::CreateString: {
          const auto &csr =
              static_cast<const SynthTrace::CreateStringRecord &>(*rec);
          Value str;
          if (csr.ascii_) {
            str = String::createFromAscii(
                rt_, csr.chars_.data(), csr.chars_.size());
          } else {
            str = String::createFromUtf8(
                rt_,
                reinterpret_cast<const uint8_t *>(csr.chars_.data()),
                csr.chars_.size());
          }
          TRACE_EXPECT_EQ(csr.chars_, str.asString(rt_).utf8(rt_));
          addToObjectMap(csr.objID_, std::move(str), currentExecIndex);
          break;
        }
        case RecordType::CreatePropNameID: {
          const auto &cpnr =
              static_cast<const SynthTrace::CreatePropNameIDRecord &>(*rec);
          // We perform the calls below for their side effects (for example,
          jsi::PropNameID propNameID = [&] {
            switch (cpnr.valueType_) {
              case SynthTrace::CreatePropNameIDRecord::ASCII:
                return PropNameID::forAscii(rt_, cpnr.chars_);
              case SynthTrace::CreatePropNameIDRecord::UTF8:
                return PropNameID::forUtf8(rt_, cpnr.chars_);
              case SynthTrace::CreatePropNameIDRecord::TRACEVALUE: {
                auto val = traceValueToJSIValue(cpnr.traceValue_);
                if (val.isSymbol())
                  return PropNameID::forSymbol(rt_, val.getSymbol(rt_));
                return PropNameID::forString(rt_, val.getString(rt_));
              }
            }
            llvm_unreachable("No other way to construct PropNameID");
          }();
          addToPropNameIDMap(
              cpnr.propNameID_, std::move(propNameID), currentExecIndex);
          break;
        }
        case RecordType::CreateHostObject: {
          const auto &chor =
              static_cast<const SynthTrace::CreateHostObjectRecord &>(*rec);
          const ObjectID objID = chor.objID_;
          addToObjectMap(
              objID, Value(rt_, createHostObject(objID)), currentExecIndex);
          break;
        }
        case RecordType::CreateHostFunction: {
          const auto &chfr =
              static_cast<const SynthTrace::CreateHostFunctionRecord &>(*rec);
          addToObjectMap(
              chfr.objID_,
              Value(
                  rt_,
                  createHostFunction(
                      chfr, getPropNameIDForUse(chfr.propNameID_))),
              currentExecIndex);
          break;
        }
        case RecordType::QueueMicrotask: {
          const auto &queueRecord =
              static_cast<const SynthTrace::QueueMicrotaskRecord &>(*rec);
          jsi::Function callback = getJSIValueForUse(queueRecord.callbackID_)
                                       .getObject(rt_)
                                       .asFunction(rt_);
          rt_.queueMicrotask(callback);
          break;
        }
        case RecordType::DrainMicrotasks: {
          const auto &drainRecord =
              static_cast<const SynthTrace::DrainMicrotasksRecord &>(*rec);
          rt_.drainMicrotasks(drainRecord.maxMicrotasksHint_);
          break;
        }
        case RecordType::GetProperty: {
          const auto &gpr =
              static_cast<const SynthTrace::GetPropertyRecord &>(*rec);
          // Call get property on the object specified, and possibly define
          // the result.
          jsi::Value value;
          const auto &obj = getJSIValueForUse(gpr.objID_).asObject(rt_);
          if (gpr.propID_.isString()) {
            const jsi::String propString =
                getJSIValueForUse(gpr.propID_.getUID()).asString(rt_);
#ifdef HERMESVM_API_TRACE_DEBUG
            assert(propString.utf8(rt_) == gpr.propNameDbg_);
#endif
            value = obj.getProperty(rt_, propString);
          } else {
            assert(gpr.propID_.isPropNameID());
            auto propNameID = getPropNameIDForUse(gpr.propID_.getUID());
#ifdef HERMESVM_API_TRACE_DEBUG
            assert(propNameID.utf8(rt_) == gpr.propNameDbg_);
#endif
            value = obj.getProperty(rt_, propNameID);
          }
          retval = std::move(value);
          break;
        }
        case RecordType::SetProperty: {
          const auto &spr =
              static_cast<const SynthTrace::SetPropertyRecord &>(*rec);
          auto obj = getJSIValueForUse(spr.objID_).asObject(rt_);
          // Call set property on the object specified and give it the value.
          if (spr.propID_.isString()) {
            const jsi::String propString =
                getJSIValueForUse(spr.propID_.getUID()).asString(rt_);
#ifdef HERMESVM_API_TRACE_DEBUG
            assert(propString.utf8(rt_) == spr.propNameDbg_);
#endif
            obj.setProperty(rt_, propString, traceValueToJSIValue(spr.value_));
          } else {
            assert(spr.propID_.isPropNameID());
            auto propNameID = getPropNameIDForUse(spr.propID_.getUID());
#ifdef HERMESVM_API_TRACE_DEBUG
            assert(propNameID.utf8(rt_) == spr.propNameDbg_);
#endif
            obj.setProperty(rt_, propNameID, traceValueToJSIValue(spr.value_));
          }
          break;
        }
        case RecordType::HasProperty: {
          const auto &hpr =
              static_cast<const SynthTrace::HasPropertyRecord &>(*rec);
          auto obj = getJSIValueForUse(hpr.objID_).asObject(rt_);
          if (hpr.propID_.isString()) {
            const jsi::String propString =
                getJSIValueForUse(hpr.propID_.getUID()).asString(rt_);
#ifdef HERMESVM_API_TRACE_DEBUG
            assert(propString.utf8(rt_) == hpr.propNameDbg_);
#endif
            obj.hasProperty(rt_, propString);
          } else {
            assert(hpr.propID_.isPropNameID());
            auto propNameID = getPropNameIDForUse(hpr.propID_.getUID());
#ifdef HERMESVM_API_TRACE_DEBUG
            assert(propNameID.utf8(rt_) == hpr.propNameDbg_);
#endif
            obj.hasProperty(rt_, propNameID);
          }
          break;
        }
        case RecordType::GetPropertyNames: {
          const auto &gpnr =
              static_cast<const SynthTrace::GetPropertyNamesRecord &>(*rec);
          jsi::Array arr = getJSIValueForUse(gpnr.objID_)
                               .asObject(rt_)
                               .getPropertyNames(rt_);
          retval = std::move(arr);
          break;
        }
        case RecordType::CreateArray: {
          const auto &car =
              static_cast<const SynthTrace::CreateArrayRecord &>(*rec);
          // Make an array of the appropriate length to be used.
          addToObjectMap(car.objID_, Array(rt_, car.length_), currentExecIndex);
          break;
        }
        case RecordType::ArrayRead: {
          const auto &arr =
              static_cast<const SynthTrace::ArrayReadRecord &>(*rec);
          // Read from the specified array, and possibly define the result.
          auto value = getJSIValueForUse(arr.objID_)
                           .asObject(rt_)
                           .asArray(rt_)
                           .getValueAtIndex(rt_, arr.index_);
          retval = std::move(value);
          break;
        }
        case RecordType::ArrayWrite: {
          const auto &awr =
              static_cast<const SynthTrace::ArrayWriteRecord &>(*rec);
          // Write to the array and give it the value.
          getJSIValueForUse(awr.objID_)
              .asObject(rt_)
              .asArray(rt_)
              .setValueAtIndex(
                  rt_, awr.index_, traceValueToJSIValue(awr.value_));
          break;
        }
        case RecordType::CallFromNative: {
          const auto &cfnr =
              static_cast<const SynthTrace::CallFromNativeRecord &>(*rec);
          auto func =
              getJSIValueForUse(cfnr.functionID_).asObject(rt_).asFunction(rt_);
          std::vector<Value> args;
          for (const auto &arg : cfnr.args_) {
            args.emplace_back(traceValueToJSIValue(arg));
          }
          // Save the return result into retval so that ReturnToNative can
          // access it and put it at the correct object id.
          const Value *argStart = args.data();
          if (cfnr.thisArg_.isUndefined()) {
            retval = func.call(rt_, argStart, args.size());
          } else {
            assert(
                cfnr.thisArg_.isObject() &&
                "Encountered a thisArg which was not undefined or an object");
            retval = func.callWithThis(
                rt_,
                getJSIValueForUse(cfnr.thisArg_.getUID()).asObject(rt_),
                argStart,
                args.size());
          }
          break;
        }
        case RecordType::ConstructFromNative: {
          const auto &cfnr =
              static_cast<const SynthTrace::ConstructFromNativeRecord &>(*rec);
          // Essentially the same implementation as CallFromNative, except
          // calls the construct path.
          auto func =
              getJSIValueForUse(cfnr.functionID_).asObject(rt_).asFunction(rt_);
          std::vector<Value> args;
          for (const auto &arg : cfnr.args_) {
            args.emplace_back(traceValueToJSIValue(arg));
          }
          assert(
              cfnr.thisArg_.isUndefined() &&
              "The this arg should always be undefined for a construct call");
          // Save the return result into retval so that ReturnToNative can
          // access it and put it at the correct object id.
          const Value *argStart = args.data();
          retval = func.callAsConstructor(rt_, argStart, args.size());
          break;
        }
        case RecordType::ReturnFromNative: {
          return;
        }
        case RecordType::ReturnToNative: {
          const auto &rtnr =
              record_cast<const SynthTrace::ReturnToNativeRecord>(*rec);
          ifObjectAddToObjectMap(
              rtnr.retVal_, std::move(retval), currentExecIndex);
          // If the return value wasn't an object, it can be ignored.
          break;
        }
        case RecordType::CallToNative: {
          break;
        }
        case RecordType::GetPropertyNative: {
          break;
        }
        case RecordType::GetPropertyNativeReturn: {
          return;
        }
        case RecordType::SetPropertyNative: {
          break;
        }
        case RecordType::SetPropertyNativeReturn: {
          return;
        }
        case RecordType::GetNativePropertyNames: {
          // Nothing actually needs to happen here, as no defs are provided to
          // the local function. The HostObject already handles accessing the
          // property names.
          break;
        }
        case RecordType::GetNativePropertyNamesReturn: {
          return;
        }
        case RecordType::SetExternalMemoryPressure: {
          const auto &record =
              static_cast<const SynthTrace::SetExternalMemoryPressureRecord &>(
                  *rec);
          const auto &obj = getJSIValueForUse(record.objID_).asObject(rt_);
          obj.setExternalMemoryPressure(rt_, record.amount_);
          break;
        }
        case RecordType::Utf8: {
          const auto &record =
              static_cast<const SynthTrace::Utf8Record &>(*rec);

          if (record.objID_.isString()) {
            const auto &val = getJSIValueForUse(record.objID_.getUID());
            TRACE_EXPECT_EQ(record.retVal_, val.getString(rt_).utf8(rt_));
          } else if (record.objID_.isPropNameID()) {
            auto propNameID = getPropNameIDForUse(record.objID_.getUID());
            TRACE_EXPECT_EQ(record.retVal_, propNameID.utf8(rt_));
          } else if (record.objID_.isSymbol()) {
            jsi::Value val = getJSIValueForUse(record.objID_.getUID());
            TRACE_EXPECT_EQ(record.retVal_, val.asSymbol(rt_).toString(rt_));
          }
          break;
        }
        case RecordType::Global: {
          const auto &record =
              static_cast<const SynthTrace::GlobalRecord &>(*rec);
          addToObjectMap(record.objID_, rt_.global(), currentExecIndex);
          break;
        }
      }
    } catch (const std::exception &e) {
      crashOnException(e, currentExecIndex);
    }
    // If the top of the stack is reached after a marker flushed the stats,
    // exit early.
    if (depth_ == 1 && markerFound_) {
      return;
    }
  } // the end of loop

  if (depth_ != 1) {
    // For the non-entry point, there should always be an explicit
    // ReturnFromNative or Get/SetPropertyNativeReturn which will return early
    // from this function. If there was no explicit return, the trace is
    // malformed.
    llvm_unreachable("There was no return in the call");
  }

  return;
}

void TraceInterpreter::eraseRefsBefore(uint64_t index) {
  while (lastUsesIndex_ < lastUses_.size() &&
         lastUses_[lastUsesIndex_].first < index) {
    const std::pair<uint64_t, SynthTrace::ObjectID> &kv =
        lastUses_[lastUsesIndex_];
    gom_.erase(kv.second);
    gpnm_.erase(kv.second);
    ++lastUsesIndex_;
  }
}

namespace {

template <typename ValueType, typename MapType>
void addValueToMap(
    MapType &map,
    SynthTrace::ObjectID id,
    ValueType &&val,
    const std::unordered_map<SynthTrace::ObjectID, uint64_t> &lastUsePerObj,
    uint64_t defIndex) {
  auto it = lastUsePerObj.find(id);
  if (it == lastUsePerObj.end()) {
    // It is possible that this Object was defined but never used in the record.
    return;
  }

  if (map.find(id) != map.end()) {
    return;
  }

  // If this defIndex is within the range of the live range, add it to the map.
  const uint64_t lastUse = it->second;
  if (defIndex <= lastUse) {
    map.emplace(id, std::forward<ValueType>(val));
    return;
  }
}

} // namespace

void TraceInterpreter::addToObjectMap(
    SynthTrace::ObjectID id,
    jsi::Value &&val,
    uint64_t defIndex) {
  addValueToMap(gom_, id, std::move(val), lastUsePerObj_, defIndex);
}

void TraceInterpreter::addToPropNameIDMap(
    SynthTrace::ObjectID id,
    jsi::PropNameID &&val,
    uint64_t defIndex) {
  addValueToMap(gpnm_, id, std::move(val), lastUsePerObj_, defIndex);
}

void TraceInterpreter::ifObjectAddToObjectMap(
    SynthTrace::TraceValue traceValue,
    jsi::Value &&val,
    uint64_t defIndex,
    bool isThis) {
  // TODO(T84791675): Include 'this' once all traces are correctly recording it.
  if (!isThis) {
    assertMatch(traceValue, val);
  }

  if (traceValue.isUID()) {
    addToObjectMap(traceValue.getUID(), std::move(val), defIndex);
  }
}

void TraceInterpreter::ifObjectAddToObjectMap(
    SynthTrace::TraceValue traceValue,
    const jsi::Value &val,
    uint64_t defIndex,
    bool isThis) {
  ifObjectAddToObjectMap(traceValue, jsi::Value{rt_, val}, defIndex, isThis);
}

void TraceInterpreter::checkMarker(const std::string &marker) {
  // Return early in these cases:
  // * If we've already found the marker
  // * If the marker we've found doesn't match the one we're looking for
  if (markerFound_ || marker != options_.marker) {
    return;
  }
  switch (options_.action) {
    case ExecuteOptions::MarkerAction::SNAPSHOT:
      if (HermesRuntime *hermesRT = dynamic_cast<HermesRuntime *>(&rt_)) {
        hermesRT->instrumentation().createSnapshotToFile(
            options_.profileFileName);
      } else {
        llvh::errs() << "Heap snapshot requested from non-Hermes runtime\n";
      }
      break;
    case ExecuteOptions::MarkerAction::TIMELINE:
      if (HermesRuntime *hermesRT = dynamic_cast<HermesRuntime *>(&rt_)) {
        hermesRT->instrumentation().stopTrackingHeapObjectStackTraces();
        hermesRT->instrumentation().createSnapshotToFile(
            options_.profileFileName);
      } else {
        llvh::errs() << "Heap timeline requested from non-Hermes runtime\n";
      }
      break;
    case ExecuteOptions::MarkerAction::SAMPLE_MEMORY:
      if (HermesRuntime *hermesRT = dynamic_cast<HermesRuntime *>(&rt_)) {
        std::ofstream stream(options_.profileFileName);
        hermesRT->instrumentation().stopHeapSampling(stream);
      } else {
        llvh::errs() << "Heap sampling requested from non-Hermes runtime\n";
      }
      break;
    case ExecuteOptions::MarkerAction::SAMPLE_TIME:
      if (dynamic_cast<HermesRuntime *>(&rt_)) {
        HermesRuntime::dumpSampledTraceToFile(options_.profileFileName);
        HermesRuntime::disableSamplingProfiler();
      } else {
        llvh::errs() << "CPU sampling requested from non-Hermes runtime\n";
      }
      break;
    case ExecuteOptions::MarkerAction::NONE:
      // Nothing extra needs to be done for the None case. Handle here to avoid
      // warnings.
      break;
  }
  stats_ = printStats();
  markerFound_ = true;
}

std::string TraceInterpreter::printStats() {
  if (options_.forceGCBeforeStats) {
    rt_.instrumentation().collectGarbage("forced for stats");
  }
  std::string stats = rt_.instrumentation().getRecordedGCStats();
  ::hermes::vm::instrumentation::PerfEvents::endAndInsertStats(stats);
#ifdef HERMESVM_PROFILER_OPCODE
  stats += "\n";
  std::ostringstream os;
  if (auto *hermesRuntime = dynamic_cast<HermesRuntime *>(&rt_)) {
    hermesRuntime->dumpOpcodeStats(os);
  } else {
    throw std::runtime_error("Unable to cast runtime into HermesRuntime");
  }
  stats += os.str();
  stats += "\n";
#endif
  return stats;
}

LLVM_ATTRIBUTE_NORETURN void TraceInterpreter::crashOnException(
    const std::exception &e,
    ::hermes::OptValue<uint64_t> globalRecordNum) {
  llvh::errs() << "An exception occurred while running the benchmark:\n";
  if (globalRecordNum) {
    llvh::errs() << "At record number " << globalRecordNum.getValue() << ":\n";
  }
  llvh::errs() << e.what() << "\n";
  if (traceStream_) {
    llvh::errs() << "Writing out the trace\n";
    dynamic_cast<TracingRuntime &>(rt_).flushAndDisableTrace();
    llvh::errs() << "\n";
  } else {
    llvh::errs() << "Pass --trace to get a trace for comparison\n";
  }
  // Do not re-throw, since that will pass back and forth between JS and
  // Native, causing more perturbations to the state of this interpreter,
  // which will cause an assertion to fire.
  // Instead, crash here so that it's clear where the error actually
  // occurred.
  llvh::errs() << "Crashing now\n";
  std::abort();
}

} // namespace tracing
} // namespace hermes
} // namespace facebook
