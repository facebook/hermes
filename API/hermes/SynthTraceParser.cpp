/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/SynthTraceParser.h"

#include "hermes/Parser/JSLexer.h"
#include "hermes/Parser/JSONParser.h"
#include "hermes/Support/SourceErrorManager.h"

#include <sstream>

namespace facebook {
namespace hermes {
namespace tracing {

using namespace ::hermes::parser;

namespace {

::hermes::SHA1 parseHashStrAsNumber(llvh::StringRef hashStr) {
  ::hermes::SHA1 sourceHash{};
  // Each byte is 2 characters.
  if (hashStr.size() != 2 * sourceHash.size()) {
    throw std::runtime_error("sourceHash is not the right length");
  }
  for (size_t i = 0; i < sourceHash.size(); ++i) {
    auto byte = llvh::StringRef{hashStr.data() + (i * 2), 2};
    sourceHash[i] =
        ::hermes::parseIntWithRadix</* AllowNumericSeparator */ false>(byte, 16)
            .getValue();
  }
  return sourceHash;
}

JSONObject *parseJSON(
    JSONFactory::Allocator &alloc,
    std::unique_ptr<llvh::MemoryBuffer> stream) {
  JSONFactory factory(alloc);
  ::hermes::SourceErrorManager sm;
  // Convert surrogates, since JSI deals in UTF-8.
  JSONParser parser(factory, std::move(stream), sm, /*convertSurrogates*/ true);
  auto rootObj = parser.parse();
  if (!rootObj) {
    // The source error manager will print to stderr.
    throw std::invalid_argument("JSON invalid");
  }
  return llvh::cast<JSONObject>(rootObj.getValue());
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
  return static_cast<NumericType>(llvh::cast<JSONNumber>(val)->getValue());
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

::hermes::vm::GCConfig::Builder getGCConfig(JSONObject *rtConfig) {
  // This function should extract all fields from GCConfig that can affect
  // performance metrics. Configs for debugging can be ignored.
  ::hermes::vm::GCConfig::Builder gcconf;
  if (!rtConfig) {
    return gcconf;
  }
  auto *val = rtConfig->get("gcConfig");
  if (!val) {
    return gcconf;
  }
  if (val->getKind() != JSONKind::Object) {
    throw std::invalid_argument("gcConfig should be an object");
  }
  auto *gcConfig = llvh::cast<JSONObject>(val);
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
    gcconf.withShouldReleaseUnused(SynthTrace::releaseUnusedFromName(
        llvh::cast<JSONString>(shouldRelease)->c_str()));
  }
  if (auto *name = gcConfig->get("name")) {
    gcconf.withName(llvh::cast<JSONString>(name)->str());
  }
  if (auto *allocInYoung = gcConfig->get("allocInYoung")) {
    gcconf.withAllocInYoung(llvh::cast<JSONBoolean>(allocInYoung)->getValue());
  }
  if (auto *revertAtTTI = gcConfig->get("revertToYGAtTTI")) {
    gcconf.withRevertToYGAtTTI(
        llvh::cast<JSONBoolean>(revertAtTTI)->getValue());
  }
  return gcconf;
}

::hermes::vm::RuntimeConfig::Builder getRuntimeConfig(JSONObject *rtConfig) {
  ::hermes::vm::RuntimeConfig::Builder conf;
  if (!rtConfig) {
    // If the config doesn't exist, return some default values.
    return conf;
  }

  if (auto *maxNumRegisters = rtConfig->get("maxNumRegisters")) {
    conf.withMaxNumRegisters(getNumberAs<unsigned>(maxNumRegisters));
  }
  if (auto *promise = rtConfig->get("ES6Promise")) {
    conf.withES6Promise(llvh::cast<JSONBoolean>(promise)->getValue());
  }
  if (auto *proxy = rtConfig->get("ES6Proxy")) {
    conf.withES6Proxy(llvh::cast<JSONBoolean>(proxy)->getValue());
  }
  if (auto *intl = rtConfig->get("Intl")) {
    conf.withIntl(llvh::cast<JSONBoolean>(intl)->getValue());
  }
  if (auto *microtasks = rtConfig->get("MicrotasksQueue")) {
    conf.withMicrotaskQueue(llvh::cast<JSONBoolean>(microtasks)->getValue());
  }
  if (auto *enableSampledStats = rtConfig->get("enableSampledStats")) {
    conf.withEnableSampledStats(
        llvh::cast<JSONBoolean>(enableSampledStats)->getValue());
  }
  if (auto *vmExperimentFlags = rtConfig->get("vmExperimentFlags")) {
    conf.withVMExperimentFlags(getNumberAs<uint32_t>(vmExperimentFlags));
  }

  return conf;
}

template <template <typename, typename> class Collection>
Collection<std::string, std::allocator<std::string>> getListOfStrings(
    JSONArray *array) {
  Collection<std::string, std::allocator<std::string>> strings;
  std::transform(
      array->begin(),
      array->end(),
      std::back_inserter(strings),
      [](const JSONValue *value) -> std::string {
        if (value->getKind() != JSONKind::String) {
          throw std::invalid_argument("Array should contain only strings");
        }
        return std::string(llvh::cast<JSONString>(value)->c_str());
      });
  return strings;
}

template <template <typename, typename> class Collection>
Collection<
    ::hermes::vm::MockedEnvironment::StatsTable,
    std::allocator<::hermes::vm::MockedEnvironment::StatsTable>>
getListOfStatsTable(JSONArray *array) {
  Collection<
      ::hermes::vm::MockedEnvironment::StatsTable,
      std::allocator<::hermes::vm::MockedEnvironment::StatsTable>>
      calls;
  if (!array) {
    return calls;
  }
  std::transform(
      array->begin(),
      array->end(),
      std::back_inserter(calls),
      [](const JSONValue *value)
          -> ::hermes::vm::MockedEnvironment::StatsTable {
        if (value->getKind() != JSONKind::Object) {
          throw std::invalid_argument("Stats table JSON rep is not object");
        }
        const JSONObject *obj = llvh::cast<JSONObject>(value);
        ::hermes::vm::MockedEnvironment::StatsTable result;
        for (auto name : *obj->getHiddenClass()) {
          auto valForName = obj->at(name->str());
          if (valForName->getKind() == JSONKind::Number) {
            result.try_emplace(
                name->str(), llvh::cast<JSONNumber>(valForName)->getValue());
          } else if (valForName->getKind() == JSONKind::String) {
            result.try_emplace(
                name->str(),
                std::string(llvh::cast<JSONString>(valForName)->c_str()));
          } else {
            throw std::invalid_argument(
                "Stats table kind is not num or string.");
          }
        }
        return result;
      });
  return calls;
}

::hermes::vm::MockedEnvironment getMockedEnvironment(JSONObject *env) {
  auto callsToHermesInternalGetInstrumentedStats =
      getListOfStatsTable<std::deque>(llvh::cast_or_null<JSONArray>(
          env->get("callsToHermesInternalGetInstrumentedStats")));
  return ::hermes::vm::MockedEnvironment{
      callsToHermesInternalGetInstrumentedStats};
}

SynthTrace getTrace(JSONArray *array, SynthTrace::ObjectID globalObjID) {
  using RecordType = SynthTrace::RecordType;
  SynthTrace trace(globalObjID, ::hermes::vm::RuntimeConfig());
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
          return trace.decode(llvh::cast<JSONString>(value)->c_str());
        });
    return values;
  };
  for (auto *val : *array) {
    auto *obj = llvh::cast<JSONObject>(val);
    auto timeFromStart =
        std::chrono::milliseconds(getNumberAs<uint64_t>(obj->get("time"), 0));
    std::stringstream ss;
    RecordType kind;
    ss << llvh::cast<JSONString>(obj->get("type"))->c_str();
    ss >> kind;
    // Common properties, they may not exist on all objects so use a
    // dynamic cast.
    auto *objID = llvh::dyn_cast_or_null<JSONNumber>(obj->get("objID"));
    auto *hostObjID =
        llvh::dyn_cast_or_null<JSONNumber>(obj->get("hostObjectID"));
    auto *funcID = llvh::dyn_cast_or_null<JSONNumber>(obj->get("functionID"));
    auto *propID = llvh::dyn_cast_or_null<JSONNumber>(obj->get("propID"));
    auto *propNameID =
        llvh::dyn_cast_or_null<JSONNumber>(obj->get("propNameID"));
    auto *propName = llvh::dyn_cast_or_null<JSONString>(obj->get("propName"));
    auto *propValue = llvh::dyn_cast_or_null<JSONString>(obj->get("value"));
    auto *arrayIndex = llvh::dyn_cast_or_null<JSONNumber>(obj->get("index"));
    auto *callArgs = llvh::dyn_cast_or_null<JSONArray>(obj->get("args"));
    auto *thisArg = llvh::dyn_cast_or_null<JSONString>(obj->get("thisArg"));
    auto *retval = llvh::dyn_cast_or_null<JSONString>(obj->get("retval"));
    switch (kind) {
      case RecordType::BeginExecJS: {
        std::string sourceURL;
        ::hermes::SHA1 hash{};
        bool sourceIsBytecode{false};
        if (JSONString *sourceURLJSON =
                llvh::dyn_cast_or_null<JSONString>(obj->get("sourceURL"))) {
          sourceURL = sourceURLJSON->str();
        }
        if (JSONString *sourceHash =
                llvh::dyn_cast_or_null<JSONString>(obj->get("sourceHash"))) {
          hash = parseHashStrAsNumber(sourceHash->str());
        }
        if (JSONBoolean *sourceIsBytecodeJson =
                llvh::dyn_cast_or_null<JSONBoolean>(
                    obj->get("sourceIsBytecode"))) {
          sourceIsBytecode = sourceIsBytecodeJson->getValue();
        }
        trace.emplace_back<SynthTrace::BeginExecJSRecord>(
            timeFromStart,
            std::move(sourceURL),
            std::move(hash),
            sourceIsBytecode);
        break;
      }
      case RecordType::EndExecJS:
        trace.emplace_back<SynthTrace::EndExecJSRecord>(
            timeFromStart, trace.decode(retval->c_str()));
        break;
      case RecordType::Marker:
        trace.emplace_back<SynthTrace::MarkerRecord>(
            timeFromStart, llvh::cast<JSONString>(obj->get("tag"))->c_str());
        break;
      case RecordType::CreateObject:
        trace.emplace_back<SynthTrace::CreateObjectRecord>(
            timeFromStart, objID->getValue());
        break;
      case RecordType::DrainMicrotasks: {
        int maxMicrotasksHint = getNumberAs<int>(obj->get("maxMicrotasksHint"));
        trace.emplace_back<SynthTrace::DrainMicrotasksRecord>(
            timeFromStart, maxMicrotasksHint);
        break;
      }
      case RecordType::CreateString: {
        auto encoding =
            llvh::dyn_cast_or_null<JSONString>(obj->get("encoding"));
        bool isAscii = false;
        if (encoding->str() == "ASCII") {
          isAscii = true;
        } else {
          assert(encoding->str() == "UTF-8");
        }
        auto str = llvh::dyn_cast_or_null<JSONString>(obj->get("chars"));
        if (isAscii) {
          trace.emplace_back<SynthTrace::CreateStringRecord>(
              timeFromStart,
              objID->getValue(),
              str->str().data(),
              str->str().size());
        } else {
          trace.emplace_back<SynthTrace::CreateStringRecord>(
              timeFromStart,
              objID->getValue(),
              reinterpret_cast<const uint8_t *>(str->str().data()),
              str->str().size());
        }
        break;
      }
      case RecordType::CreatePropNameID: {
        auto id = llvh::dyn_cast_or_null<JSONNumber>(obj->get("objID"));
        if (propValue) {
          trace.emplace_back<SynthTrace::CreatePropNameIDRecord>(
              timeFromStart, id->getValue(), trace.decode(propValue->c_str()));
        } else {
          auto encoding =
              llvh::dyn_cast_or_null<JSONString>(obj->get("encoding"));
          bool isAscii = false;
          if (encoding->str() == "ASCII") {
            isAscii = true;
          } else {
            assert(encoding->str() == "UTF-8");
          }
          auto str = llvh::dyn_cast_or_null<JSONString>(obj->get("chars"));
          if (isAscii) {
            trace.emplace_back<SynthTrace::CreatePropNameIDRecord>(
                timeFromStart,
                id->getValue(),
                str->str().data(),
                str->str().size());
          } else {
            trace.emplace_back<SynthTrace::CreatePropNameIDRecord>(
                timeFromStart,
                id->getValue(),
                reinterpret_cast<const uint8_t *>(str->str().data()),
                str->str().size());
          }
        }
        break;
      }
      case RecordType::CreateHostObject:
        trace.emplace_back<SynthTrace::CreateHostObjectRecord>(
            timeFromStart, objID->getValue());
        break;
      case RecordType::CreateHostFunction: {
        unsigned paramCount = 0;
        if (JSONNumber *jsonParamCount = llvh::dyn_cast_or_null<JSONNumber>(
                obj->get("parameterCount"))) {
          paramCount = jsonParamCount->getValue();
        }
        std::string functionName;
        if (JSONString *jsonFunctionName =
                llvh::dyn_cast_or_null<JSONString>(obj->get("functionName"))) {
          functionName = jsonFunctionName->str();
        }
        trace.emplace_back<SynthTrace::CreateHostFunctionRecord>(
            timeFromStart,
            objID->getValue(),
            propNameID->getValue(),
#ifdef HERMESVM_API_TRACE_DEBUG
            functionName,
#endif
            paramCount);
        break;
      }
      case RecordType::GetProperty:
        trace.emplace_back<SynthTrace::GetPropertyRecord>(
            timeFromStart,
            objID->getValue(),
            propID->getValue(),
#ifdef HERMESVM_API_TRACE_DEBUG
            std::string(propName->c_str()),
#endif
            trace.decode(propValue->c_str()));
        break;
      case RecordType::SetProperty:
        trace.emplace_back<SynthTrace::SetPropertyRecord>(
            timeFromStart,
            objID->getValue(),
            propID->getValue(),
#ifdef HERMESVM_API_TRACE_DEBUG
            std::string(propName->c_str()),
#endif
            trace.decode(propValue->c_str()));
        break;
      case RecordType::HasProperty:
        trace.emplace_back<SynthTrace::HasPropertyRecord>(
            timeFromStart,
            objID->getValue(),
            propID->getValue()
#ifdef HERMESVM_API_TRACE_DEBUG
                ,
            std::string(propName->c_str())
#endif
        );
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
            timeFromStart,
            hostObjID->getValue(),
            propNameID->getValue(),
            propName->c_str());
        break;
      case RecordType::GetPropertyNativeReturn:
        trace.emplace_back<SynthTrace::GetPropertyNativeReturnRecord>(
            timeFromStart, trace.decode(retval->c_str()));
        break;
      case RecordType::SetPropertyNative:
        trace.emplace_back<SynthTrace::SetPropertyNativeRecord>(
            timeFromStart,
            hostObjID->getValue(),
            propNameID->getValue(),
            propName->c_str(),
            trace.decode(propValue->c_str()));
        break;
      case RecordType::SetPropertyNativeReturn:
        trace.emplace_back<SynthTrace::SetPropertyNativeReturnRecord>(
            timeFromStart);
        break;
      case RecordType::GetNativePropertyNames:
        trace.emplace_back<SynthTrace::GetNativePropertyNamesRecord>(
            timeFromStart, hostObjID->getValue());
        break;
      case RecordType::GetNativePropertyNamesReturn:
        trace.emplace_back<SynthTrace::GetNativePropertyNamesReturnRecord>(
            timeFromStart,
            getListOfStrings<std::vector>(
                llvh::cast<JSONArray>(obj->get("properties"))));
        break;
    }
  }
  return trace;
}

} // namespace

std::tuple<
    SynthTrace,
    ::hermes::vm::RuntimeConfig::Builder,
    ::hermes::vm::GCConfig::Builder,
    ::hermes::vm::MockedEnvironment>
parseSynthTrace(std::unique_ptr<llvh::MemoryBuffer> trace) {
  JSLexer::Allocator alloc;
  JSONObject *root = llvh::cast<JSONObject>(parseJSON(alloc, std::move(trace)));
  if (!llvh::dyn_cast_or_null<JSONNumber>(root->get("globalObjID"))) {
    throw std::invalid_argument(
        "Trace does not have a \"globalObjID\" value that is a number");
  }
  if (!llvh::dyn_cast_or_null<JSONObject>(root->get("env"))) {
    throw std::invalid_argument(
        "Trace does not have an \"env\" value that is an object");
  }
  if (auto *ver = root->get("version")) {
    // Version exists, validate that it is a number, and the correct version.
    if (auto *verNum = llvh::dyn_cast<JSONNumber>(ver)) {
      // version is a number.
      const uint32_t version = verNum->getValue();
      if (version != SynthTrace::synthVersion()) {
        throw std::invalid_argument(
            "Trace version mismatch, expected " +
            std::to_string(SynthTrace::synthVersion()) + ", actual: " +
            std::to_string(static_cast<uint32_t>(verNum->getValue())));
      }
    } else {
      throw std::invalid_argument(
          "version exists, but is not a Number. Found instead value of kind: " +
          std::string(::hermes::parser::JSONKindToString(ver->getKind())));
    }
  }
  // Else, for backwards compatibility, allow no version to be specified, which
  // will imply "latest version".

  auto globalObjID =
      getNumberAs<SynthTrace::ObjectID>(root->get("globalObjID"));
  // Get and parse the records list.
  JSONObject *const rtConfig =
      llvh::cast_or_null<JSONObject>(root->get("runtimeConfig"));
  return std::make_tuple(
      getTrace(llvh::cast<JSONArray>(root->at("trace")), globalObjID),
      getRuntimeConfig(rtConfig),
      getGCConfig(rtConfig),
      getMockedEnvironment(llvh::cast<JSONObject>(root->at("env"))));
}

std::tuple<
    SynthTrace,
    ::hermes::vm::RuntimeConfig::Builder,
    ::hermes::vm::GCConfig::Builder,
    ::hermes::vm::MockedEnvironment>
parseSynthTrace(const std::string &tracefile) {
  return parseSynthTrace(
      std::move(llvh::MemoryBuffer::getFile(tracefile).get()));
}

} // namespace tracing
} // namespace hermes
} // namespace facebook
