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
  struct HostObjectPropNames final : public StackValue {
    ObjectID objID;
    HostObjectPropNames(ObjectID hostObjID) : StackValue(), objID(hostObjID) {}
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
      // name.
      return hostObjIDToNameToRecords[hostObj->objID]
          .propNameToCalls[hostObj->propName];
    } else if (
        const auto *hostObj =
            dynamic_cast<const HostObjectPropNames *>(&stackObj)) {
      return hostObjIDToNameToRecords[hostObj->objID].callsToGetPropertyNames;
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
          createHORec.objID_, TraceInterpreter::HostObjectInfo{});
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
          .propNameToCalls[nativeAccessRec.propName_]
          .emplace_back(
              TraceInterpreter::Call(TraceInterpreter::Call::Piece(recordNum)));
    } else if (rec->getType() == RecordType::GetNativePropertyNames) {
      const auto &nativePropNamesRec =
          static_cast<const SynthTrace::GetNativePropertyNamesRecord &>(*rec);
      // JS asked for all properties on a host object. Add a new call on the
      // stack, and add a call with an empty piece to the object it is calling.
      stack.emplace_back(
          new HostObjectPropNames(nativePropNamesRec.hostObjectID_));
      hostObjIDToNameToRecords[nativePropNamesRec.hostObjectID_]
          .callsToGetPropertyNames.emplace_back(
              TraceInterpreter::Call(TraceInterpreter::Call::Piece(recordNum)));
    } else if (rec->getType() == RecordType::GetNativePropertyNamesReturn) {
      // Set the vector of strings that were returned from the original call.
      const auto &nativePropNamesRec =
          static_cast<const SynthTrace::GetNativePropertyNamesReturnRecord &>(
              *rec);
      // The stack object must be a HostObjectPropNames in order for this to be
      // a return from there.
      hostObjIDToNameToRecords
          [dynamic_cast<const HostObjectPropNames &>(*stack.back()).objID]
              .resultsOfGetPropertyNames.emplace_back(
                  nativePropNamesRec.propNames_);
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
        rec->getType() == RecordType::GetNativePropertyNamesReturn ||
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
  // For Objects, Strings, and PropNameIDs.
  std::unordered_map<ObjectID, Glob> firstAndLastGlobalUses;
  // For Objects, Strings, and PropNameIDs.
  std::unordered_map<ObjectID, std::set<uint64_t>> defsPerObj;

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
    for (const auto &x : p.second.propNameToCalls) {
      for (const auto &call : x.second) {
        calls.push_back(&call);
      }
    }
    for (const auto &call : p.second.callsToGetPropertyNames) {
      calls.push_back(&call);
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
    const std::set<uint64_t> &defs = defsPerObj[objID];
    assert(
        defs.size() &&
        "There must be at least one def for any globally used object");
    const auto firstDefAfterFirstUseIter = defs.upper_bound(firstUse);
    assert(
        firstDefAfterFirstUseIter != defs.begin() &&
        "Must have at least one def before first use.");
    uint64_t lastDefBeforeFirstUse = *std::prev(firstDefAfterFirstUseIter);
    assert(
        lastDefBeforeFirstUse <= lastUse &&
        "Should never have the last def before first use be greater than "
        "the last use");
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
    for (auto &x : p.second.propNameToCalls) {
      for (TraceInterpreter::Call &call : x.second) {
        createCallMetadata(&call);
      }
    }
    for (auto &call : p.second.callsToGetPropertyNames) {
      createCallMetadata(&call);
    }
  }
}

Value traceValueToJSIValue(
    Runtime &rt,
    const SynthTrace &trace,
    std::function<Value(ObjectID)> getJSIValueForUse,
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
  if (value.isObject() || value.isString() || value.isSymbol()) {
    return getJSIValueForUse(value.getUID());
  }
  llvm_unreachable("Unrecognized value type encountered");
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

#ifndef NDEBUG
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

/// Assert that \p val seen at replay matches the recorded \p traceValue
void assertMatch(const SynthTrace::TraceValue &traceValue, const Value &val) {
  if (traceValue.isUndefined()) {
    assert(val.isUndefined() && "type mismatch between trace and replay");
  } else if (traceValue.isNull()) {
    assert(val.isNull() && "type mismatch between trace and replay");
  } else if (traceValue.isBool()) {
    assert(val.isBool() && "type mismatch between trace and replay");
    assert(
        val.getBool() == traceValue.getBool() &&
        "value mismatch between trace and replay");
  } else if (traceValue.isNumber()) {
    assert(val.isNumber() && "type mismatch between trace and replay");
    double valNum = val.getNumber();
    double traceValueNum = traceValue.getNumber();
    assert(
        isDoubleEqual(valNum, traceValueNum) &&
        "value mismatch between trace and replay");
  } else if (traceValue.isString()) {
    assert(val.isString() && "type mismatch between trace and replay");
  } else if (traceValue.isObject()) {
    assert(val.isObject() && "type mismatch between trace and replay");
  } else if (traceValue.isSymbol()) {
    assert(val.isSymbol() && "type mismatch between trace and replay");
  }
}
#endif

} // namespace

TraceInterpreter::TraceInterpreter(
    jsi::Runtime &rt,
    const ExecuteOptions &options,
    const SynthTrace &trace,
    std::map<::hermes::SHA1, std::shared_ptr<const Buffer>> bundles,
    const std::unordered_map<ObjectID, TraceInterpreter::DefAndUse>
        &globalDefsAndUses,
    const HostFunctionToCalls &hostFunctionCalls,
    const HostObjectToCalls &hostObjectCalls)
    : rt_(rt),
      options_(options),
      bundles_(std::move(bundles)),
      trace_(trace),
      globalDefsAndUses_(globalDefsAndUses),
      hostFunctionCalls_(hostFunctionCalls),
      hostObjectCalls_(hostObjectCalls),
      hostFunctions_(),
      hostFunctionsCallCount_(),
      hostObjects_(),
      hostObjectsCallCount_(),
      hostObjectsPropertyNamesCallCount_(),
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
    std::unique_ptr<llvh::raw_ostream> traceStream) {
  assert(traceStream && "traceStream must be provided (precondition)");
  execFromFileNames(traceFile, bytecodeFiles, options, std::move(traceStream));
}

/* static */
std::string TraceInterpreter::execFromFileNames(
    const std::string &traceFile,
    const std::vector<std::string> &bytecodeFiles,
    const ExecuteOptions &options,
    std::unique_ptr<llvh::raw_ostream> traceStream) {
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
  return execFromMemoryBuffer(
      std::move(traceBuf),
      std::move(bytecodeBuffers),
      options,
      std::move(traceStream));
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
  rtConfigBuilder.withTraceEnabled(options.traceEnabled);

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
std::string TraceInterpreter::execFromMemoryBuffer(
    std::unique_ptr<llvh::MemoryBuffer> &&traceBuf,
    std::vector<std::unique_ptr<llvh::MemoryBuffer>> &&codeBufs,
    const ExecuteOptions &options,
    std::unique_ptr<llvh::raw_ostream> traceStream) {
  auto traceAndConfigAndEnv = parseSynthTrace(std::move(traceBuf));
  const auto &trace = std::get<0>(traceAndConfigAndEnv);
  bool codeIsMmapped;
  bool isBytecode;
  std::map<::hermes::SHA1, std::shared_ptr<const jsi::Buffer>>
      sourceHashToBundle = getSourceHashToBundleMap(
          std::move(codeBufs), trace, options, &codeIsMmapped, &isBytecode);
  options.traceEnabled = (traceStream != nullptr);

  const auto &rtConfig = merge(
      std::get<1>(traceAndConfigAndEnv),
      std::get<2>(traceAndConfigAndEnv),
      options,
      codeIsMmapped,
      isBytecode);

  std::vector<std::string> repGCStats(options.reps);
  for (int rep = -options.warmupReps; rep < options.reps; ++rep) {
    ::hermes::vm::instrumentation::PerfEvents::begin();
    std::unique_ptr<jsi::Runtime> rt;
    std::unique_ptr<HermesRuntime> hermesRuntime = makeHermesRuntime(rtConfig);
    // Set up the mocks for environment-dependent JS behavior
    {
      auto env = std::get<3>(traceAndConfigAndEnv);
      env.stabilizeInstructionCount = options.stabilizeInstructionCount;
      hermesRuntime->setMockedEnvironment(env);
    }
    bool tracing = false;
    if (traceStream) {
      tracing = true;
      rt = makeTracingHermesRuntime(
          std::move(hermesRuntime),
          rtConfig,
          std::move(traceStream),
          /* forReplay */ true);
    } else {
      rt = std::move(hermesRuntime);
    }
    auto stats = exec(*rt, options, trace, sourceHashToBundle);
    // If we're not warming up, save the stats.
    if (rep >= 0) {
      repGCStats[rep] = stats;
    }
    // If we're tracing, flush the trace.
    if (tracing) {
      (void)rt->instrumentation().flushAndDisableBridgeTrafficTrace();
    }
  }
  return rtConfig.getGCConfig().getShouldRecordStats()
      ? mergeGCStats(repGCStats)
      : "";
}

/* static */
std::string TraceInterpreter::execFromMemoryBuffer(
    std::unique_ptr<llvh::MemoryBuffer> &&traceBuf,
    std::vector<std::unique_ptr<llvh::MemoryBuffer>> &&codeBufs,
    jsi::Runtime &runtime,
    const ExecuteOptions &options) {
  auto traceAndConfigAndEnv = parseSynthTrace(std::move(traceBuf));
  const auto &trace = std::get<0>(traceAndConfigAndEnv);
  return exec(
      runtime,
      options,
      trace,
      getSourceHashToBundleMap(std::move(codeBufs), trace, options));
}

/* static */
std::string TraceInterpreter::exec(
    jsi::Runtime &rt,
    const ExecuteOptions &options,
    const SynthTrace &trace,
    std::map<::hermes::SHA1, std::shared_ptr<const jsi::Buffer>> bundles) {
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
      trace,
      std::move(bundles),
      globalDefsAndUses,
      hostFuncs,
      hostObjs);
  return interpreter.execEntryFunction(hostFuncs.at(setupFuncID).at(0));
}

Function TraceInterpreter::createHostFunction(
    const SynthTrace::CreateHostFunctionRecord &rec,
    const PropNameID &propNameID) {
  const auto funcID = rec.objID_;
  const std::vector<TraceInterpreter::Call> &calls =
      hostFunctionCalls_.at(funcID);
#ifdef HERMESVM_API_TRACE_DEBUG
  assert(propNameID.utf8(rt_) == rec.functionName_);
#endif
  return Function::createFromHostFunction(
      rt_,
      propNameID,
      rec.paramCount_,
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
          crashOnException(e, llvh::None);
        }
      });
}

Object TraceInterpreter::createHostObject(ObjectID objID) {
  struct FakeHostObject : public HostObject {
    TraceInterpreter &interpreter;
    const HostObjectInfo &hostObjectInfo;
    std::unordered_map<std::string, uint64_t> &callCounts;
    uint64_t &propertyNamesCallCounts;

    FakeHostObject(
        TraceInterpreter &interpreter,
        const HostObjectInfo &hostObjectInfo,
        std::unordered_map<std::string, uint64_t> &callCounts,
        uint64_t &propertyNamesCallCounts)
        : interpreter(interpreter),
          hostObjectInfo(hostObjectInfo),
          callCounts(callCounts),
          propertyNamesCallCounts(propertyNamesCallCounts) {}

    Value get(Runtime &rt, const PropNameID &name) override {
      try {
        const std::string propName = name.utf8(rt);
        // There are no arguments to pass to a get call.
        return interpreter.execFunction(
            hostObjectInfo.propNameToCalls.at(propName).at(
                callCounts[propName]++),
            // This is undefined since there's no way to access the host object
            // as a JS value normally from this position.
            Value::undefined(),
            nullptr,
            0,
            &name);
      } catch (const std::exception &e) {
        interpreter.crashOnException(e, llvh::None);
      }
    }

    void set(Runtime &rt, const PropNameID &name, const Value &value) override {
      try {
        const std::string propName = name.utf8(rt);
        const Value args[] = {Value(rt, value)};
        // There is exactly one argument to pass to a set call.
        interpreter.execFunction(
            hostObjectInfo.propNameToCalls.at(propName).at(
                callCounts[propName]++),
            // This is undefined since there's no way to access the host object
            // as a JS value normally from this position.
            Value::undefined(),
            args,
            1);
      } catch (const std::exception &e) {
        interpreter.crashOnException(e, llvh::None);
      }
    }

    std::vector<PropNameID> getPropertyNames(Runtime &rt) override {
      try {
        const auto callCount = propertyNamesCallCounts++;
        interpreter.execFunction(
            hostObjectInfo.callsToGetPropertyNames.at(callCount),
            // This is undefined since there's no way to access the host object
            // as a JS value normally from this position.
            Value::undefined(),
            nullptr,
            0);
        const std::vector<std::string> &names =
            hostObjectInfo.resultsOfGetPropertyNames.at(callCount);
        std::vector<PropNameID> props;
        for (const std::string &name : names) {
          props.emplace_back(PropNameID::forUtf8(rt, name));
        }
        return props;
      } catch (const std::exception &e) {
        interpreter.crashOnException(e, llvh::None);
      }
    }
  };

  return Object::createFromHostObject(
      rt_,
      std::make_shared<FakeHostObject>(
          *this,
          hostObjectCalls_.at(objID),
          hostObjectsCallCount_[objID],
          hostObjectsPropertyNamesCallCount_[objID]));
}

std::string TraceInterpreter::execEntryFunction(
    const TraceInterpreter::Call &entryFunc) {
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
  execFunction(entryFunc, Value::undefined(), nullptr, 0);

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

Value TraceInterpreter::execFunction(
    const TraceInterpreter::Call &call,
    const Value &thisVal,
    const Value *args,
    uint64_t count,
    const PropNameID *nativePropNameToConsumeAsDef) {
  llvh::SaveAndRestore<uint64_t> depthGuard(depth_, depth_ + 1);
  // A mapping from an ObjectID to the Object for local variables.
  // Invariant: value is Object or String;
  std::unordered_map<ObjectID, Value> locals;
  std::unordered_map<ObjectID, PropNameID> pniLocals;
  // Save a value so that Call can set it, and Return can access it.
  Value retval;
  // Carry the return value from BeginJSExec to EndJSExec.
  Value overallRetval;
#ifndef NDEBUG
  if (depth_ != 1) {
    RecordType firstRecType = call.pieces.front().records.front()->getType();
    assert(
        (firstRecType == RecordType::CallToNative ||
         firstRecType == RecordType::GetNativePropertyNames ||
         firstRecType == RecordType::GetPropertyNative ||
         firstRecType == RecordType::SetPropertyNative) &&
        "Illegal starting record");
  }
#endif
#ifndef NDEBUG
  // We'll want the first record, to verify that it's the one that consumes
  // nativePropNameToConsumeAsDef if that is non-null.
  const SynthTrace::Record *firstRec = call.pieces.at(0).records.at(0);
#endif
  for (const TraceInterpreter::Call::Piece &piece : call.pieces) {
    uint64_t globalRecordNum = piece.start;
    const auto getJSIValueForUseOpt =
        [this, &call, &locals, &globalRecordNum](
            ObjectID obj) -> llvh::Optional<Value> {
      // Check locals, then globals.
      auto it = locals.find(obj);
      if (it != locals.end()) {
        // Satisfiable locally
        Value val{rt_, it->second};
        assert(val.isObject() || val.isString() || val.isSymbol());
        // If it was the last local use, delete that object id from locals.
        auto defAndUse = call.locals.find(obj);
        if (defAndUse != call.locals.end() &&
            defAndUse->second.lastUse == globalRecordNum) {
          assert(
              defAndUse->second.lastDefBeforeFirstUse != DefAndUse::kUnused &&
              "All uses must be preceded by a def");
          locals.erase(it);
        }
        return val;
      }
      auto defAndUse = globalDefsAndUses_.find(obj);
      // Since the use might not be a jsi::Value, it might be found in the
      // locals table for non-Values, and therefore might not be in the global
      // use/def table.
      if (defAndUse == globalDefsAndUses_.end()) {
        return llvh::None;
      }
      it = gom_.find(obj);
      if (it != gom_.end()) {
        Value val{rt_, it->second};
        assert(val.isObject() || val.isString() || val.isSymbol());
        // If it was the last global use, delete that object id from globals.
        if (defAndUse->second.lastUse == globalRecordNum) {
          gom_.erase(it);
        }
        return val;
      }
      return llvh::None;
    };
    const auto getJSIValueForUse =
        [&getJSIValueForUseOpt](ObjectID obj) -> Value {
      auto valOpt = getJSIValueForUseOpt(obj);
      assert(valOpt && "There must be a definition for all uses.");
      return std::move(*valOpt);
    };
    const auto getObjForUse = [this,
                               getJSIValueForUse](ObjectID obj) -> Object {
      return getJSIValueForUse(obj).getObject(rt_);
    };
    const auto getPropNameIDForUse =
        [this, &call, &pniLocals, &globalRecordNum](
            ObjectID obj) -> PropNameID {
      // Check locals, then globals.
      auto it = pniLocals.find(obj);
      if (it != pniLocals.end()) {
        // Satisfiable locally
        PropNameID propNameID{rt_, it->second};
        // If it was the last local use, delete that object id from locals.
        auto defAndUse = call.locals.find(obj);
        if (defAndUse != call.locals.end() &&
            defAndUse->second.lastUse == globalRecordNum) {
          assert(
              defAndUse->second.lastDefBeforeFirstUse != DefAndUse::kUnused &&
              "All uses must be preceded by a def");
          pniLocals.erase(it);
        }
        return propNameID;
      }
      auto defAndUse = globalDefsAndUses_.find(obj);
      assert(
          defAndUse != globalDefsAndUses_.end() &&
          "All global uses must have a global definition");
      it = gpnm_.find(obj);
      assert(
          it != gpnm_.end() &&
          "If there is a global definition, it must exist in one of the maps");
      PropNameID propNameID{rt_, it->second};
      // If it was the last global use, delete that object id from globals.
      if (defAndUse->second.lastUse == globalRecordNum) {
        gpnm_.erase(it);
      }
      return propNameID;
    };

    for (const SynthTrace::Record *rec : piece.records) {
      try {
        switch (rec->getType()) {
          case RecordType::BeginExecJS: {
            const auto &bejsr =
                dynamic_cast<const SynthTrace::BeginExecJSRecord &>(*rec);
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
                dynamic_cast<const SynthTrace::EndExecJSRecord &>(*rec);
            ifObjectAddToDefs(
                eejsr.retVal_,
                std::move(overallRetval),
                call,
                globalRecordNum,
                locals);
            LLVM_FALLTHROUGH;
          }
          case RecordType::Marker: {
            const auto &mr =
                dynamic_cast<const SynthTrace::MarkerRecord &>(*rec);
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
            addJSIValueToDefs(
                call, cor.objID_, globalRecordNum, Object(rt_), locals);
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
            assert(str.asString(rt_).utf8(rt_) == csr.chars_);
            addJSIValueToDefs(
                call, csr.objID_, globalRecordNum, std::move(str), locals);
            break;
          }
          case RecordType::CreatePropNameID: {
            const auto &cpnr =
                static_cast<const SynthTrace::CreatePropNameIDRecord &>(*rec);
            // We perform the calls below for their side effects (for example,
            auto propNameID = [&] {
              switch (cpnr.valueType_) {
                case SynthTrace::CreatePropNameIDRecord::ASCII:
                  return PropNameID::forAscii(
                      rt_, cpnr.chars_.data(), cpnr.chars_.size());
                case SynthTrace::CreatePropNameIDRecord::UTF8:
                  return PropNameID::forUtf8(
                      rt_,
                      reinterpret_cast<const uint8_t *>(cpnr.chars_.data()),
                      cpnr.chars_.size());
                case SynthTrace::CreatePropNameIDRecord::TRACEVALUE: {
                  auto val = traceValueToJSIValue(
                      rt_, trace_, getJSIValueForUse, cpnr.traceValue_);
                  if (val.isSymbol())
                    return PropNameID::forSymbol(rt_, val.getSymbol(rt_));
                  return PropNameID::forString(rt_, val.getString(rt_));
                }
              }
              llvm_unreachable("No other way to construct PropNameID");
            }();
            addPropNameIDToDefs(
                call,
                cpnr.propNameID_,
                globalRecordNum,
                std::move(propNameID),
                pniLocals);
            break;
          }
          case RecordType::CreateHostObject: {
            const auto &chor =
                static_cast<const SynthTrace::CreateHostObjectRecord &>(*rec);
            const ObjectID objID = chor.objID_;
            auto iterAndDidCreate =
                hostObjects_.emplace(objID, createHostObject(objID));
            assert(
                iterAndDidCreate.second &&
                "This should always be creating a new host object");
            addJSIValueToDefs(
                call,
                objID,
                globalRecordNum,
                Value(rt_, iterAndDidCreate.first->second),
                locals);
            break;
          }
          case RecordType::CreateHostFunction: {
            const auto &chfr =
                static_cast<const SynthTrace::CreateHostFunctionRecord &>(*rec);
            auto iterAndDidCreate = hostFunctions_.emplace(
                chfr.objID_,
                createHostFunction(
                    chfr, getPropNameIDForUse(chfr.propNameID_)));
            assert(
                iterAndDidCreate.second &&
                "This should always be creating a new host function");
            addJSIValueToDefs(
                call,
                chfr.objID_,
                globalRecordNum,
                Value(rt_, iterAndDidCreate.first->second),
                locals);
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
            const auto &obj = getObjForUse(gpr.objID_);
            auto propIDValOpt = getJSIValueForUseOpt(gpr.propID_);
            // TODO(T111638575): We have to check whether the value is a string,
            // because we cannot disambiguate Symbols from PropNameIDs.
            if (propIDValOpt && propIDValOpt->isString()) {
              const jsi::String propString = (*propIDValOpt).asString(rt_);
#ifdef HERMESVM_API_TRACE_DEBUG
              assert(propString.utf8(rt_) == gpr.propNameDbg_);
#endif
              value = obj.getProperty(rt_, propString);
            } else {
              auto propNameID = getPropNameIDForUse(gpr.propID_);
#ifdef HERMESVM_API_TRACE_DEBUG
              assert(propNameID.utf8(rt_) == gpr.propNameDbg_);
#endif
              value = obj.getProperty(rt_, propNameID);
            }
            ifObjectAddToDefs(
                gpr.value_, std::move(value), call, globalRecordNum, locals);
            break;
          }
          case RecordType::SetProperty: {
            const auto &spr =
                static_cast<const SynthTrace::SetPropertyRecord &>(*rec);
            auto obj = getObjForUse(spr.objID_);
            // Call set property on the object specified and give it the value.
            auto propIDValOpt = getJSIValueForUseOpt(spr.propID_);
            // TODO(T111638575)
            if (propIDValOpt && propIDValOpt->isString()) {
              const jsi::String propString = (*propIDValOpt).asString(rt_);
#ifdef HERMESVM_API_TRACE_DEBUG
              assert(propString.utf8(rt_) == spr.propNameDbg_);
#endif
              obj.setProperty(
                  rt_,
                  propString,
                  traceValueToJSIValue(
                      rt_, trace_, getJSIValueForUse, spr.value_));
            } else {
              auto propNameID = getPropNameIDForUse(spr.propID_);
#ifdef HERMESVM_API_TRACE_DEBUG
              assert(propNameID.utf8(rt_) == spr.propNameDbg_);
#endif
              obj.setProperty(
                  rt_,
                  propNameID,
                  traceValueToJSIValue(
                      rt_, trace_, getJSIValueForUse, spr.value_));
            }
            break;
          }
          case RecordType::HasProperty: {
            const auto &hpr =
                static_cast<const SynthTrace::HasPropertyRecord &>(*rec);
            auto obj = getObjForUse(hpr.objID_);
            auto propIDValOpt = getJSIValueForUseOpt(hpr.propID_);
            // TODO(T111638575)
            if (propIDValOpt && propIDValOpt->isString()) {
              const jsi::String propString = (*propIDValOpt).asString(rt_);
#ifdef HERMESVM_API_TRACE_DEBUG
              assert(propString.utf8(rt_) == hpr.propNameDbg_);
#endif
              obj.hasProperty(rt_, propString);
            } else {
              auto propNameID = getPropNameIDForUse(hpr.propID_);
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
            jsi::Array arr = getObjForUse(gpnr.objID_).getPropertyNames(rt_);
            addJSIValueToDefs(
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
            addJSIValueToDefs(
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
            ifObjectAddToDefs(
                arr.value_, std::move(value), call, globalRecordNum, locals);
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
                        rt_, trace_, getJSIValueForUse, awr.value_));
            break;
          }
          case RecordType::CallFromNative: {
            const auto &cfnr =
                static_cast<const SynthTrace::CallFromNativeRecord &>(*rec);
            auto func = getObjForUse(cfnr.functionID_).asFunction(rt_);
            std::vector<Value> args;
            for (const auto &arg : cfnr.args_) {
              args.emplace_back(
                  traceValueToJSIValue(rt_, trace_, getJSIValueForUse, arg));
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
                  getObjForUse(cfnr.thisArg_.getUID()),
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
            for (const auto &arg : cfnr.args_) {
              args.emplace_back(
                  traceValueToJSIValue(rt_, trace_, getJSIValueForUse, arg));
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
                rt_, trace_, getJSIValueForUse, rfnr.retVal_);
          }
          case RecordType::ReturnToNative: {
            const auto &rtnr =
                dynamic_cast<const SynthTrace::ReturnToNativeRecord &>(*rec);
            ifObjectAddToDefs(
                rtnr.retVal_, std::move(retval), call, globalRecordNum, locals);
            // If the return value wasn't an object, it can be ignored.
            break;
          }
          case RecordType::CallToNative: {
            const auto &ctnr =
                static_cast<const SynthTrace::CallToNativeRecord &>(*rec);
            // Associate the this arg with its object id.
            ifObjectAddToDefs(
                ctnr.thisArg_,
                std::move(thisVal),
                call,
                globalRecordNum,
                locals,
                /* isThis = */ true);
            // Associate each argument with its object id.
            assert(
                ctnr.args_.size() == count &&
                "Called at runtime with a different number of args than "
                "the trace expected");
            for (uint64_t i = 0; i < ctnr.args_.size(); ++i) {
              ifObjectAddToDefs(
                  ctnr.args_[i],
                  std::move(args[i]),
                  call,
                  globalRecordNum,
                  locals);
            }
            break;
          }
          case RecordType::GetPropertyNative: {
            assert(count == 0 && "Should have no arguments");
            const auto &gpnr =
                static_cast<const SynthTrace::GetPropertyNativeRecord &>(*rec);
            // The propName is a definition.
            assert(nativePropNameToConsumeAsDef);
            // This must be the first record in the call.
            assert(rec == firstRec);
            addPropNameIDToDefs(
                call,
                gpnr.propNameID_,
                globalRecordNum,
                *nativePropNameToConsumeAsDef,
                pniLocals);
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
            assert(
                count == 1 &&
                "There should be exactly one argument to SetPropertyNative");
            // The propName is a definition.
            assert(nativePropNameToConsumeAsDef);
            // This must be the first record in the call.
            assert(rec == firstRec);
            addPropNameIDToDefs(
                call,
                spnr.propNameID_,
                globalRecordNum,
                *nativePropNameToConsumeAsDef,
                pniLocals);
            // Associate the single argument with its object id (if it's an
            // object).
            ifObjectAddToDefs(
                spnr.value_, std::move(args[0]), call, globalRecordNum, locals);
            break;
          }
          case RecordType::SetPropertyNativeReturn: {
            const auto &spnr =
                static_cast<const SynthTrace::SetPropertyNativeRecord &>(*rec);
            // The propName is a definition.
            assert(nativePropNameToConsumeAsDef);
            // This must be the first record in the call.
            assert(rec == firstRec);
            addPropNameIDToDefs(
                call,
                spnr.propNameID_,
                globalRecordNum,
                *nativePropNameToConsumeAsDef,
                pniLocals);
            // Since a SetPropertyNative does not have a return value, return
            // undefined.
            return Value::undefined();
            break;
          }
          case RecordType::GetNativePropertyNames: {
            // Nothing actually needs to happen here, as no defs are provided to
            // the local function. The HostObject already handles accessing the
            // property names.
            break;
          }
          case RecordType::GetNativePropertyNamesReturn: {
            // Accessing the list of property names on a host object doesn't
            // return a JS value.
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

template <typename ValueType>
void TraceInterpreter::addValueToDefs(
    const Call &call,
    SynthTrace::ObjectID valID,
    uint64_t globalRecordNum,
    ValueType &&valRef,
    std::unordered_map<SynthTrace::ObjectID, ValueType> &locals,
    std::unordered_map<SynthTrace::ObjectID, ValueType> &globals) {
  {
    // Either insert this def into the global map or the local one.
    auto iter = globalDefsAndUses_.find(valID);
    if (iter != globalDefsAndUses_.end() &&
        globalRecordNum == iter->second.lastDefBeforeFirstUse) {
      // This was the last def before a global use, insert into the map.
      assert(
          globals.find(valID) == globals.end() &&
          "object already exists in the global map");
      globals.emplace(valID, std::move(valRef));
      return;
    }
  }
  {
    auto iter = locals.find(valID);
    if (iter == locals.end()) {
      auto defAndUse = call.locals.find(valID);
      assert(
          defAndUse != call.locals.end() &&
          "Should always be local def and use information");
      if (defAndUse->second.lastUse != TraceInterpreter::DefAndUse::kUnused) {
        // This is used locally, put into a local set.
        locals.emplace(valID, std::move(valRef));
      }
    }
  }
}

template <typename ValueType>
void TraceInterpreter::addValueToDefs(
    const Call &call,
    SynthTrace::ObjectID valID,
    uint64_t globalRecordNum,
    const ValueType &valRef,
    std::unordered_map<SynthTrace::ObjectID, ValueType> &locals,
    std::unordered_map<SynthTrace::ObjectID, ValueType> &globals) {
  addValueToDefs(
      call, valID, globalRecordNum, ValueType{rt_, valRef}, locals, globals);
}

void TraceInterpreter::ifObjectAddToDefs(
    const SynthTrace::TraceValue &traceValue,
    Value &&val,
    const Call &call,
    uint64_t globalRecordNum,
    std::unordered_map<SynthTrace::ObjectID, jsi::Value> &locals,
    bool isThis) {
#ifndef NDEBUG
  // TODO(T84791675): Include 'this' once all traces are correctly recording it.
  if (!isThis) {
    assertMatch(traceValue, val);
  }
#endif
  if (traceValue.isObject() || traceValue.isString() || traceValue.isSymbol()) {
    addJSIValueToDefs(
        call, traceValue.getUID(), globalRecordNum, std::move(val), locals);
  }
}

void TraceInterpreter::ifObjectAddToDefs(
    const SynthTrace::TraceValue &traceValue,
    const Value &val,
    const Call &call,
    uint64_t globalRecordNum,
    std::unordered_map<SynthTrace::ObjectID, jsi::Value> &locals,
    bool isThis) {
  ifObjectAddToDefs(
      traceValue, Value{rt_, val}, call, globalRecordNum, locals, isThis);
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
