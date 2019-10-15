/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/SynthTraceParser.h"

#include "hermes/Parser/JSLexer.h"
#include "hermes/Parser/JSONParser.h"
#include "hermes/Support/SourceErrorManager.h"

namespace facebook {
namespace hermes {
namespace tracing {

using namespace ::hermes::parser;

namespace {

::hermes::SHA1 parseHashStrAsNumber(llvm::StringRef hashStr) {
  ::hermes::SHA1 sourceHash;
  // Each byte is 2 characters.
  if (hashStr.size() != 2 * sourceHash.size()) {
    throw std::runtime_error("sourceHash is not the right length");
  }
  for (size_t i = 0; i < sourceHash.size(); ++i) {
    auto byte = llvm::StringRef{hashStr.data() + (i * 2), 2};
    sourceHash[i] = ::hermes::parseIntWithRadix(byte, 16).getValue();
  }
  return sourceHash;
}

JSONObject *parseJSON(
    JSONFactory::Allocator &alloc,
    std::unique_ptr<llvm::MemoryBuffer> stream) {
  JSONFactory factory(alloc);
  ::hermes::SourceErrorManager sm;
  JSONParser parser(factory, std::move(stream), sm);
  auto rootObj = parser.parse();
  if (!rootObj) {
    // The source error manager will print to stderr.
    throw std::invalid_argument("JSON invalid");
  }
  return llvm::cast<JSONObject>(rootObj.getValue());
}

/// Get a number from a JSON value.
/// \throws invalid_argument if it's not a number.
template <typename NumericType>
NumericType getNumberAs(const JSONValue *val) {
  if (!val) {
    throw std::invalid_argument("value doesn't exist");
  }
  if (val->getKind() != JSONKind::Number) {
    throw std::invalid_argument("value is not a number");
  }
  return static_cast<NumericType>(llvm::cast<JSONNumber>(val)->getValue());
}

/// Get a number from a JSON value.
/// Use \param dflt if the JSON value doesn't exist.
/// \throws invalid_argument if it's not a number.
template <typename NumericType>
NumericType getNumberAs(const JSONValue *val, NumericType dflt) {
  if (!val) {
    return dflt;
  } else {
    return getNumberAs<NumericType>(val);
  }
}

::hermes::vm::GCConfig getGCConfig(JSONObject *rtConfig) {
  // This function should extract all fields from GCConfig that can affect
  // performance metrics. Configs for debugging can be ignored.
  ::hermes::vm::GCConfig::Builder gcconf;
  auto *val = rtConfig->get("gcConfig");
  if (!val) {
    return gcconf.build();
  }
  if (val->getKind() != JSONKind::Object) {
    throw std::invalid_argument("gcConfig should be an object");
  }
  auto *gcConfig = llvm::cast<JSONObject>(val);
  if (auto *sz = gcConfig->get("minHeapSize")) {
    gcconf.withMinHeapSize(getNumberAs<::hermes::vm::gcheapsize_t>(sz));
  }
  if (auto *sz = gcConfig->get("initHeapSize")) {
    gcconf.withInitHeapSize(getNumberAs<::hermes::vm::gcheapsize_t>(sz));
  }
  if (auto *sz = gcConfig->get("maxHeapSize")) {
    gcconf.withMaxHeapSize(getNumberAs<::hermes::vm::gcheapsize_t>(sz));
  }
  if (auto *occupancy = gcConfig->get("occupancyTarget")) {
    gcconf.withOccupancyTarget(getNumberAs<double>(occupancy));
  }
  if (auto *threshold = gcConfig->get("effectiveOOMThreshold")) {
    gcconf.withEffectiveOOMThreshold(getNumberAs<unsigned>(threshold));
  }
  if (auto *shouldRelease = gcConfig->get("shouldReleaseUnused")) {
    gcconf.withShouldReleaseUnused(SynthTrace::Printable::releaseUnusedFromName(
        llvm::cast<JSONString>(shouldRelease)->c_str()));
  }
  if (auto *name = gcConfig->get("name")) {
    gcconf.withName(llvm::cast<JSONString>(name)->str());
  }
  if (auto *allocInYoung = gcConfig->get("allocInYoung")) {
    gcconf.withAllocInYoung(llvm::cast<JSONBoolean>(allocInYoung)->getValue());
  }
  if (auto *revertAtTTI = gcConfig->get("revertToYGAtTTI")) {
    gcconf.withRevertToYGAtTTI(
        llvm::cast<JSONBoolean>(revertAtTTI)->getValue());
  }
  return gcconf.build();
}

::hermes::vm::RuntimeConfig getRuntimeConfig(JSONObject *root) {
  JSONValue *val = root->get("runtimeConfig");
  ::hermes::vm::RuntimeConfig::Builder conf;
  if (!val) {
    // If the config doesn't exist, return some default values.
    return conf.build();
  }
  if (val->getKind() != JSONKind::Object) {
    throw std::invalid_argument("runtimeConfig should be an object");
  }
  auto *rtConfig = llvm::cast<JSONObject>(val);

  conf.withGCConfig(getGCConfig(rtConfig));

  if (auto *maxNumRegisters = rtConfig->get("maxNumRegisters")) {
    conf.withMaxNumRegisters(getNumberAs<unsigned>(maxNumRegisters));
  }
  if (auto *symbol = rtConfig->get("ES6Symbol")) {
    conf.withES6Symbol(llvm::cast<JSONBoolean>(symbol)->getValue());
  }
  if (auto *enableSampledStats = rtConfig->get("enableSampledStats")) {
    conf.withEnableSampledStats(
        llvm::cast<JSONBoolean>(enableSampledStats)->getValue());
  }
  if (auto *vmExperimentFlags = rtConfig->get("vmExperimentFlags")) {
    conf.withVMExperimentFlags(getNumberAs<uint32_t>(vmExperimentFlags));
  }

  return conf.build();
}

::hermes::vm::MockedEnvironment getMockedEnvironment(JSONObject *env) {
  auto getListOfNumbers = [](JSONArray *array) -> std::deque<uint64_t> {
    std::deque<uint64_t> calls;
    std::transform(
        array->begin(),
        array->end(),
        std::back_inserter(calls),
        [](const JSONValue *value) { return getNumberAs<uint64_t>(value); });
    return calls;
  };
  auto getListOfStrings = [](JSONArray *array) -> std::deque<std::string> {
    std::deque<std::string> calls;
    std::transform(
        array->begin(),
        array->end(),
        std::back_inserter(calls),
        [](const JSONValue *value) -> std::string {
          if (value->getKind() != JSONKind::String) {
            throw std::invalid_argument("Array should contain only strings");
          }
          return std::string(llvm::cast<JSONString>(value)->c_str());
        });
    return calls;
  };

  if (!llvm::dyn_cast_or_null<JSONNumber>(env->get("mathRandomSeed"))) {
    throw std::invalid_argument("env.mathRandomSeed is not a number");
  }
  std::minstd_rand::result_type mathRandomSeed =
      llvm::cast<JSONNumber>(env->at("mathRandomSeed"))->getValue();
  auto callsToDateNow =
      getListOfNumbers(llvm::cast<JSONArray>(env->at("callsToDateNow")));
  auto callsToNewDate =
      getListOfNumbers(llvm::cast<JSONArray>(env->at("callsToNewDate")));
  auto callsToDateAsFunction =
      getListOfStrings(llvm::cast<JSONArray>(env->at("callsToDateAsFunction")));
  return ::hermes::vm::MockedEnvironment{
      mathRandomSeed, callsToDateNow, callsToNewDate, callsToDateAsFunction};
}

SynthTrace getTrace(
    JSONArray *array,
    SynthTrace::ObjectID globalObjID,
    const ::hermes::SHA1 &sourceHash) {
  using RecordType = SynthTrace::RecordType;
  SynthTrace trace(globalObjID);
  trace.setSourceHash(sourceHash);
  auto getListOfTraceValues =
      [](JSONArray *array,
         SynthTrace &trace) -> std::vector<SynthTrace::TraceValue> {
    std::vector<SynthTrace::TraceValue> values;
    std::transform(
        array->begin(),
        array->end(),
        std::back_inserter(values),
        [&trace](const JSONValue *value) -> SynthTrace::TraceValue {
          if (value->getKind() != JSONKind::String) {
            throw std::invalid_argument("Array should contain only strings");
          }
          return trace.decode(llvm::cast<JSONString>(value)->c_str());
        });
    return values;
  };
  for (auto *val : *array) {
    auto *obj = llvm::cast<JSONObject>(val);
    auto timeFromStart =
        std::chrono::milliseconds(getNumberAs<uint64_t>(obj->get("time"), 0));
    std::stringstream ss;
    RecordType kind;
    ss << llvm::cast<JSONString>(obj->get("type"))->c_str();
    ss >> kind;
    // Common properties, they may not exist on all objects so use a
    // dynamic cast.
    auto *objID = llvm::dyn_cast_or_null<JSONNumber>(obj->get("objID"));
    auto *hostObjID =
        llvm::dyn_cast_or_null<JSONNumber>(obj->get("hostObjectID"));
    auto *funcID = llvm::dyn_cast_or_null<JSONNumber>(obj->get("functionID"));
    auto *propName = llvm::dyn_cast_or_null<JSONString>(obj->get("propName"));
    auto *propValue = llvm::dyn_cast_or_null<JSONString>(obj->get("value"));
    auto *arrayIndex = llvm::dyn_cast_or_null<JSONNumber>(obj->get("index"));
    auto *callArgs = llvm::dyn_cast_or_null<JSONArray>(obj->get("args"));
    auto *thisArg = llvm::dyn_cast_or_null<JSONString>(obj->get("thisArg"));
    auto *retval = llvm::dyn_cast_or_null<JSONString>(obj->get("retval"));
    switch (kind) {
      case RecordType::BeginExecJS:
        trace.emplace_back<SynthTrace::BeginExecJSRecord>(timeFromStart);
        break;
      case RecordType::EndExecJS:
        trace.emplace_back<SynthTrace::EndExecJSRecord>(
            timeFromStart, trace.decode(retval->c_str()));
        break;
      case RecordType::Marker:
        trace.emplace_back<SynthTrace::MarkerRecord>(
            timeFromStart, llvm::cast<JSONString>(obj->get("tag"))->c_str());
        break;
      case RecordType::CreateObject:
        trace.emplace_back<SynthTrace::CreateObjectRecord>(
            timeFromStart, objID->getValue());
        break;
      case RecordType::CreateHostObject:
        trace.emplace_back<SynthTrace::CreateHostObjectRecord>(
            timeFromStart, objID->getValue());
        break;
      case RecordType::CreateHostFunction:
        trace.emplace_back<SynthTrace::CreateHostFunctionRecord>(
            timeFromStart, objID->getValue());
        break;
      case RecordType::GetProperty:
        trace.emplace_back<SynthTrace::GetPropertyRecord>(
            timeFromStart,
            objID->getValue(),
            propName->c_str(),
            trace.decode(propValue->c_str()));
        break;
      case RecordType::SetProperty:
        trace.emplace_back<SynthTrace::SetPropertyRecord>(
            timeFromStart,
            objID->getValue(),
            propName->c_str(),
            trace.decode(propValue->c_str()));
        break;
      case RecordType::HasProperty:
        trace.emplace_back<SynthTrace::HasPropertyRecord>(
            timeFromStart, objID->getValue(), propName->c_str());
        break;
      case RecordType::GetPropertyNames:
        trace.emplace_back<SynthTrace::GetPropertyNamesRecord>(
            timeFromStart,
            objID->getValue(),
            getNumberAs<SynthTrace::ObjectID>(obj->get("propNamesID")));
        break;
      case RecordType::CreateArray:
        trace.emplace_back<SynthTrace::CreateArrayRecord>(
            timeFromStart,
            objID->getValue(),
            getNumberAs<uint64_t>(obj->get("length")));
        break;
      case RecordType::ArrayRead:
        trace.emplace_back<SynthTrace::ArrayReadRecord>(
            timeFromStart,
            objID->getValue(),
            arrayIndex->getValue(),
            trace.decode(propValue->c_str()));
        break;
      case RecordType::ArrayWrite:
        trace.emplace_back<SynthTrace::ArrayWriteRecord>(
            timeFromStart,
            objID->getValue(),
            arrayIndex->getValue(),
            trace.decode(propValue->c_str()));
        break;
      case RecordType::CallFromNative:
        trace.emplace_back<SynthTrace::CallFromNativeRecord>(
            timeFromStart,
            funcID->getValue(),
            trace.decode(thisArg->c_str()),
            getListOfTraceValues(callArgs, trace));
        break;
      case RecordType::ConstructFromNative:
        trace.emplace_back<SynthTrace::ConstructFromNativeRecord>(
            timeFromStart,
            funcID->getValue(),
            trace.decode(thisArg->c_str()),
            getListOfTraceValues(callArgs, trace));
        break;
      case RecordType::ReturnFromNative:
        trace.emplace_back<SynthTrace::ReturnFromNativeRecord>(
            timeFromStart, trace.decode(retval->c_str()));
        break;
      case RecordType::ReturnToNative:
        trace.emplace_back<SynthTrace::ReturnToNativeRecord>(
            timeFromStart, trace.decode(retval->c_str()));
        break;
      case RecordType::CallToNative:
        trace.emplace_back<SynthTrace::CallToNativeRecord>(
            timeFromStart,
            funcID->getValue(),
            trace.decode(thisArg->c_str()),
            getListOfTraceValues(callArgs, trace));
        break;
      case RecordType::GetPropertyNative:
        trace.emplace_back<SynthTrace::GetPropertyNativeRecord>(
            timeFromStart, hostObjID->getValue(), propName->c_str());
        break;
      case RecordType::GetPropertyNativeReturn:
        trace.emplace_back<SynthTrace::GetPropertyNativeReturnRecord>(
            timeFromStart, trace.decode(retval->c_str()));
        break;
      case RecordType::SetPropertyNative:
        trace.emplace_back<SynthTrace::SetPropertyNativeRecord>(
            timeFromStart,
            hostObjID->getValue(),
            propName->c_str(),
            trace.decode(propValue->c_str()));
        break;
      case RecordType::SetPropertyNativeReturn:
        trace.emplace_back<SynthTrace::SetPropertyNativeReturnRecord>(
            timeFromStart);
        break;
      default:
        llvm_unreachable("Not a valid record type");
    }
  }
  return trace;
}

} // namespace

std::tuple<
    SynthTrace,
    ::hermes::vm::RuntimeConfig,
    ::hermes::vm::MockedEnvironment>
parseSynthTrace(std::unique_ptr<llvm::MemoryBuffer> trace) {
  JSLexer::Allocator alloc;
  JSONObject *root = llvm::cast<JSONObject>(parseJSON(alloc, std::move(trace)));
  if (!llvm::dyn_cast_or_null<JSONNumber>(root->get("globalObjID"))) {
    throw std::invalid_argument(
        "Trace does not have a \"globalObjID\" value that is a number");
  }
  if (!llvm::dyn_cast_or_null<JSONObject>(root->get("env"))) {
    throw std::invalid_argument(
        "Trace does not have an \"env\" value that is an object");
  }
  if (auto *ver = root->get("version")) {
    // Version exists, validate that it is a number, and the correct version.
    if (auto *verNum = llvm::dyn_cast<JSONNumber>(ver)) {
      // version is a number.
      const uint32_t version = verNum->getValue();
      if (version != SynthTrace::synthVersion()) {
        throw std::invalid_argument(
            "Trace version mismatch, expected " +
            ::hermes::oscompat::to_string(SynthTrace::synthVersion()) +
            ", actual: " +
            ::hermes::oscompat::to_string(
                static_cast<uint32_t>(verNum->getValue())));
      }
    } else {
      throw std::invalid_argument(
          "version exists, but is not a Number. Found instead value of kind: " +
          std::string(::hermes::parser::JSONKindToString(ver->getKind())));
    }
  }
  // Else, for backwards compatibility, allow no version to be specified, which
  // will imply "latest version".

  ::hermes::SHA1 hash{};
  if (auto *sourceHashAsStr =
          llvm::dyn_cast_or_null<JSONString>(root->get("sourceHash"))) {
    llvm::StringRef hashStr = sourceHashAsStr->str();
    hash = parseHashStrAsNumber(hashStr);
  }

  auto globalObjID =
      getNumberAs<SynthTrace::ObjectID>(root->get("globalObjID"));
  // Get and parse the records list.
  return std::make_tuple(
      getTrace(llvm::cast<JSONArray>(root->at("trace")), globalObjID, hash),
      getRuntimeConfig(root),
      getMockedEnvironment(llvm::cast<JSONObject>(root->at("env"))));
}

std::tuple<
    SynthTrace,
    ::hermes::vm::RuntimeConfig,
    ::hermes::vm::MockedEnvironment>
parseSynthTrace(const std::string &tracefile) {
  return parseSynthTrace(
      std::move(llvm::MemoryBuffer::getFile(tracefile).get()));
}

} // namespace tracing
} // namespace hermes
} // namespace facebook
