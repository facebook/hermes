/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/SynthTraceParser.h"

#include "hermes/Parser/JSLexer.h"
#include "hermes/Parser/JSONParser.h"
#include "hermes/Support/ErrorHandling.h"
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
    ::hermes::hermes_fatal("sourceHash is not the right length");
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
    ::hermes::hermes_fatal("JSON invalid");
  }
  return llvh::cast<JSONObject>(rootObj.getValue());
}

/// Get a number from a JSON value.
template <typename NumericType>
NumericType getNumberAs(const JSONValue *val) {
  if (!val) {
    ::hermes::hermes_fatal("value doesn't exist");
  }
  if (val->getKind() != JSONKind::Number) {
    ::hermes::hermes_fatal("value is not a number");
  }
  return static_cast<NumericType>(llvh::cast<JSONNumber>(val)->getValue());
}

/// Get a number from a JSON value.
/// Use \param dflt if the JSON value doesn't exist.
template <typename NumericType>
NumericType getNumberAs(const JSONValue *val, NumericType dflt) {
  if (!val) {
    return dflt;
  } else {
    return getNumberAs<NumericType>(val);
  }
}

/// Parses the json \p value (which must be a string) as a uint64_t.
uint64_t jsonStringToUint64(const ::hermes::parser::JSONValue *val) {
  if (!val) {
    ::hermes::hermes_fatal("value doesn't exist");
  }

  auto str = llvh::dyn_cast<JSONString>(val);
  if (!str) {
    ::hermes::hermes_fatal("value is not a string");
  }

  uint64_t ret = std::strtoull(str->c_str(), nullptr, 10);
  return ret;
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
    ::hermes::hermes_fatal("gcConfig should be an object");
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
          ::hermes::hermes_fatal("Array should contain only strings");
        }
        return std::string(llvh::cast<JSONString>(value)->c_str());
      });
  return strings;
}

SynthTrace getTrace(
    JSONArray *array,
    std::optional<SynthTrace::ObjectID> globalObjID) {
  using RecordType = SynthTrace::RecordType;
  SynthTrace trace(
      ::hermes::vm::RuntimeConfig(), /* traceStream */ nullptr, globalObjID);
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
            ::hermes::hermes_fatal("Array should contain only strings");
          }
          return trace.decode(llvh::cast<JSONString>(value)->c_str());
        });
    return values;
  };

  auto strToRecordType = [](llvh::StringRef str) {
#define CASE(t)           \
  if (str == #t "Record") \
    return RecordType::t;
    SYNTH_TRACE_RECORD_TYPES(CASE)
#undef CASE
    ::hermes::hermes_fatal("Unknown record type");
  };

  for (auto *val : *array) {
    auto *obj = llvh::cast<JSONObject>(val);
    auto timeFromStart =
        std::chrono::milliseconds(getNumberAs<uint64_t>(obj->get("time"), 0));
    RecordType kind =
        strToRecordType(llvh::cast<JSONString>(obj->get("type"))->str());
    // Common properties, they may not exist on all objects so use a
    // dynamic cast.
    auto *objID = llvh::dyn_cast_or_null<JSONNumber>(obj->get("objID"));
    auto *hostObjID =
        llvh::dyn_cast_or_null<JSONNumber>(obj->get("hostObjectID"));
    auto *funcID = llvh::dyn_cast_or_null<JSONNumber>(obj->get("functionID"));
    auto *propID = llvh::dyn_cast_or_null<JSONString>(obj->get("propID"));
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
      case RecordType::QueueMicrotask: {
        auto callbackID =
            getNumberAs<SynthTrace::ObjectID>(obj->get("callbackID"));
        trace.emplace_back<SynthTrace::QueueMicrotaskRecord>(
            timeFromStart, callbackID);
        break;
      }
      case RecordType::DrainMicrotasks: {
        int maxMicrotasksHint = getNumberAs<int>(obj->get("maxMicrotasksHint"));
        trace.emplace_back<SynthTrace::DrainMicrotasksRecord>(
            timeFromStart, maxMicrotasksHint);
        break;
      }
      case RecordType::CreateBigInt: {
        auto method = llvh::dyn_cast<JSONString>(obj->get("method"));
        SynthTrace::CreateBigIntRecord::Method creationMethod =
            SynthTrace::CreateBigIntRecord::Method::FromUint64;
        if (method->str() == "FromInt64") {
          creationMethod = SynthTrace::CreateBigIntRecord::Method::FromInt64;
        } else {
          assert(method->str() == "FromUint64");
        }
        trace.emplace_back<SynthTrace::CreateBigIntRecord>(
            timeFromStart,
            objID->getValue(),
            creationMethod,
            jsonStringToUint64(obj->get("bits")));
        break;
      }
      case RecordType::BigIntToString: {
        auto *strID = llvh::dyn_cast<JSONNumber>(obj->get("strID"));
        auto *bigintID = llvh::dyn_cast<JSONNumber>(obj->get("bigintID"));
        trace.emplace_back<SynthTrace::BigIntToStringRecord>(
            timeFromStart,
            strID->getValue(),
            bigintID->getValue(),
            getNumberAs<int>(obj->get("radix")));
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
      case RecordType::GetProperty: {
        trace.emplace_back<SynthTrace::GetPropertyRecord>(
            timeFromStart,
            objID->getValue(),
            SynthTrace::decode(propID->str())
#ifdef HERMESVM_API_TRACE_DEBUG
                ,
            std::string(propName->c_str()),
#endif
        );
        break;
      }
      case RecordType::SetProperty:
        trace.emplace_back<SynthTrace::SetPropertyRecord>(
            timeFromStart,
            objID->getValue(),
            SynthTrace::decode(propID->str()),
#ifdef HERMESVM_API_TRACE_DEBUG
            std::string(propName->c_str()),
#endif
            trace.decode(propValue->c_str()));
        break;
      case RecordType::HasProperty:
        trace.emplace_back<SynthTrace::HasPropertyRecord>(
            timeFromStart,
            objID->getValue(),
            SynthTrace::decode(propID->str())
#ifdef HERMESVM_API_TRACE_DEBUG
                ,
            std::string(propName->c_str())
#endif
        );
        break;
      case RecordType::GetPropertyNames: {
        trace.emplace_back<SynthTrace::GetPropertyNamesRecord>(
            timeFromStart, objID->getValue());
        break;
      }
      case RecordType::CreateArray:
        trace.emplace_back<SynthTrace::CreateArrayRecord>(
            timeFromStart,
            objID->getValue(),
            getNumberAs<uint64_t>(obj->get("length")));
        break;
      case RecordType::ArrayRead: {
        trace.emplace_back<SynthTrace::ArrayReadRecord>(
            timeFromStart, objID->getValue(), arrayIndex->getValue());
        break;
      }
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
      case RecordType::GetNativePropertyNamesReturn: {
        auto *pnids =
            llvh::dyn_cast_or_null<JSONArray>(obj->get("propNameIDs"));
        trace.emplace_back<SynthTrace::GetNativePropertyNamesReturnRecord>(
            timeFromStart, getListOfTraceValues(pnids, trace));
        break;
      }
      case RecordType::SetExternalMemoryPressure: {
        size_t amount = getNumberAs<size_t>(obj->get("amount"));
        trace.emplace_back<SynthTrace::SetExternalMemoryPressureRecord>(
            timeFromStart, objID->getValue(), amount);
        break;
      }
      case RecordType::Utf8: {
        auto *objId = llvh::dyn_cast_or_null<JSONString>(obj->get("objID"));
        trace.emplace_back<SynthTrace::Utf8Record>(
            timeFromStart, SynthTrace::decode(objId->str()), retval->c_str());
        break;
      }
      case RecordType::Global: {
        trace.emplace_back<SynthTrace::GlobalRecord>(
            timeFromStart, objID->getValue());
        break;
      }
    }
  }
  return trace;
}

} // namespace

std::tuple<
    SynthTrace,
    ::hermes::vm::RuntimeConfig::Builder,
    ::hermes::vm::GCConfig::Builder>
parseSynthTrace(std::unique_ptr<llvh::MemoryBuffer> trace) {
  JSLexer::Allocator alloc;
  JSONObject *root = llvh::cast<JSONObject>(parseJSON(alloc, std::move(trace)));
  if (auto *ver = root->get("version")) {
    // Version exists, validate that it is a number, and the correct version.
    if (auto *verNum = llvh::dyn_cast<JSONNumber>(ver)) {
      // version is a number.
      const uint32_t version = verNum->getValue();
      if (version != SynthTrace::synthVersion()) {
        ::hermes::hermes_fatal(
            "Trace version mismatch, expected " +
            std::to_string(SynthTrace::synthVersion()) + ", actual: " +
            std::to_string(static_cast<uint32_t>(verNum->getValue())));
      }
    } else {
      ::hermes::hermes_fatal(
          "version exists, but is not a Number. Found instead value of kind: " +
          std::string(::hermes::parser::JSONKindToString(ver->getKind())));
    }
  }
  // Else, for backwards compatibility, allow no version to be specified, which
  // will imply "latest version".

  auto *gid = llvh::dyn_cast_or_null<JSONNumber>(root->get("globalObjID"));
  std::optional<SynthTrace::ObjectID> globalObjID = gid
      ? getNumberAs<SynthTrace::ObjectID>(gid)
      : std::optional<SynthTrace::ObjectID>();
  // Get and parse the records list.
  JSONObject *const rtConfig =
      llvh::cast_or_null<JSONObject>(root->get("runtimeConfig"));
  return std::make_tuple(
      getTrace(llvh::cast<JSONArray>(root->at("trace")), globalObjID),
      getRuntimeConfig(rtConfig),
      getGCConfig(rtConfig));
}

std::tuple<
    SynthTrace,
    ::hermes::vm::RuntimeConfig::Builder,
    ::hermes::vm::GCConfig::Builder>
parseSynthTrace(const std::string &tracefile) {
  return parseSynthTrace(
      std::move(llvh::MemoryBuffer::getFile(tracefile).get()));
}

} // namespace tracing
} // namespace hermes
} // namespace facebook
