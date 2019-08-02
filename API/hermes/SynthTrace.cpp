/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "SynthTrace.h"

#include "hermes/Parser/JSLexer.h"
#include "hermes/Parser/JSONParser.h"
#include "hermes/Support/Conversions.h"
#include "hermes/Support/JSONEmitter.h"
#include "hermes/Support/SourceErrorManager.h"
#include "hermes/Support/UTF8.h"
#include "hermes/VM/MockedEnvironment.h"

#include "llvm/Support/Endian.h"
#include "llvm/Support/NativeFormatting.h"
#include "llvm/Support/raw_ostream.h"

#include <cmath>

namespace facebook {
namespace hermes {
namespace tracing {

namespace {

using namespace ::hermes::parser;
using RecordType = SynthTrace::RecordType;
using JSONEmitter = ::hermes::JSONEmitter;

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

::hermes::vm::RuntimeConfig getRuntimeConfig(JSONObject *root) {
  auto *val = root->get("gcConfig");
  ::hermes::vm::RuntimeConfig::Builder conf;
  if (!val) {
    // If the config doesn't exist, return some default values.
    return conf.build();
  }
  if (val->getKind() != JSONKind::Object) {
    throw std::invalid_argument("gcConfig should be an object");
  }
  auto *gcConfig = llvm::cast<JSONObject>(val);
  ::hermes::vm::GCConfig::Builder gcconf;
  if (auto sz = getNumberAs<::hermes::vm::gcheapsize_t>(
          gcConfig->get("initHeapSize"))) {
    gcconf.withInitHeapSize(sz);
  }
  if (auto sz = getNumberAs<::hermes::vm::gcheapsize_t>(
          gcConfig->get("maxHeapSize"))) {
    gcconf.withMaxHeapSize(sz);
  }
  return conf.withGCConfig(gcconf.build()).build();
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

SynthTrace getTrace(
    JSONArray *array,
    SynthTrace::ObjectID globalObjID,
    const ::hermes::SHA1 &sourceHash) {
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

double decodeNumber(const std::string &numberAsString) {
  // Assume the original platform and the current platform are both little
  // endian for simplicity.
  static_assert(
      llvm::support::endian::system_endianness() ==
          llvm::support::endianness::little,
      "Only little-endian decoding allowed");
  assert(
      numberAsString.substr(0, 2) == std::string("0x") && "Invalid hex number");
  // NOTE: This should use stoul, but that's not defined on Android.
  std::stringstream ss;
  ss << std::hex << numberAsString;
  uint64_t x;
  ss >> x;
  return ::hermes::safeTypeCast<uint64_t, double>(x);
}

std::string doublePrinter(double x) {
  // Encode a number as its little-endian hex encoding.
  // NOTE: While this could use big-endian, it makes it more complicated for
  // JS to read out (forces use of a DataView rather than a Float64Array)
  static_assert(
      llvm::support::endian::system_endianness() ==
          llvm::support::endianness::little,
      "Only little-endian encoding allowed");
  std::string result;
  llvm::raw_string_ostream resultStream{result};
  llvm::write_hex(
      resultStream,
      ::hermes::safeTypeCast<double, uint64_t>(x),
      llvm::HexPrintStyle::PrefixLower,
      16);
  resultStream.flush();
  return result;
}

} // namespace

SynthTrace::TraceValue SynthTrace::encodeUndefined() {
  return TraceValue::encodeUndefinedValue();
}

SynthTrace::TraceValue SynthTrace::encodeNull() {
  return TraceValue::encodeNullValue();
}

SynthTrace::TraceValue SynthTrace::encodeBool(bool value) {
  return TraceValue::encodeBoolValue(value);
}

SynthTrace::TraceValue SynthTrace::encodeNumber(double value) {
  return TraceValue::encodeNumberValue(value);
}

SynthTrace::TraceValue SynthTrace::encodeString(const std::string &value) {
  auto idx = stringTable_.insert(value);
  // Fake a HermesValue string with a non-pointer. Don't use this value in a
  // GC or it will think the index is a pointer.
  return TraceValue::encodeStringValue(
      reinterpret_cast<::hermes::vm::StringPrimitive *>(idx));
}

const std::string &SynthTrace::decodeString(TraceValue value) const {
  return stringTable_[reinterpret_cast<uintptr_t>(value.getString())];
}

SynthTrace::TraceValue SynthTrace::encodeObject(ObjectID objID) {
  // Put the id as a pointer. This value should not be GC'ed, since it will
  // mistake the id for a pointer.
  return TraceValue::encodeObjectValue(reinterpret_cast<void *>(objID));
}

SynthTrace::ObjectID SynthTrace::decodeObject(TraceValue value) {
  return reinterpret_cast<ObjectID>(value.getObject());
}

std::string SynthTrace::encode(TraceValue value) const {
  if (value.isUndefined()) {
    return "undefined:";
  } else if (value.isNull()) {
    return "null:";
  } else if (value.isString()) {
    // This is not properly escaped yet, and must be passed through
    // JSONEmitter::emitValue before it can be put into JSON.
    return std::string("string:") + decodeString(value);
  } else if (value.isObject()) {
    return std::string("object:") +
        ::hermes::oscompat::to_string(decodeObject(value));
  } else if (value.isNumber()) {
    return std::string("number:") + doublePrinter(value.getDouble());
  } else if (value.isBool()) {
    return std::string("bool:") + (value.getBool() ? "true" : "false");
  } else {
    llvm_unreachable("No other values allowed in the trace");
  }
}

SynthTrace::TraceValue SynthTrace::decode(const std::string &str) {
  auto location = str.find(':');
  assert(location < str.size() && "Must contain a type tag");
  auto tag = std::string(str.begin(), str.begin() + location);
  // Don't include the colon, so add 1
  auto rest = std::string(str.begin() + location + 1, str.end());
  // This should be a switch, but C++ doesn't allow strings in switch
  // statements.
  if (tag == "undefined") {
    return encodeUndefined();
  } else if (tag == "null") {
    return encodeNull();
  } else if (tag == "bool") {
    return encodeBool(rest == "true");
  } else if (tag == "number") {
    return encodeNumber(decodeNumber(rest));
  } else if (tag == "string") {
    return encodeString(rest);
  } else if (tag == "object") {
    return encodeObject(std::atol(rest.c_str()));
  } else {
    llvm_unreachable("Illegal object encountered");
  }
}

/* static */
std::tuple<
    SynthTrace,
    ::hermes::vm::RuntimeConfig,
    ::hermes::vm::MockedEnvironment>
SynthTrace::parse(std::unique_ptr<llvm::MemoryBuffer> trace) {
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
      if (version != synthVersion()) {
        throw std::invalid_argument(
            "Trace version mismatch, expected " +
            ::hermes::oscompat::to_string(synthVersion()) + ", actual: " +
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

  ObjectID globalObjID = getNumberAs<ObjectID>(root->get("globalObjID"));
  // Get and parse the records list.
  return std::make_tuple(
      getTrace(llvm::cast<JSONArray>(root->at("trace")), globalObjID, hash),
      getRuntimeConfig(root),
      getMockedEnvironment(llvm::cast<JSONObject>(root->at("env"))));
}

/* static */
std::tuple<
    SynthTrace,
    ::hermes::vm::RuntimeConfig,
    ::hermes::vm::MockedEnvironment>
SynthTrace::parse(const std::string &tracefile) {
  return parse(std::move(llvm::MemoryBuffer::getFile(tracefile).get()));
}

void SynthTrace::Record::toJSON(JSONEmitter &json, const SynthTrace &trace)
    const {
  json.openDict();
  toJSONInternal(json, trace);
  json.closeDict();
}

bool SynthTrace::MarkerRecord::operator==(const Record &that) const {
  return Record::operator==(that) &&
      tag_ == dynamic_cast<const MarkerRecord &>(that).tag_;
}

bool SynthTrace::ReturnMixin::operator==(const ReturnMixin &that) const {
  return equal(retVal_, that.retVal_);
}

bool SynthTrace::EndExecJSRecord::operator==(const Record &that) const {
  if (!Record::operator==(that)) {
    return false;
  }
  auto thatCasted = dynamic_cast<const EndExecJSRecord &>(that);
  return MarkerRecord::operator==(thatCasted) &&
      ReturnMixin::operator==(thatCasted);
}

bool SynthTrace::CreateObjectRecord::operator==(const Record &that) const {
  if (!Record::operator==(that)) {
    return false;
  }
  auto thatCasted = dynamic_cast<const CreateObjectRecord &>(that);
  return objID_ == thatCasted.objID_;
}

bool SynthTrace::GetOrSetPropertyRecord::operator==(const Record &that) const {
  if (!Record::operator==(that)) {
    return false;
  }
  auto thatCasted = dynamic_cast<const GetOrSetPropertyRecord *>(&that);
  return objID_ == thatCasted->objID_ && propName_ == thatCasted->propName_ &&
      equal(value_, thatCasted->value_);
}

bool SynthTrace::HasPropertyRecord::operator==(const Record &that) const {
  if (!Record::operator==(that)) {
    return false;
  }
  auto thatCasted = dynamic_cast<const HasPropertyRecord &>(that);
  return objID_ == thatCasted.objID_ && propName_ == thatCasted.propName_;
}

bool SynthTrace::GetPropertyNamesRecord::operator==(const Record &that) const {
  if (!Record::operator==(that)) {
    return false;
  }
  auto thatCasted = dynamic_cast<const GetPropertyNamesRecord &>(that);
  return objID_ == thatCasted.objID_ && propNamesID_ == thatCasted.propNamesID_;
}

bool SynthTrace::CreateArrayRecord::operator==(const Record &that) const {
  if (!Record::operator==(that)) {
    return false;
  }
  auto thatCasted = dynamic_cast<const CreateArrayRecord &>(that);
  return objID_ == thatCasted.objID_ && length_ == thatCasted.length_;
}

bool SynthTrace::ArrayReadOrWriteRecord::operator==(const Record &that) const {
  if (!Record::operator==(that)) {
    return false;
  }
  auto thatCasted = dynamic_cast<const ArrayReadOrWriteRecord *>(&that);
  return objID_ == thatCasted->objID_ && index_ == thatCasted->index_ &&
      value_.getRaw() == thatCasted->value_.getRaw();
}

bool SynthTrace::CallRecord::operator==(const Record &that) const {
  if (!Record::operator==(that)) {
    return false;
  }
  auto thatCasted = dynamic_cast<const CallRecord *>(&that);
  return functionID_ == thatCasted->functionID_ &&
      std::equal(
             args_.begin(),
             args_.end(),
             thatCasted->args_.begin(),
             [](TraceValue x, TraceValue y) { return equal(x, y); });
}

bool SynthTrace::ReturnFromNativeRecord::operator==(const Record &that) const {
  if (!Record::operator==(that)) {
    return false;
  }
  return ReturnMixin::operator==(
      dynamic_cast<const ReturnFromNativeRecord &>(that));
}

bool SynthTrace::ReturnToNativeRecord::operator==(const Record &that) const {
  if (!Record::operator==(that)) {
    return false;
  }
  return ReturnMixin::operator==(dynamic_cast<const ReturnMixin &>(that));
}

bool SynthTrace::GetOrSetPropertyNativeRecord::operator==(
    const Record &that) const {
  if (!Record::operator==(that)) {
    return false;
  }
  auto thatCasted = dynamic_cast<const GetOrSetPropertyNativeRecord *>(&that);
  return hostObjectID_ == thatCasted->hostObjectID_ &&
      propName_ == thatCasted->propName_;
}

bool SynthTrace::GetPropertyNativeRecord::operator==(const Record &that) const {
  if (!Record::operator==(that)) {
    return false;
  }
  auto thatCasted = dynamic_cast<const GetPropertyNativeRecord &>(that);
  return GetOrSetPropertyNativeRecord::operator==(thatCasted);
}

bool SynthTrace::GetPropertyNativeReturnRecord::operator==(
    const Record &that) const {
  if (!Record::operator==(that)) {
    return false;
  }
  auto thatCasted = dynamic_cast<const GetPropertyNativeReturnRecord &>(that);
  return ReturnMixin::operator==(thatCasted);
}

bool SynthTrace::SetPropertyNativeRecord::operator==(const Record &that) const {
  if (!Record::operator==(that)) {
    return false;
  }
  auto thatCasted = dynamic_cast<const SetPropertyNativeRecord &>(that);
  return hostObjectID_ == thatCasted.hostObjectID_ &&
      propName_ == thatCasted.propName_ && equal(value_, thatCasted.value_);
}

void SynthTrace::Record::toJSONInternal(JSONEmitter &json, const SynthTrace &)
    const {
  std::string storage;
  llvm::raw_string_ostream os{storage};
  os << getType() << "Record";
  os.flush();

  json.emitKeyValue("type", storage);
  json.emitKeyValue("time", time_.count());
}

void SynthTrace::MarkerRecord::toJSONInternal(
    JSONEmitter &json,
    const SynthTrace &trace) const {
  Record::toJSONInternal(json, trace);
  json.emitKeyValue("tag", tag_);
}

void SynthTrace::CreateObjectRecord::toJSONInternal(
    JSONEmitter &json,
    const SynthTrace &trace) const {
  Record::toJSONInternal(json, trace);
  json.emitKeyValue("objID", objID_);
}

void SynthTrace::GetOrSetPropertyRecord::toJSONInternal(
    JSONEmitter &json,
    const SynthTrace &trace) const {
  Record::toJSONInternal(json, trace);
  json.emitKeyValue("objID", objID_);
  json.emitKeyValue("propName", propName_);
  json.emitKeyValue("value", trace.encode(value_));
}

void SynthTrace::HasPropertyRecord::toJSONInternal(
    JSONEmitter &json,
    const SynthTrace &trace) const {
  Record::toJSONInternal(json, trace);
  json.emitKeyValue("propName", propName_);
}

void SynthTrace::GetPropertyNamesRecord::toJSONInternal(
    JSONEmitter &json,
    const SynthTrace &trace) const {
  Record::toJSONInternal(json, trace);
  json.emitKeyValue("objID", objID_);
  json.emitKeyValue("propNamesID", propNamesID_);
}

void SynthTrace::CreateArrayRecord::toJSONInternal(
    JSONEmitter &json,
    const SynthTrace &trace) const {
  Record::toJSONInternal(json, trace);
  json.emitKeyValue("objID", objID_);
  json.emitKeyValue("length", length_);
}

void SynthTrace::ArrayReadOrWriteRecord::toJSONInternal(
    JSONEmitter &json,
    const SynthTrace &trace) const {
  Record::toJSONInternal(json, trace);
  json.emitKeyValue("objID", objID_);
  json.emitKeyValue("index", index_);
  json.emitKeyValue("value", trace.encode(value_));
}

void SynthTrace::CallRecord::toJSONInternal(
    JSONEmitter &json,
    const SynthTrace &trace) const {
  Record::toJSONInternal(json, trace);
  json.emitKeyValue("functionID", functionID_);
  json.emitKeyValue("thisArg", trace.encode(thisArg_));
  json.emitKey("args");
  json.openArray();
  for (const TraceValue &arg : args_) {
    json.emitValue(trace.encode(arg));
  }
  json.closeArray();
}

void SynthTrace::ReturnMixin::toJSONInternal(
    JSONEmitter &json,
    const SynthTrace &trace) const {
  json.emitKeyValue("retval", trace.encode(retVal_));
}

void SynthTrace::EndExecJSRecord::toJSONInternal(
    ::hermes::JSONEmitter &json,
    const SynthTrace &trace) const {
  MarkerRecord::toJSONInternal(json, trace);
  ReturnMixin::toJSONInternal(json, trace);
}

void SynthTrace::ReturnFromNativeRecord::toJSONInternal(
    ::hermes::JSONEmitter &json,
    const SynthTrace &trace) const {
  Record::toJSONInternal(json, trace);
  ReturnMixin::toJSONInternal(json, trace);
}

void SynthTrace::ReturnToNativeRecord::toJSONInternal(
    ::hermes::JSONEmitter &json,
    const SynthTrace &trace) const {
  Record::toJSONInternal(json, trace);
  ReturnMixin::toJSONInternal(json, trace);
}

void SynthTrace::GetOrSetPropertyNativeRecord::toJSONInternal(
    JSONEmitter &json,
    const SynthTrace &trace) const {
  Record::toJSONInternal(json, trace);
  json.emitKeyValue("hostObjectID", hostObjectID_);
  json.emitKeyValue("propName", propName_);
}

void SynthTrace::GetPropertyNativeReturnRecord::toJSONInternal(
    JSONEmitter &json,
    const SynthTrace &trace) const {
  Record::toJSONInternal(json, trace);
  ReturnMixin::toJSONInternal(json, trace);
}

void SynthTrace::SetPropertyNativeRecord::toJSONInternal(
    JSONEmitter &json,
    const SynthTrace &trace) const {
  GetOrSetPropertyNativeRecord::toJSONInternal(json, trace);
  json.emitKeyValue("value", trace.encode(value_));
}

llvm::raw_ostream &operator<<(
    llvm::raw_ostream &os,
    const SynthTrace::Printable &tracePrinter) {
  // Don't need to emit start time, since each time is output with respect to
  // the start time.

  JSONEmitter json{os};
  {
    // Global section.
    json.openDict();
    json.emitKeyValue("version", tracePrinter.trace.synthVersion());
    json.emitKeyValue("globalObjID", tracePrinter.trace.globalObjID());
    json.emitKeyValue(
        "sourceHash", ::hermes::hashAsString(tracePrinter.trace.sourceHash()));

    // GCConfig section.
    {
      json.emitKey("gcConfig");
      json.openDict();
      json.emitKeyValue(
          "initHeapSize", tracePrinter.conf.getGCConfig().getInitHeapSize());
      json.emitKeyValue(
          "maxHeapSize", tracePrinter.conf.getGCConfig().getMaxHeapSize());
      json.closeDict();
    }

    {
      // Environment section.
      json.emitKey("env");
      json.openDict();
      json.emitKeyValue("mathRandomSeed", tracePrinter.env.mathRandomSeed);

      json.emitKey("callsToDateNow");
      json.openArray();
      for (uint64_t dateNow : tracePrinter.env.callsToDateNow) {
        json.emitValue(dateNow);
      }
      json.closeArray();

      json.emitKey("callsToNewDate");
      json.openArray();
      for (uint64_t newDate : tracePrinter.env.callsToNewDate) {
        json.emitValue(newDate);
      }
      json.closeArray();

      json.emitKey("callsToDateAsFunction");
      json.openArray();
      for (const std::string &dateAsFunc :
           tracePrinter.env.callsToDateAsFunction) {
        json.emitValue(dateAsFunc);
      }
      json.closeArray();

      json.closeDict();
    }

    {
      // Records section.
      json.emitKey("trace");
      json.openArray();
      for (const std::unique_ptr<SynthTrace::Record> &rec :
           tracePrinter.trace.records()) {
        rec->toJSON(json, tracePrinter.trace);
      }
      json.closeArray();
    }

    json.closeDict();
  }
  return os;
}

llvm::raw_ostream &operator<<(
    llvm::raw_ostream &os,
    SynthTrace::RecordType type) {
#define CASE(t)                   \
  case SynthTrace::RecordType::t: \
    return os << #t
  switch (type) {
    CASE(BeginExecJS);
    CASE(EndExecJS);
    CASE(Marker);
    CASE(CreateObject);
    CASE(CreateHostObject);
    CASE(CreateHostFunction);
    CASE(GetProperty);
    CASE(SetProperty);
    CASE(HasProperty);
    CASE(GetPropertyNames);
    CASE(CreateArray);
    CASE(ArrayRead);
    CASE(ArrayWrite);
    CASE(CallFromNative);
    CASE(ConstructFromNative);
    CASE(ReturnFromNative);
    CASE(ReturnToNative);
    CASE(CallToNative);
    CASE(GetPropertyNative);
    CASE(GetPropertyNativeReturn);
    CASE(SetPropertyNative);
    CASE(SetPropertyNativeReturn);
  }
#undef CASE
  // This only exists to appease gcc.
  return os;
}

std::istream &operator>>(std::istream &is, SynthTrace::RecordType &type) {
  std::string kindstr;
  is >> kindstr;
  // Can't use switch on strings, use if statements instead.
  // There's definitely a faster way to do this, but this code isn't critical.
#define CASE(t)                              \
  if (kindstr == std::string(#t "Record")) { \
    type = SynthTrace::RecordType::t;        \
    return is;                               \
  }
  CASE(BeginExecJS)
  CASE(EndExecJS)
  CASE(Marker)
  CASE(CreateObject)
  CASE(CreateHostObject)
  CASE(CreateHostFunction)
  CASE(GetProperty)
  CASE(SetProperty)
  CASE(HasProperty)
  CASE(GetPropertyNames);
  CASE(CreateArray)
  CASE(ArrayRead)
  CASE(ArrayWrite)
  CASE(CallFromNative)
  CASE(ConstructFromNative)
  CASE(ReturnFromNative)
  CASE(ReturnToNative)
  CASE(CallToNative)
  CASE(GetPropertyNative)
  CASE(GetPropertyNativeReturn)
  CASE(SetPropertyNative)
  CASE(SetPropertyNativeReturn)
#undef CASE
  return is;
}

} // namespace tracing
} // namespace hermes
} // namespace facebook
