/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifdef HERMESVM_API_TRACE

#include <hermes/TraceInterpreter.h>

#include <hermes/SynthTraceParser.h>
#include <hermes/TracingRuntime.h>
#include <hermes/VM/instrumentation/PerfEvents.h>
#include <jsi/instrumentation.h>
#include <llvm/Support/SHA1.h>
#include <llvm/Support/SaveAndRestore.h>

#include <algorithm>
#include <unordered_set>

using namespace hermes::parser;
using namespace facebook::jsi;

namespace facebook {
namespace hermes {
namespace tracing {

using RecordType = SynthTrace::RecordType;
using ObjectID = SynthTrace::ObjectID;

namespace {

constexpr ObjectID setupFuncID = std::numeric_limits<ObjectID>::max();

/// Get the map from host functions to calls, and host objects to calls, for
/// the given list of records.
/// \param setupFuncID The function id to use for the top-level function. This
///   should not collide with any possible existing function id.
/// \param records A list of records inside a trace.
std::pair<
    TraceInterpreter::HostFunctionToCalls,
    TraceInterpreter::HostObjectToCalls>
getCalls(
    ObjectID setupFuncID,
    const std::vector<std::unique_ptr<SynthTrace::Record>> &records) {
  // This function iterates through the list of records, maintaining a stack of
  // function calls that mimics the same sequence that was taken during trace
  // collection. It attributes each contiguous sequence of records that are part
  // of the same function invocation into a Piece, which it then puts into the
  // call.

  // NOTE: StackValue should be a std::variant when it's available. In the
  // meantime, use inheritance to keep it typesafe, in particular to destruct
  // the std::string correctly.
  struct StackValue {
    virtual ~StackValue() {}
  };
  struct HostFunction final : public StackValue {
    ObjectID hostFunctionID;
    HostFunction(ObjectID hostFunctionID)
        : StackValue(), hostFunctionID(hostFunctionID) {}
  };
  struct HostObject final : public StackValue {
    ObjectID objID;
    std::string propName;
    HostObject(ObjectID hostObjID, const std::string &propName)
        : StackValue(), objID(hostObjID), propName(propName) {}
  };
  TraceInterpreter::HostFunctionToCalls funcIDToRecords;
  // A mapping from a host object id to a map from property name to
  // list of records
  TraceInterpreter::HostObjectToCalls hostObjIDToNameToRecords;
  // As CallRecords are encountered, the id of the object or function being
  // called is placed at the end of this stack, so that when the matching return
  // is executed, records can be attributed to the previous call.
  std::vector<std::unique_ptr<StackValue>> stack;
  // Make the setup function. It is the bottom of the stack, and also a host
  // function.
  stack.emplace_back(new HostFunction(setupFuncID));
  funcIDToRecords[setupFuncID].emplace_back(
      TraceInterpreter::Call(TraceInterpreter::Call::Piece()));

  // Get a list of calls of the currently executing function from the stack.
  const auto getCallsFromStack =
      [&funcIDToRecords, &hostObjIDToNameToRecords](
          const StackValue &stackObj) -> std::vector<TraceInterpreter::Call> & {
    if (const auto *hostFunc = dynamic_cast<const HostFunction *>(&stackObj)) {
      // A function is just the function id.
      return funcIDToRecords[hostFunc->hostFunctionID];
    } else if (
        const auto *hostObj = dynamic_cast<const HostObject *>(&stackObj)) {
      // Host object is a combination of the object id and property
      // name
      return hostObjIDToNameToRecords[hostObj->objID][hostObj->propName];
    } else {
      llvm_unreachable("Shouldn't be any other subclasses of StackValue");
    }
  };
  for (uint64_t recordNum = 0; recordNum < records.size(); ++recordNum) {
    const auto &rec = records[recordNum];
    if (rec->getType() == RecordType::CreateHostFunction) {
      const auto &createHFRec =
          static_cast<const SynthTrace::CreateHostFunctionRecord &>(*rec);
      assert(
          funcIDToRecords.find(createHFRec.objID_) == funcIDToRecords.end() &&
          "The host function was already initialized, it was called before it was created");
      // Insert a call so that this function is in the map, even if nothing
      // calls it.
      funcIDToRecords.emplace(
          createHFRec.objID_, std::vector<TraceInterpreter::Call>());
    } else if (rec->getType() == RecordType::CreateHostObject) {
      const auto &createHORec =
          static_cast<const SynthTrace::CreateHostObjectRecord &>(*rec);
      assert(
          hostObjIDToNameToRecords.find(createHORec.objID_) ==
              hostObjIDToNameToRecords.end() &&
          "The host object was already initialized, it was used before it was created");
      // Insert an entry so that this host object is in the map, even if nothing
      // uses it.
      hostObjIDToNameToRecords.emplace(
          createHORec.objID_, TraceInterpreter::PropNameToCalls());
    } else if (rec->getType() == RecordType::CallToNative) {
      // JS will cause a call to native code. Add a new call on the stack, and
      // add a call with an empty piece to the function it is calling.
      const auto &callRec =
          static_cast<const SynthTrace::CallToNativeRecord &>(*rec);
      assert(
          callRec.functionID_ != setupFuncID &&
          "Should never encounter a call into the setup function");
      stack.emplace_back(new HostFunction(callRec.functionID_));
      funcIDToRecords[callRec.functionID_].emplace_back(
          TraceInterpreter::Call(TraceInterpreter::Call::Piece(recordNum)));
    } else if (
        rec->getType() == RecordType::GetPropertyNative ||
        rec->getType() == RecordType::SetPropertyNative) {
      const auto &nativeAccessRec =
          static_cast<const SynthTrace::GetOrSetPropertyNativeRecord &>(*rec);
      // JS will access a property on a host object, which delegates to an
      // accessor. Add a new call on the stack, and add a call with an empty
      // piece to the object and property it is calling.
      stack.emplace_back(new HostObject(
          nativeAccessRec.hostObjectID_, nativeAccessRec.propName_));
      hostObjIDToNameToRecords[nativeAccessRec.hostObjectID_]
                              [nativeAccessRec.propName_]
                                  .emplace_back(TraceInterpreter::Call(
                                      TraceInterpreter::Call::Piece(
                                          recordNum)));
    }
    auto &calls = getCallsFromStack(*stack.back());
    assert(!calls.empty() && "There should always be at least one call");
    auto &call = calls.back();
    if (rec->getType() == RecordType::ReturnToNative) {
      // Add a new piece to the current call because we're about to
      // re-enter native.
      call.pieces.emplace_back(TraceInterpreter::Call::Piece(recordNum));
    }
    auto &piece = call.pieces.back();
    piece.records.emplace_back(&*rec);
    if (rec->getType() == RecordType::GetPropertyNativeReturn ||
        rec->getType() == RecordType::SetPropertyNativeReturn ||
        rec->getType() == RecordType::ReturnFromNative) {
      stack.pop_back();
    }
  }
  assert(stack.size() == 1 && "Stack was not fully exhausted");
  return std::make_pair(funcIDToRecords, hostObjIDToNameToRecords);
}

/// Given a call, computes each local object's last def before first use, and
/// last use.
/// Stores this information into \code call->locals.
void createCallMetadata(TraceInterpreter::Call *call) {
  // start is unused, since there's no need to convert to global indices
  for (auto &piece : call->pieces) {
    auto recordNum = piece.start;
    for (auto *rec : piece.records) {
      // For records that define and use the same object, this order is
      // important, it might def an object that it uses, or use an object
      // that it defs.
      // For now, there's only one case where this happens, in
      // GetPropertyRecord, which is the case of def'ing an object that it
      // uses. So the uses need to come before defs.
      // A general solution would require each record saying which order the
      // two should be run in, or have a conflict resolver inside itself.
      for (ObjectID use : rec->uses()) {
        auto &loc = call->locals[use];
        // Locally defined object access, move the last use forward.
        loc.lastUse = recordNum;
      }
      for (ObjectID def : rec->defs()) {
        auto &loc = call->locals[def];
        if (loc.lastUse == TraceInterpreter::DefAndUse::kUnused) {
          // Nothing local has used this object yet, we can eliminate any
          // previous defs.
          loc.lastDefBeforeFirstUse = recordNum;
        }
      }
      recordNum++;
    }
  }
  // TODO (T31512967): Prune unused defs.
}

std::unordered_map<ObjectID, TraceInterpreter::DefAndUse> createGlobalMap(
    const TraceInterpreter::HostFunctionToCalls &funcCallStacks,
    const TraceInterpreter::HostObjectToCalls &objCallStacks,
    ObjectID globalObjID) {
  struct Glob {
    uint64_t firstUse{TraceInterpreter::DefAndUse::kUnused};
    uint64_t lastUse{TraceInterpreter::DefAndUse::kUnused};
  };
  std::unordered_map<ObjectID, Glob> firstAndLastGlobalUses;
  std::unordered_map<ObjectID, std::unordered_set<uint64_t>> defsPerObj;
  defsPerObj[globalObjID].insert(0);
  firstAndLastGlobalUses[globalObjID].firstUse = 0;
  firstAndLastGlobalUses[globalObjID].lastUse = std::numeric_limits<int>::max();

  std::vector<const TraceInterpreter::Call *> calls;
  for (const auto &p : funcCallStacks) {
    const std::vector<TraceInterpreter::Call> &funcCalls = p.second;
    for (const auto &call : funcCalls) {
      calls.push_back(&call);
    }
  }
  for (const auto &p : objCallStacks) {
    for (const auto &x : p.second) {
      for (const auto &call : x.second) {
        calls.push_back(&call);
      }
    }
  }
  for (const auto *call : calls) {
    for (const auto &piece : call->pieces) {
      auto globalRecordNum = piece.start;
      for (const auto *rec : piece.records) {
        for (ObjectID def : rec->defs()) {
          // Add to the set of defs.
          defsPerObj[def].emplace(globalRecordNum);
        }
        for (ObjectID use : rec->uses()) {
          // Update the last use only if it can't be satisfied by a local def.
          const auto &loc = call->locals.at(use);
          if (loc.lastDefBeforeFirstUse ==
              TraceInterpreter::DefAndUse::kUnused) {
            // It was a global access
            auto &glob = firstAndLastGlobalUses[use];
            if (globalRecordNum < glob.firstUse ||
                glob.firstUse == TraceInterpreter::DefAndUse::kUnused) {
              // Earlier global use or first global use, update.
              glob.firstUse = globalRecordNum;
            }
            if (globalRecordNum > glob.lastUse ||
                glob.lastUse == TraceInterpreter::DefAndUse::kUnused) {
              // Later global use, update.
              glob.lastUse = globalRecordNum;
            }
          }
        }
        globalRecordNum++;
      };
    };
  }
  // For each object, find the max def that is before the first use.
  std::unordered_map<ObjectID, TraceInterpreter::DefAndUse> globalDefsAndUses;
  for (auto &p : firstAndLastGlobalUses) {
    const auto objID = p.first;
    const auto firstUse = p.second.firstUse;
    const auto lastUse = p.second.lastUse;
    assert(
        firstUse <= lastUse &&
        "Should never have the first use be greater than the last use");
    std::unordered_set<uint64_t> &defs = defsPerObj[objID];
    assert(
        defs.size() &&
        "There must be at least one def for any globally used object");
    uint64_t lastDefBeforeFirstUse = *defs.begin();
    for (ObjectID def : defs) {
      if (def < firstUse) {
        lastDefBeforeFirstUse = std::max(lastDefBeforeFirstUse, def);
      }
    }
    assert(
        lastDefBeforeFirstUse <= lastUse &&
        "Should never have the last def before first use be greater than the last use");
    globalDefsAndUses[objID] =
        TraceInterpreter::DefAndUse{lastDefBeforeFirstUse, lastUse};
  }
  return globalDefsAndUses;
}

/// Merge host function calls and host object calls into one set of calls, and
/// compute which values are local to each call.
void createCallMetadataForHostFunctions(
    TraceInterpreter::HostFunctionToCalls *funcCallStacks) {
  for (auto &p : *funcCallStacks) {
    for (TraceInterpreter::Call &call : p.second) {
      createCallMetadata(&call);
    }
  }
}

void createCallMetadataForHostObjects(
    TraceInterpreter::HostObjectToCalls *objCallStacks) {
  for (auto &p : *objCallStacks) {
    for (auto &x : p.second) {
      for (TraceInterpreter::Call &call : x.second) {
        createCallMetadata(&call);
      }
    }
  }
}

Value traceValueToJSIValue(
    Runtime &rt,
    const SynthTrace &trace,
    std::function<Object(ObjectID)> getObjForUse,
    SynthTrace::TraceValue value) {
  if (value.isUndefined()) {
    return Value::undefined();
  }
  if (value.isNull()) {
    return Value::null();
  }
  if (value.isString()) {
    return String::createFromUtf8(rt, trace.decodeString(value));
  }
  if (value.isNumber()) {
    return Value(value.getNumber());
  }
  if (value.isBool()) {
    return Value(value.getBool());
  }
  if (value.isObject()) {
    return getObjForUse(SynthTrace::decodeObject(value));
  }
  llvm_unreachable("Unrecognized value type encountered");
}

std::unique_ptr<const jsi::Buffer> bufConvert(
    std::unique_ptr<llvm::MemoryBuffer> buf) {
  // A jsi::Buffer adapter that owns a llvm::MemoryBuffer.
  class OwnedMemoryBuffer : public jsi::Buffer {
   public:
    OwnedMemoryBuffer(std::unique_ptr<llvm::MemoryBuffer> buffer)
        : data_(std::move(buffer)) {}
    size_t size() const override {
      return data_->getBufferSize();
    }
    const uint8_t *data() const override {
      return reinterpret_cast<const uint8_t *>(data_->getBufferStart());
    }

   private:
    std::unique_ptr<llvm::MemoryBuffer> data_;
  };

  return llvm::make_unique<const OwnedMemoryBuffer>(std::move(buf));
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
          dynamic_cast<const SynthTrace::BeginExecJSRecord &>(*rec);

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
    first = true;
    for (const auto &p : bundles) {
      if (!first) {
        msg += ", ";
      }
      msg += ::hermes::hashAsString(p.first);
      first = false;
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

} // namespace

TraceInterpreter::TraceInterpreter(
    jsi::Runtime &rt,
    const ExecuteOptions &options,
    llvm::raw_ostream *outTrace,
    const SynthTrace &trace,
    std::map<::hermes::SHA1, std::shared_ptr<const Buffer>> bundles,
    const std::unordered_map<ObjectID, TraceInterpreter::DefAndUse>
        &globalDefsAndUses,
    const HostFunctionToCalls &hostFunctionCalls,
    const HostObjectToCalls &hostObjectCalls)
    : rt_(rt),
      options_(options),
      outTrace_(outTrace),
      bundles_(std::move(bundles)),
      trace_(trace),
      globalDefsAndUses_(globalDefsAndUses),
      hostFunctionCalls_(hostFunctionCalls),
      hostObjectCalls_(hostObjectCalls),
      hostFunctions_(),
      hostFunctionsCallCount_(),
      hostObjects_(),
      hostObjectsCallCount_(),
      gom_() {
  // Add the global object to the global object map
  gom_.emplace(trace.globalObjID(), rt.global());
}

/* static */
void TraceInterpreter::exec(
    const std::string &traceFile,
    const std::vector<std::string> &bytecodeFiles,
    const ExecuteOptions &options) {
  // If there is a trace, don't write it out, not used here.
  execFromFileNames(traceFile, bytecodeFiles, options, nullptr);
}

/* static */
std::string TraceInterpreter::execAndGetStats(
    const std::string &traceFile,
    const std::vector<std::string> &bytecodeFiles,
    const ExecuteOptions &options) {
  // If there is a trace, don't write it out, not used here.
  return execFromFileNames(traceFile, bytecodeFiles, options, nullptr);
}

/* static */
void TraceInterpreter::execAndTrace(
    const std::string &traceFile,
    const std::vector<std::string> &bytecodeFiles,
    const ExecuteOptions &options,
    llvm::raw_ostream &outTrace) {
  execFromFileNames(traceFile, bytecodeFiles, options, &outTrace);
}

/* static */
std::string TraceInterpreter::execFromFileNames(
    const std::string &traceFile,
    const std::vector<std::string> &bytecodeFiles,
    const ExecuteOptions &options,
    llvm::raw_ostream *outTrace) {
  auto errorOrFile = llvm::MemoryBuffer::getFile(traceFile);
  if (!errorOrFile) {
    throw std::system_error(errorOrFile.getError());
  }
  std::unique_ptr<llvm::MemoryBuffer> traceBuf = std::move(errorOrFile.get());
  std::vector<std::unique_ptr<llvm::MemoryBuffer>> bytecodeBuffers;
  for (const std::string &bytecode : bytecodeFiles) {
    errorOrFile = llvm::MemoryBuffer::getFile(bytecode);
    if (!errorOrFile) {
      throw std::system_error(errorOrFile.getError());
    }
    bytecodeBuffers.emplace_back(std::move(errorOrFile.get()));
  }
  return execFromMemoryBuffer(
      std::move(traceBuf), std::move(bytecodeBuffers), options, outTrace);
}

/* static */
std::string TraceInterpreter::execFromMemoryBuffer(
    std::unique_ptr<llvm::MemoryBuffer> traceBuf,
    std::vector<std::unique_ptr<llvm::MemoryBuffer>> codeBufs,
    const ExecuteOptions &options,
    llvm::raw_ostream *outTrace) {
  auto traceAndConfigAndEnv = parseSynthTrace(std::move(traceBuf));
  const auto &trace = std::get<0>(traceAndConfigAndEnv);
  bool codeIsMmapped = true;
  for (const auto &buf : codeBufs) {
    if (buf->getBufferKind() != llvm::MemoryBuffer::MemoryBuffer_MMap) {
      // If any of the buffers aren't mmapped, don't turn on I/O tracking.
      codeIsMmapped = false;
      break;
    }
  }
  std::vector<std::unique_ptr<const jsi::Buffer>> bundles;
  for (auto &buf : codeBufs) {
    bundles.emplace_back(bufConvert(std::move(buf)));
  }

  bool isBytecode = true;
  for (const auto &bundle : bundles) {
    if (HermesRuntime::isHermesBytecode(bundle->data(), bundle->size())) {
      // If any of the buffers are source code, don't turn on I/O tracking.
      isBytecode = false;
      break;
    }
  }

  auto &rtConfig = std::get<1>(traceAndConfigAndEnv);
  ::hermes::vm::RuntimeConfig::Builder rtConfigBuilder = rtConfig.rebuild();
  rtConfigBuilder.withBytecodeWarmupPercent(options.bytecodeWarmupPercent);
  rtConfigBuilder.withTrackIO(
      options.shouldTrackIO && isBytecode && codeIsMmapped);
  if (outTrace) {
    // If an out trace is requested, turn on tracing in the VM as well.
    rtConfigBuilder.withTraceEnvironmentInteractions(true);
  }
  ::hermes::vm::GCConfig::Builder gcConfigBuilder =
      rtConfig.getGCConfig().rebuild();
  gcConfigBuilder.withShouldRecordStats(options.shouldPrintGCStats);
  if (options.minHeapSize != 0) {
    gcConfigBuilder.withMinHeapSize(options.minHeapSize);
  }
  if (options.initHeapSize != 0) {
    gcConfigBuilder.withMinHeapSize(options.initHeapSize);
  }
  if (options.maxHeapSize != 0) {
    gcConfigBuilder.withMaxHeapSize(options.maxHeapSize);
  }
  gcConfigBuilder.withOccupancyTarget(options.occupancyTarget);
  gcConfigBuilder.withShouldReleaseUnused(options.shouldReleaseUnused);
  gcConfigBuilder.withAllocInYoung(options.allocInYoung);
  gcConfigBuilder.withRevertToYGAtTTI(options.revertToYGAtTTI);
  gcConfigBuilder.withSanitizeConfig(
      ::hermes::vm::GCSanitizeConfig::Builder()
          .withSanitizeRate(options.sanitizeRate)
          .withRandomSeed(options.sanitizeRandomSeed)
          .build());
  // If aggregating multiple reps, randomize the placement of some data
  // structures in each rep, for a more robust time metric.
  if (options.reps > 1) {
    rtConfigBuilder.withRandomizeMemoryLayout(true);
  }
  rtConfig = rtConfigBuilder.withGCConfig(gcConfigBuilder.build()).build();

  // Map source hashes of files to their memory buffer.
  std::map<::hermes::SHA1, std::shared_ptr<const jsi::Buffer>>
      sourceHashToBundle;
  for (auto &bundle : bundles) {
    ::hermes::SHA1 sourceHash{};
    if (HermesRuntime::isHermesBytecode(bundle->data(), bundle->size())) {
      sourceHash =
          ::hermes::hbc::BCProviderFromBuffer::getSourceHashFromBytecode(
              llvm::makeArrayRef(bundle->data(), bundle->size()));
    } else {
      sourceHash =
          llvm::SHA1::hash(llvm::makeArrayRef(bundle->data(), bundle->size()));
    }
    auto inserted = sourceHashToBundle.insert({sourceHash, std::move(bundle)});
    assert(
        inserted.second &&
        "Duplicate source hash detected, files only need to be supplied once");
    (void)inserted;
  }

  verifyBundlesExist(sourceHashToBundle, trace);

  std::vector<std::string> repGCStats(options.reps);
  for (int rep = -options.warmupReps; rep < options.reps; ++rep) {
    ::hermes::vm::instrumentation::PerfEvents::begin();
    std::unique_ptr<jsi::Runtime> rt;
    std::unique_ptr<HermesRuntime> hermesRuntime = makeHermesRuntime(rtConfig);
    // Set up the mocks for environment-dependent JS behavior
    {
      auto env = std::get<2>(traceAndConfigAndEnv);
      env.stabilizeInstructionCount = options.stabilizeInstructionCount;
      hermesRuntime->setMockedEnvironment(env);
    }
    if (outTrace) {
      rt = makeTracingHermesRuntime(std::move(hermesRuntime), rtConfig);
    } else {
      rt = std::move(hermesRuntime);
    }
    auto stats = exec(*rt, options, trace, sourceHashToBundle, outTrace);
    // If we're not warming up, save the stats.
    if (rep >= 0) {
      repGCStats[rep] = stats;
    }
  }
  return options.shouldPrintGCStats ? mergeGCStats(repGCStats) : "";
}

/* static */
std::string TraceInterpreter::exec(
    jsi::Runtime &rt,
    const ExecuteOptions &options,
    const SynthTrace &trace,
    std::map<::hermes::SHA1, std::shared_ptr<const jsi::Buffer>> bundles,
    llvm::raw_ostream *outTrace) {
  // Partition the records into each call.
  auto funcCallsAndObjectCalls = getCalls(setupFuncID, trace.records());
  auto &hostFuncs = funcCallsAndObjectCalls.first;
  auto &hostObjs = funcCallsAndObjectCalls.second;
  // Write in the metadata.
  createCallMetadataForHostFunctions(&hostFuncs);
  createCallMetadataForHostObjects(&hostObjs);
  assert(
      hostFuncs.at(setupFuncID).size() == 1 &&
      "The setup function should only have one call");
  // Calculate the globals and locals information for the records to know
  // which objects need to be put into a global map data structure.
  const auto globalDefsAndUses =
      createGlobalMap(hostFuncs, hostObjs, trace.globalObjID());
  TraceInterpreter interpreter(
      rt,
      options,
      outTrace,
      trace,
      std::move(bundles),
      globalDefsAndUses,
      hostFuncs,
      hostObjs);
  return interpreter.execEntryFunction(hostFuncs.at(setupFuncID).at(0));
}

Function TraceInterpreter::createHostFunction(
    ObjectID funcID,
    const std::vector<TraceInterpreter::Call> &calls) {
  return Function::createFromHostFunction(
      rt_,
      PropNameID::forAscii(
          rt_,
          std::string("fakeHostFunction") +
              ::hermes::oscompat::to_string(funcID)),
      // Length is irrelevant for host functions.
      0,
      [this, funcID, &calls](
          Runtime &, const Value &thisVal, const Value *args, size_t count)
          -> Value {
        try {
          return execFunction(
              calls.at(hostFunctionsCallCount_[funcID]++),
              thisVal,
              args,
              count);
        } catch (const std::exception &e) {
          crashOnException(e, llvm::None);
        }
      });
}

Object TraceInterpreter::createHostObject(
    ObjectID objID,
    const PropNameToCalls &props) {
  struct FakeHostObject : public HostObject {
    TraceInterpreter &interpreter;
    const PropNameToCalls &props;
    std::unordered_map<std::string, uint64_t> &callCounts;
    FakeHostObject(
        TraceInterpreter &interpreter,
        const PropNameToCalls &props,
        std::unordered_map<std::string, uint64_t> &callCounts)
        : interpreter(interpreter), props(props), callCounts(callCounts) {}
    Value get(Runtime &rt, const PropNameID &name) override {
      try {
        const std::string propName = name.utf8(rt);
        // There are no arguments to pass to a get call.
        return interpreter.execFunction(
            props.at(propName).at(callCounts[propName]++),
            // This is undefined since there's no way to access the host object
            // as a JS value normally from this position.
            Value::undefined(),
            nullptr,
            0);
      } catch (const std::exception &e) {
        interpreter.crashOnException(e, llvm::None);
      }
    }
    void set(Runtime &rt, const PropNameID &name, const Value &value) override {
      try {
        const std::string propName = name.utf8(rt);
        const Value args[] = {Value(rt, value)};
        // There is exactly one argument to pass to a set call.
        interpreter.execFunction(
            props.at(propName).at(callCounts[propName]++),
            // This is undefined since there's no way to access the host object
            // as a JS value normally from this position.
            Value::undefined(),
            args,
            1);
      } catch (const std::exception &e) {
        interpreter.crashOnException(e, llvm::None);
      }
    }
    std::vector<PropNameID> getPropertyNames(Runtime &rt) override {
      // TODO T31386973: Add trace tracking to getPropertyNames
      return {};
    }
  };
  return Object::createFromHostObject(
      rt_,
      std::make_shared<FakeHostObject>(
          *this, props, hostObjectsCallCount_[objID]));
}

std::string TraceInterpreter::execEntryFunction(
    const TraceInterpreter::Call &entryFunc) {
  execFunction(entryFunc, Value::undefined(), nullptr, 0);
  if (outTrace_) {
    // If tracing is also turned on, write out the trace to the given stream.
    dynamic_cast<TracingRuntime &>(rt_).writeTrace(*outTrace_);
  }

#ifdef HERMESVM_PROFILER_BB
  if (auto *hermesRuntime = dynamic_cast<HermesRuntime *>(&rt_)) {
    hermesRuntime->dumpBasicBlockProfileTrace(llvm::errs());
  }
#endif

  if (options_.snapshotMarker == "end") {
    // Take a snapshot at the end if requested.
    if (HermesRuntime *hermesRT = dynamic_cast<HermesRuntime *>(&rt_)) {
      hermesRT->instrumentation().createSnapshotToFile(
          options_.snapshotMarkerFileName);
    } else {
      llvm::errs() << "Heap snapshot requested from non-Hermes runtime\n";
    }
    snapshotMarkerFound_ = true;
  }

  if (!options_.snapshotMarker.empty() && !snapshotMarkerFound_) {
    // Snapshot was requested at a marker but that marker wasn't found.
    throw std::runtime_error(
        std::string("Requested a heap snapshot at \"") +
        options_.snapshotMarker + "\", but that marker wasn't reached\n");
  }

  // If this was a trace then stats were already collected.
  if (options_.marker.empty()) {
    return printStats();
  } else {
    if (!markerFound_) {
      throw std::runtime_error(
          std::string("Marker \"") + options_.marker +
          "\" specified but not found in trace");
    }
    return stats_;
  }
}

Value TraceInterpreter::execFunction(
    const TraceInterpreter::Call &call,
    const Value &thisVal,
    const Value *args,
    uint64_t count) {
  llvm::SaveAndRestore<uint64_t> depthGuard(depth_, depth_ + 1);
  // A mapping from an ObjectID to the Object for local variables.
  std::unordered_map<ObjectID, Object> locals;
  // Save a value so that Call can set it, and Return can access it.
  Value retval;
  // Carry the return value from BeginJSExec to EndJSExec.
  Value overallRetval;
#ifndef NDEBUG
  if (depth_ != 1) {
    RecordType firstRecType = call.pieces.front().records.front()->getType();
    assert(
        (firstRecType == RecordType::CallToNative ||
         firstRecType == RecordType::GetPropertyNative ||
         firstRecType == RecordType::SetPropertyNative) &&
        "Illegal starting record");
  }
#endif
  for (const TraceInterpreter::Call::Piece &piece : call.pieces) {
    uint64_t globalRecordNum = piece.start;
    const auto getObjForUse =
        [this, &call, &locals, &globalRecordNum](ObjectID obj) -> Object {
      // Check locals, then globals.
      auto it = locals.find(obj);
      if (it != locals.end()) {
        // Satisfiable locally
        Object result{Value(rt_, it->second).getObject(rt_)};
        // If it was the last local use, delete that object id from locals.
        auto defAndUse = call.locals.find(obj);
        if (defAndUse != call.locals.end() &&
            defAndUse->second.lastUse == globalRecordNum) {
          assert(
              defAndUse->second.lastDefBeforeFirstUse != DefAndUse::kUnused &&
              "All uses must be preceded by a def");
          locals.erase(it);
        }
        return result;
      }

      // Global use, access out of the map.
      it = gom_.find(obj);
      assert(
          it != gom_.end() &&
          "If there is a global definition, it must exist in the map already");
      Object result{Value(rt_, it->second).getObject(rt_)};
      // If it was the last global use, delete that object id from globals.
      auto defAndUse = globalDefsAndUses_.find(obj);
      assert(
          defAndUse != globalDefsAndUses_.end() &&
          "All global uses must have a global definition");
      if (defAndUse->second.lastUse == globalRecordNum) {
        gom_.erase(it);
      }
      return result;
    };
    for (const SynthTrace::Record *rec : piece.records) {
      try {
        switch (rec->getType()) {
          case RecordType::BeginExecJS: {
            const auto &bejsr =
                dynamic_cast<const SynthTrace::BeginExecJSRecord &>(*rec);
            auto it = bundles_.find(bejsr.sourceHash());
            if (it == bundles_.end()) {
              if (isAllZeroSourceHash(bejsr.sourceHash()) &&
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
              llvm::errs()
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
                dynamic_cast<const SynthTrace::EndExecJSRecord &>(*rec);
            if (eejsr.retVal_.isObject()) {
              addObjectToDefs(
                  call,
                  SynthTrace::decodeObject(eejsr.retVal_),
                  globalRecordNum,
                  std::move(overallRetval).asObject(rt_),
                  locals);
            } else {
              assert(
                  !overallRetval.isObject() &&
                  "Trace expects non-object but actual return was an object");
              auto v =
                  traceValueToJSIValue(rt_, trace_, nullptr, eejsr.retVal_);
              assert(
                  Value::strictEquals(rt_, v, overallRetval) &&
                  "evaluateJavaScript() retval does not match trace");
            }
            // FALLTHROUGH
          }
          case RecordType::Marker: {
            const auto &mr =
                dynamic_cast<const SynthTrace::MarkerRecord &>(*rec);
            // If the tag is the requested tag, and the stats have not already
            // been collected, collect them.
            if (mr.tag_ == options_.marker && !markerFound_) {
              if (stats_.empty()) {
                stats_ = printStats();
              }
              markerFound_ = true;
            }
            if (mr.tag_ == options_.snapshotMarker && !snapshotMarkerFound_) {
              if (HermesRuntime *hermesRT =
                      dynamic_cast<HermesRuntime *>(&rt_)) {
                hermesRT->instrumentation().createSnapshotToFile(
                    options_.snapshotMarkerFileName);
              } else {
                llvm::errs()
                    << "Heap snapshot requested from non-Hermes runtime\n";
              }
              snapshotMarkerFound_ = true;
            }
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
            addObjectToDefs(
                call, cor.objID_, globalRecordNum, Object(rt_), locals);
            break;
          }
          case RecordType::CreateHostObject: {
            const auto &chor =
                static_cast<const SynthTrace::CreateHostObjectRecord &>(*rec);
            const ObjectID objID = chor.objID_;
            auto iterAndDidCreate = hostObjects_.emplace(
                objID, createHostObject(objID, hostObjectCalls_.at(objID)));
            assert(
                iterAndDidCreate.second &&
                "This should always be creating a new host object");
            addObjectToDefs(
                call,
                objID,
                globalRecordNum,
                iterAndDidCreate.first->second,
                locals);
            break;
          }
          case RecordType::CreateHostFunction: {
            const auto &chfr =
                static_cast<const SynthTrace::CreateHostFunctionRecord &>(*rec);
            const ObjectID funcID = chfr.objID_;
            auto iterAndDidCreate = hostFunctions_.emplace(
                funcID,
                createHostFunction(funcID, hostFunctionCalls_.at(funcID)));
            assert(
                iterAndDidCreate.second &&
                "This should always be creating a new host function");
            addObjectToDefs(
                call,
                funcID,
                globalRecordNum,
                iterAndDidCreate.first->second,
                locals);
            break;
          }
          case RecordType::GetProperty: {
            const auto &gpr =
                static_cast<const SynthTrace::GetPropertyRecord &>(*rec);
            // Call get property on the object specified, and possibly define
            // the result.
            auto value =
                getObjForUse(gpr.objID_)
                    .getProperty(rt_, PropNameID::forUtf8(rt_, gpr.propName_));
            if (gpr.value_.isObject()) {
              // If the result of the get property is an object, add that to the
              // definitions.
              addObjectToDefs(
                  call,
                  SynthTrace::decodeObject(gpr.value_),
                  globalRecordNum,
                  std::move(value).asObject(rt_),
                  locals);
            }
            break;
          }
          case RecordType::SetProperty: {
            const auto &spr =
                static_cast<const SynthTrace::SetPropertyRecord &>(*rec);
            // Call set property on the object specified and give it the value.
            getObjForUse(spr.objID_)
                .setProperty(
                    rt_,
                    PropNameID::forUtf8(rt_, spr.propName_),
                    traceValueToJSIValue(
                        rt_, trace_, getObjForUse, spr.value_));
            break;
          }
          case RecordType::HasProperty: {
            const auto &hpr =
                static_cast<const SynthTrace::HasPropertyRecord &>(*rec);
            getObjForUse(hpr.objID_)
                .hasProperty(rt_, PropNameID::forUtf8(rt_, hpr.propName_));
            break;
          }
          case RecordType::GetPropertyNames: {
            const auto &gpnr =
                static_cast<const SynthTrace::GetPropertyNamesRecord &>(*rec);
            jsi::Array arr = getObjForUse(gpnr.objID_).getPropertyNames(rt_);
            addObjectToDefs(
                call,
                gpnr.propNamesID_,
                globalRecordNum,
                std::move(arr),
                locals);
            break;
          }
          case RecordType::CreateArray: {
            const auto &car =
                static_cast<const SynthTrace::CreateArrayRecord &>(*rec);
            // Make an array of the appropriate length to be used.
            addObjectToDefs(
                call,
                car.objID_,
                globalRecordNum,
                Array(rt_, car.length_),
                locals);
            break;
          }
          case RecordType::ArrayRead: {
            const auto &arr =
                static_cast<const SynthTrace::ArrayReadRecord &>(*rec);
            // Read from the specified array, and possibly define the result.
            auto value = getObjForUse(arr.objID_)
                             .asArray(rt_)
                             .getValueAtIndex(rt_, arr.index_);
            if (arr.value_.isObject()) {
              // If the result of the read is an object, add that to the
              // definitions.
              addObjectToDefs(
                  call,
                  SynthTrace::decodeObject(arr.value_),
                  globalRecordNum,
                  std::move(value).asObject(rt_),
                  locals);
            }
            break;
          }
          case RecordType::ArrayWrite: {
            const auto &awr =
                static_cast<const SynthTrace::ArrayWriteRecord &>(*rec);
            // Write to the array and give it the value.
            getObjForUse(awr.objID_)
                .asArray(rt_)
                .setValueAtIndex(
                    rt_,
                    awr.index_,
                    traceValueToJSIValue(
                        rt_, trace_, getObjForUse, awr.value_));
            break;
          }
          case RecordType::CallFromNative: {
            const auto &cfnr =
                static_cast<const SynthTrace::CallFromNativeRecord &>(*rec);
            auto func = getObjForUse(cfnr.functionID_).asFunction(rt_);
            std::vector<Value> args;
            for (const auto arg : cfnr.args_) {
              args.emplace_back(
                  traceValueToJSIValue(rt_, trace_, getObjForUse, arg));
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
                  getObjForUse(SynthTrace::decodeObject(cfnr.thisArg_)),
                  argStart,
                  args.size());
            }
            break;
          }
          case RecordType::ConstructFromNative: {
            const auto &cfnr =
                static_cast<const SynthTrace::ConstructFromNativeRecord &>(
                    *rec);
            // Essentially the same implementation as CallFromNative, except
            // calls the construct path.
            auto func = getObjForUse(cfnr.functionID_).asFunction(rt_);
            std::vector<Value> args;
            for (const auto arg : cfnr.args_) {
              args.emplace_back(
                  traceValueToJSIValue(rt_, trace_, getObjForUse, arg));
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
            const auto &rfnr =
                dynamic_cast<const SynthTrace::ReturnFromNativeRecord &>(*rec);
            return traceValueToJSIValue(
                rt_, trace_, getObjForUse, rfnr.retVal_);
          }
          case RecordType::ReturnToNative: {
            const auto &rtnr =
                dynamic_cast<const SynthTrace::ReturnToNativeRecord &>(*rec);
            if (rtnr.retVal_.isObject()) {
              // Use the retval stored by the previous CallFromNative.
              // The ReturnToNative is always the first record to be executed
              // back in the same call stack as the CallFromNative, so we know
              // nothing could have mutated the object in the meantime.
              addObjectToDefs(
                  call,
                  SynthTrace::decodeObject(rtnr.retVal_),
                  globalRecordNum,
                  std::move(retval).asObject(rt_),
                  locals);
            }
            // If the return value wasn't an object, it can be ignored.
            break;
          }
          case RecordType::CallToNative: {
            const auto &ctnr =
                static_cast<const SynthTrace::CallToNativeRecord &>(*rec);
            // Associate the this arg with its object id.
            if (ctnr.thisArg_.isObject()) {
              addObjectToDefs(
                  call,
                  SynthTrace::decodeObject(ctnr.thisArg_),
                  globalRecordNum,
                  std::move(thisVal).asObject(rt_),
                  locals);
            }
            // Associate each argument with its object id.
            assert(
                ctnr.args_.size() == count &&
                "Called at runtime with a different number of args than the trace expected");
            for (uint64_t i = 0; i < ctnr.args_.size(); ++i) {
              if (ctnr.args_[i].isObject()) {
                addObjectToDefs(
                    call,
                    SynthTrace::decodeObject(ctnr.args_[i]),
                    globalRecordNum,
                    std::move(args[i]).asObject(rt_),
                    locals);
              }
            }
            break;
          }
          case RecordType::GetPropertyNative: {
            assert(count == 0 && "Should have no arguments");
            // Don't add this to the locals, it is not technically provided to
            // the function.
            break;
          }
          case RecordType::GetPropertyNativeReturn: {
            const auto &gpnrr =
                dynamic_cast<const SynthTrace::GetPropertyNativeReturnRecord &>(
                    *rec);
            return traceValueToJSIValue(
                rt_, trace_, getObjForUse, gpnrr.retVal_);
          }
          case RecordType::SetPropertyNative: {
            const auto &spnr =
                static_cast<const SynthTrace::SetPropertyNativeRecord &>(*rec);
            // Associate the single argument with its object id (if it's an
            // object).
            if (spnr.value_.isObject()) {
              assert(
                  count == 1 &&
                  "There should be exactly one argument to SetPropertyNative");
              addObjectToDefs(
                  call,
                  SynthTrace::decodeObject(spnr.value_),
                  globalRecordNum,
                  std::move(args[0]).asObject(rt_),
                  locals);
            }
            break;
          }
          case RecordType::SetPropertyNativeReturn: {
            // Since a SetPropertyNative does not have a return value, return
            // undefined.
            return Value::undefined();
            break;
          }
        }
      } catch (const std::exception &e) {
        crashOnException(e, globalRecordNum);
      }
      // If the top of the stack is reached after a marker flushed the stats,
      // exit early.
      if (depth_ == 1 && markerFound_) {
        return Value::undefined();
      }
      globalRecordNum++;
    }
  }
  if (depth_ != 1) {
    // For the non-entry point, there should always be an explicit
    // ReturnFromNative or Get/SetPropertyNativeReturn which will return early
    // from this function. If there was no explicit return, the trace is
    // malformed.
    llvm_unreachable("There was no return in the call");
  }
  // Return undefined for the entry point, it is ignored anyway.
  return Value::undefined();
}

void TraceInterpreter::addObjectToDefs(
    const Call &call,
    ObjectID objID,
    uint64_t globalRecordNum,
    Object &&obj,
    std::unordered_map<ObjectID, Object> &locals) {
  {
    // Either insert this def into the global map or the local one.
    auto iter = globalDefsAndUses_.find(objID);
    if (iter != globalDefsAndUses_.end() &&
        globalRecordNum == iter->second.lastDefBeforeFirstUse) {
      // This was the last def before a global use, insert into the map.
      assert(
          gom_.find(objID) == gom_.end() &&
          "object already exists in the global map");
      gom_.emplace(objID, std::move(obj));
      return;
    }
  }
  {
    auto iter = locals.find(objID);
    if (iter == locals.end()) {
      auto defAndUse = call.locals.find(objID);
      assert(
          defAndUse != call.locals.end() &&
          "Should always be local def and use information");
      if (defAndUse->second.lastUse != TraceInterpreter::DefAndUse::kUnused) {
        // This is used locally, put into a local set.
        locals.emplace(objID, std::move(obj));
      }
    }
  }
}

void TraceInterpreter::addObjectToDefs(
    const Call &call,
    ObjectID objID,
    uint64_t globalRecordNum,
    const Object &obj,
    std::unordered_map<ObjectID, Object> &locals) {
  return addObjectToDefs(
      call, objID, globalRecordNum, Value(rt_, obj).getObject(rt_), locals);
}

std::string TraceInterpreter::printStats() {
  if (options_.forceGCBeforeStats) {
    rt_.instrumentation().collectGarbage();
  }
  std::string stats = rt_.instrumentation().getRecordedGCStats();
  ::hermes::vm::instrumentation::PerfEvents::endAndInsertStats(stats);
#ifdef HERMESVM_PROFILER_OPCODE
  stats += "\n";
  std::string opcodeOutput;
  llvm::raw_string_ostream os{opcodeOutput};
  if (auto *hermesRuntime = dynamic_cast<HermesRuntime *>(&rt)) {
    hermesRuntime->dumpOpcodeStats(os);
  } else {
    throw std::runtime_error("Unable to cast runtime into HermesRuntime");
  }
  os.flush();
  stats += opcodeOutput;
  stats += "\n";
#endif
  return stats;
}

LLVM_ATTRIBUTE_NORETURN void TraceInterpreter::crashOnException(
    const std::exception &e,
    ::hermes::OptValue<uint64_t> globalRecordNum) {
  llvm::errs() << "An exception occurred while running the benchmark:\n";
  if (globalRecordNum) {
    llvm::errs() << "At record number " << globalRecordNum.getValue() << ":\n";
  }
  llvm::errs() << e.what() << "\n";
  if (outTrace_) {
    llvm::errs() << "Writing out the trace\n";
    dynamic_cast<TracingRuntime &>(rt_).writeTrace(*outTrace_);
    llvm::errs() << "\n";
  } else {
    llvm::errs() << "Pass --trace to get a trace for comparison\n";
  }
  // Do not re-throw, since that will pass back and forth between JS and
  // Native, causing more perturbations to the state of this interpreter,
  // which will cause an assertion to fire.
  // Instead, crash here so that it's clear where the error actually
  // occurred.
  llvm::errs() << "Crashing now\n";
  std::abort();
}

} // namespace tracing
} // namespace hermes
} // namespace facebook

#endif
