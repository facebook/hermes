/**
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
      .mathRandomSeed = mathRandomSeed,
      .callsToDateNow = callsToDateNow,
      .callsToNewDate = callsToNewDate,
      .callsToDateAsFunction = callsToDateAsFunction};
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
        trace.emplace_back<SynthTrace::EndExecJSRecord>(timeFromStart);
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

} // namespace

static std::string doublePrinter(double x) {
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

template <typename C, typename Printer>
static void
printSeqContainer(llvm::raw_ostream &os, const C &args, Printer printer) {
  bool begin = true;
  os << "[";
  for (const auto &arg : args) {
    if (!begin) {
      os << ", ";
    }
    begin = false;
    printer(os, arg);
  }
  os << "]";
}

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
  auto it = std::find(stringTable_.begin(), stringTable_.end(), value);
  uint64_t idx = 0;
  if (it == stringTable_.end()) {
    stringTable_.push_back(value);
    idx = stringTable_.size() - 1;
  } else {
    idx = std::distance(stringTable_.begin(), it);
  }
  // Fake a HermesValue string with a non-pointer. Don't use this value in a
  // GC or it will think the index is a pointer.
  return TraceValue::encodeStringValue(
      reinterpret_cast<::hermes::vm::StringPrimitive *>(idx));
}

const std::string &SynthTrace::decodeString(TraceValue value) const {
  return stringTable_.at(reinterpret_cast<uint64_t>(value.getString()));
}

SynthTrace::TraceValue SynthTrace::encodeObject(ObjectID objID) {
  // Put the id as a pointer. This value should not be GC'ed, since it will
  // mistake the id for a pointer.
  return TraceValue::encodeObjectValue(reinterpret_cast<void *>(objID));
}

SynthTrace::ObjectID SynthTrace::decodeObject(TraceValue value) {
  return reinterpret_cast<ObjectID>(value.getObject());
}

SynthTrace::JSONEncodedString SynthTrace::encode(TraceValue value) const {
  if (value.isUndefined()) {
    return "\"undefined:\"";
  }
  if (value.isNull()) {
    return "\"null:\"";
  }
  if (value.isString()) {
    JSONEncodedString result;
    llvm::raw_string_ostream str(result);
    ::hermes::JSONEmitter emitter{str};
    emitter.emitValue(std::string("string:") + decodeString(value));
    str.flush();
    return result;
  }
  std::stringstream ss;
  if (value.isObject()) {
    ss << "\"object:" << decodeObject(value) << "\"";
  } else if (value.isNumber()) {
    ss << "\"number:" << doublePrinter(value.getDouble()) << "\"";
  } else if (value.isBool()) {
    ss << "\"bool:" << (value.getBool() ? "true" : "false") << "\"";
  }
  auto result = ss.str();
  if (result.empty()) {
    llvm_unreachable("No other values allowed in the trace");
  }
  return result;
}

SynthTrace::TraceValue SynthTrace::decode(
    const SynthTrace::JSONEncodedString &str) {
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

void SynthTrace::Record::toJSON(llvm::raw_ostream &os, const SynthTrace &trace)
    const {
  os << "{";
  toJSONInternal(os, trace);
  os << "}";
}

void SynthTrace::Record::toJSONInternal(
    llvm::raw_ostream &os,
    const SynthTrace &trace) const {
  os << "\"type\": \"" << getType() << "Record\", \"time\": " << time_.count();
}

void SynthTrace::MarkerRecord::toJSONInternal(
    llvm::raw_ostream &os,
    const SynthTrace &trace) const {
  Record::toJSONInternal(os, trace);
  os << ", \"tag\": \"" << tag_ << "\"";
}

void SynthTrace::CreateObjectRecord::toJSONInternal(
    llvm::raw_ostream &os,
    const SynthTrace &trace) const {
  Record::toJSONInternal(os, trace);
  os << ", \"objID\": " << objID_;
}

void SynthTrace::GetOrSetPropertyRecord::toJSONInternal(
    llvm::raw_ostream &os,
    const SynthTrace &trace) const {
  Record::toJSONInternal(os, trace);
  os << ", \"objID\": " << objID_ << ", \"propName\": \"" << propName_
     << "\", \"value\": " << trace.encode(value_);
}

void SynthTrace::HasPropertyRecord::toJSONInternal(
    llvm::raw_ostream &os,
    const SynthTrace &trace) const {
  Record::toJSONInternal(os, trace);
  os << ", \"propName\": \"" << propName_ << "\"";
}

void SynthTrace::GetPropertyNamesRecord::toJSONInternal(
    llvm::raw_ostream &os,
    const SynthTrace &trace) const {
  Record::toJSONInternal(os, trace);
  os << ", \"objID\": " << objID_ << ", \"propNamesID\": " << propNamesID_;
}

void SynthTrace::CreateArrayRecord::toJSONInternal(
    llvm::raw_ostream &os,
    const SynthTrace &trace) const {
  Record::toJSONInternal(os, trace);
  os << ", \"objID\": " << objID_ << ", \"length\": " << length_;
}

void SynthTrace::ArrayReadOrWriteRecord::toJSONInternal(
    llvm::raw_ostream &os,
    const SynthTrace &trace) const {
  Record::toJSONInternal(os, trace);
  os << ", \"objID\": " << objID_ << ", \"index\": " << index_
     << ", \"value\": " << trace.encode(value_);
}

void SynthTrace::CallRecord::toJSONInternal(
    llvm::raw_ostream &os,
    const SynthTrace &trace) const {
  Record::toJSONInternal(os, trace);
  os << ", \"functionID\": " << functionID_
     << ", \"thisArg\": " << trace.encode(thisArg_) << ", \"args\": ";
  printSeqContainer(
      os, args_, [&trace](llvm::raw_ostream &os, const TraceValue &arg) {
        os << trace.encode(arg);
      });
}

void SynthTrace::ReturnRecord::toJSONInternal(
    llvm::raw_ostream &os,
    const SynthTrace &trace) const {
  Record::toJSONInternal(os, trace);
  os << ", \"retval\": " << trace.encode(retVal_);
}

void SynthTrace::GetOrSetPropertyNativeRecord::toJSONInternal(
    llvm::raw_ostream &os,
    const SynthTrace &trace) const {
  Record::toJSONInternal(os, trace);
  os << ", \"hostObjectID\": " << hostObjectID_ << ", \"propName\": \""
     << propName_ << "\"";
}

void SynthTrace::SetPropertyNativeRecord::toJSONInternal(
    llvm::raw_ostream &os,
    const SynthTrace &trace) const {
  GetOrSetPropertyNativeRecord::toJSONInternal(os, trace);
  os << ", \"value\": " << trace.encode(value_);
}

llvm::raw_ostream &operator<<(
    llvm::raw_ostream &os,
    const SynthTrace::Printable &tracePrinter) {
  // Don't need to emit start time, since each time is output with respect to
  // the start time.

  // Version
  os << "{\"version\": " << tracePrinter.trace.synthVersion();

  os << ", \"globalObjID\": " << tracePrinter.trace.globalObjID()
     << ", \"sourceHash\": \""
     << ::hermes::hashAsString(tracePrinter.trace.sourceHash())
     // GC Config section.
     << "\", \"gcConfig\": {\"initHeapSize\": "
     << tracePrinter.conf.getGCConfig().getInitHeapSize()
     << ", \"maxHeapSize\": "
     << tracePrinter.conf.getGCConfig().getMaxHeapSize() << "}";

  // Environment section.
  os << ", \"env\": {\"mathRandomSeed\": " << tracePrinter.env.mathRandomSeed;
  const auto uintPrinter = [](llvm::raw_ostream &os, uint64_t x) { os << x; };
  os << ", \"callsToDateNow\": ";
  printSeqContainer(os, tracePrinter.env.callsToDateNow, uintPrinter);
  os << ", \"callsToNewDate\": ";
  printSeqContainer(os, tracePrinter.env.callsToNewDate, uintPrinter);
  os << ", \"callsToDateAsFunction\": ";
  printSeqContainer(
      os,
      tracePrinter.env.callsToDateAsFunction,
      [](llvm::raw_ostream &os, const std::string &datestr) {
        os << "\"" << datestr << "\"";
      });
  os << "}";

  // Records section.
  os << ", \"trace\": ";
  printSeqContainer(
      os,
      tracePrinter.trace.records(),
      [&tracePrinter](
          llvm::raw_ostream &os,
          const std::unique_ptr<SynthTrace::Record> &rec) {
        rec->toJSON(os, tracePrinter.trace);
      });
  return os << "}";
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
