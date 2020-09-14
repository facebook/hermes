/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifdef HERMESVM_API_TRACE

#include "SynthTrace.h"

#include "hermes/Support/Algorithms.h"
#include "hermes/Support/Conversions.h"
#include "hermes/Support/JSONEmitter.h"
#include "hermes/Support/OSCompat.h"
#include "hermes/Support/UTF8.h"
#include "hermes/VM/MockedEnvironment.h"
#include "hermes/VM/StringPrimitive.h"

#include "llvh/Support/Endian.h"
#include "llvh/Support/NativeFormatting.h"
#include "llvh/Support/raw_ostream.h"

#include <cmath>

namespace facebook {
namespace hermes {
namespace tracing {

namespace {

using RecordType = SynthTrace::RecordType;
using JSONEmitter = ::hermes::JSONEmitter;
using ::hermes::parser::JSONArray;
using ::hermes::parser::JSONObject;
using ::hermes::parser::JSONValue;

double decodeNumber(const std::string &numberAsString) {
  // Assume the original platform and the current platform are both little
  // endian for simplicity.
  static_assert(
      llvh::support::endian::system_endianness() ==
          llvh::support::endianness::little,
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
      llvh::support::endian::system_endianness() ==
          llvh::support::endianness::little,
      "Only little-endian encoding allowed");
  std::string result;
  llvh::raw_string_ostream resultStream{result};
  llvh::write_hex(
      resultStream,
      ::hermes::safeTypeCast<double, uint64_t>(x),
      llvh::HexPrintStyle::PrefixLower,
      16);
  resultStream.flush();
  return result;
}

} // namespace

bool SynthTrace::TraceValue::operator==(const TraceValue &that) const {
  if (tag_ != that.tag_) {
    return false;
  }
  switch (tag_) {
    case Tag::Bool:
      return val_.b == that.val_.b;
    case Tag::Number:
      // For now, ignore differences that result from NaN, +0/-0.
      return val_.n == that.val_.n;
    case Tag::Object:
    case Tag::String:
    case Tag::PropNameID:
      return val_.uid == that.val_.uid;
    default:
      return true;
  }
}

SynthTrace::SynthTrace(
    ObjectID globalObjID,
    const ::hermes::vm::RuntimeConfig &conf,
    std::unique_ptr<llvh::raw_ostream> traceStream)
    : traceStream_(std::move(traceStream)), globalObjID_(globalObjID) {
  if (traceStream_) {
    json_ = std::make_unique<JSONEmitter>(*traceStream_, /*pretty*/ true);
    json_->openDict();
    json_->emitKeyValue("version", synthVersion());
    json_->emitKeyValue("globalObjID", globalObjID_);

    // RuntimeConfig section.
    json_->emitKey("runtimeConfig");
    json_->openDict();
    {
      json_->emitKey("gcConfig");
      json_->openDict();
      json_->emitKeyValue("minHeapSize", conf.getGCConfig().getMinHeapSize());
      json_->emitKeyValue("initHeapSize", conf.getGCConfig().getInitHeapSize());
      json_->emitKeyValue("maxHeapSize", conf.getGCConfig().getMaxHeapSize());
      json_->emitKeyValue(
          "occupancyTarget", conf.getGCConfig().getOccupancyTarget());
      json_->emitKeyValue(
          "effectiveOOMThreshold",
          conf.getGCConfig().getEffectiveOOMThreshold());
      json_->emitKeyValue(
          "shouldReleaseUnused",
          nameFromReleaseUnused(conf.getGCConfig().getShouldReleaseUnused()));
      json_->emitKeyValue("name", conf.getGCConfig().getName());
      json_->emitKeyValue("allocInYoung", conf.getGCConfig().getAllocInYoung());
      json_->emitKeyValue(
          "revertToYGAtTTI", conf.getGCConfig().getRevertToYGAtTTI());
      json_->closeDict();
    }
    json_->emitKeyValue("maxNumRegisters", conf.getMaxNumRegisters());
    json_->emitKeyValue("ES6Proxy", conf.getES6Proxy());
    json_->emitKeyValue("ES6Symbol", conf.getES6Symbol());
    json_->emitKeyValue("enableSampledStats", conf.getEnableSampledStats());
    json_->emitKeyValue("vmExperimentFlags", conf.getVMExperimentFlags());
    json_->closeDict();

    // Build properties section
    json_->emitKey("buildProperties");
    json_->openDict();
    json_->emitKeyValue("nativePointerSize", sizeof(void *));
    json_->emitKeyValue(
        "allowCompressedPointers",
#ifdef HERMESVM_ALLOW_COMPRESSED_POINTERS
        true
#else
        false
#endif

    );
    json_->emitKeyValue(
        "debugBuild",
#ifdef NDEBUG
        false
#else
        true
#endif
    );
    json_->emitKeyValue(
        "slowDebug",
#ifdef HERMES_SLOW_DEBUG
        true
#else
        false
#endif
    );
    json_->emitKeyValue(
        "enableDebugger",
#ifdef HERMES_ENABLE_DEBUGGER
        true
#else
        false
#endif
    );
    json_->emitKeyValue(
        "enableIRInstrumentation",
#ifdef HERMES_ENABLE_IR_INSTRUMENTATION
        true
#else
        false
#endif
    );
    // The size of this type was a thing that varied between 64-bit Android and
    // 64-bit desktop Linux builds.  A config flag now allows us to build on
    // Linux desktop in a way that matches the sizes.
    json_->emitKeyValue(
        "sizeofExternalASCIIStringPrimitive",
        sizeof(::hermes::vm::ExternalASCIIStringPrimitive));
    json_->closeDict();

    // Both the top-level dict and the trace array remain open.  The latter is
    // added to during execution.  Both are closed by flushAndDisable.
    json_->emitKey("trace");
    json_->openArray();
  }
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

SynthTrace::TraceValue SynthTrace::encodeObject(ObjectID objID) {
  return TraceValue::encodeObjectValue(objID);
}

SynthTrace::TraceValue SynthTrace::encodeString(ObjectID objID) {
  return TraceValue::encodeStringValue(objID);
}

SynthTrace::TraceValue SynthTrace::encodePropNameID(ObjectID objID) {
  return TraceValue::encodePropNameIDValue(objID);
}

/*static*/
std::string SynthTrace::encode(TraceValue value) {
  if (value.isUndefined()) {
    return "undefined:";
  } else if (value.isNull()) {
    return "null:";
  } else if (value.isObject()) {
    return std::string("object:") +
        ::hermes::oscompat::to_string(value.getUID());
  } else if (value.isString()) {
    return std::string("string:") +
        ::hermes::oscompat::to_string(value.getUID());
  } else if (value.isPropNameID()) {
    return std::string("propNameID:") +
        ::hermes::oscompat::to_string(value.getUID());
  } else if (value.isNumber()) {
    return std::string("number:") + doublePrinter(value.getNumber());
  } else if (value.isBool()) {
    return std::string("bool:") + (value.getBool() ? "true" : "false");
  } else {
    llvm_unreachable("No other values allowed in the trace");
  }
}

/*static*/
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
  } else if (tag == "object") {
    return encodeObject(std::atol(rest.c_str()));
  } else if (tag == "string") {
    return encodeString(std::atol(rest.c_str()));
  } else if (tag == "propNameID") {
    return encodePropNameID(std::atol(rest.c_str()));
  } else {
    llvm_unreachable("Illegal object encountered");
  }
}

void SynthTrace::Record::toJSON(JSONEmitter &json) const {
  json.openDict();
  toJSONInternal(json);
  json.closeDict();
}

bool SynthTrace::MarkerRecord::operator==(const Record &that) const {
  return Record::operator==(that) &&
      tag_ == dynamic_cast<const MarkerRecord &>(that).tag_;
}

bool SynthTrace::ReturnMixin::operator==(const ReturnMixin &that) const {
  return retVal_ == that.retVal_;
}

bool SynthTrace::BeginExecJSRecord::operator==(const Record &that) const {
  if (!Record::operator==(that)) {
    return false;
  }
  const auto &thatCasted = dynamic_cast<const BeginExecJSRecord &>(that);
  return sourceURL_ == thatCasted.sourceURL_ &&
      sourceHash_ == thatCasted.sourceHash_ &&
      sourceIsBytecode_ == thatCasted.sourceIsBytecode_;
}

bool SynthTrace::EndExecJSRecord::operator==(const Record &that) const {
  if (!Record::operator==(that)) {
    return false;
  }
  const auto &thatCasted = dynamic_cast<const EndExecJSRecord &>(that);
  return MarkerRecord::operator==(thatCasted) &&
      ReturnMixin::operator==(thatCasted);
}

bool SynthTrace::CreateObjectRecord::operator==(const Record &that) const {
  if (!Record::operator==(that)) {
    return false;
  }
  const auto &thatCasted = dynamic_cast<const CreateObjectRecord &>(that);
  return objID_ == thatCasted.objID_;
}

bool SynthTrace::CreateStringRecord::operator==(const Record &that) const {
  if (!Record::operator==(that)) {
    return false;
  }
  auto &thatCasted = dynamic_cast<const CreateStringRecord &>(that);
  return objID_ == thatCasted.objID_ && ascii_ == thatCasted.ascii_ &&
      chars_ == thatCasted.chars_;
}

bool SynthTrace::CreatePropNameIDRecord::operator==(const Record &that) const {
  if (!Record::operator==(that)) {
    return false;
  }
  auto &thatCasted = dynamic_cast<const CreatePropNameIDRecord &>(that);
  return propNameID_ == thatCasted.propNameID_ && ascii_ == thatCasted.ascii_ &&
      chars_ == thatCasted.chars_;
}

bool SynthTrace::CreateHostFunctionRecord::operator==(
    const Record &that) const {
  if (!CreateObjectRecord::operator==(that)) {
    return false;
  }
  const auto &thatCasted = dynamic_cast<const CreateHostFunctionRecord &>(that);
  if (!(propNameID_ == thatCasted.propNameID_ &&
        paramCount_ == thatCasted.paramCount_)) {
    return false;
  }
#ifdef HERMESVM_API_TRACE_DEBUG
  assert(functionName_ == thatCasted.functionName_);
#endif
  return true;
}

bool SynthTrace::GetOrSetPropertyRecord::operator==(const Record &that) const {
  if (!Record::operator==(that)) {
    return false;
  }
  const auto &thatCasted = dynamic_cast<const GetOrSetPropertyRecord &>(that);
  if (!(objID_ == thatCasted.objID_ && propID_ == thatCasted.propID_ &&
        value_ == thatCasted.value_)) {
    return false;
  }
#ifdef HERMESVM_API_TRACE_DEBUG
  assert(propNameDbg_ == thatCasted.propNameDbg_);
#endif
  return true;
}

bool SynthTrace::HasPropertyRecord::operator==(const Record &that) const {
  if (!Record::operator==(that)) {
    return false;
  }
  const auto &thatCasted = dynamic_cast<const HasPropertyRecord &>(that);
  return objID_ == thatCasted.objID_ && propID_ == thatCasted.propID_;
}

bool SynthTrace::GetPropertyNamesRecord::operator==(const Record &that) const {
  if (!Record::operator==(that)) {
    return false;
  }
  const auto &thatCasted = dynamic_cast<const GetPropertyNamesRecord &>(that);
  return objID_ == thatCasted.objID_ && propNamesID_ == thatCasted.propNamesID_;
}

bool SynthTrace::CreateArrayRecord::operator==(const Record &that) const {
  if (!Record::operator==(that)) {
    return false;
  }
  const auto &thatCasted = dynamic_cast<const CreateArrayRecord &>(that);
  return objID_ == thatCasted.objID_ && length_ == thatCasted.length_;
}

bool SynthTrace::ArrayReadOrWriteRecord::operator==(const Record &that) const {
  if (!Record::operator==(that)) {
    return false;
  }
  const auto &thatCasted = dynamic_cast<const ArrayReadOrWriteRecord &>(that);
  return objID_ == thatCasted.objID_ && index_ == thatCasted.index_ &&
      value_ == thatCasted.value_;
}

bool SynthTrace::CallRecord::operator==(const Record &that) const {
  if (!Record::operator==(that)) {
    return false;
  }
  const auto &thatCasted = dynamic_cast<const CallRecord &>(that);
  return functionID_ == thatCasted.functionID_ &&
      std::equal(
             args_.begin(),
             args_.end(),
             thatCasted.args_.begin(),
             [](TraceValue x, TraceValue y) { return x == y; });
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
      propNameID_ == thatCasted->propNameID_ &&
      propName_ == thatCasted->propName_;
}

bool SynthTrace::GetPropertyNativeRecord::operator==(const Record &that) const {
  return GetOrSetPropertyNativeRecord::operator==(that);
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
  return GetOrSetPropertyNativeRecord::operator==(that) &&
      value_ == dynamic_cast<const SetPropertyNativeRecord &>(that).value_;
}

bool SynthTrace::GetNativePropertyNamesRecord::operator==(
    const Record &that) const {
  if (!Record::operator==(that)) {
    return false;
  }
  auto &thatCasted = dynamic_cast<const GetNativePropertyNamesRecord &>(that);
  return hostObjectID_ == thatCasted.hostObjectID_;
}

bool SynthTrace::GetNativePropertyNamesReturnRecord::operator==(
    const Record &that) const {
  if (!Record::operator==(that)) {
    return false;
  }
  auto &thatCasted =
      dynamic_cast<const GetNativePropertyNamesReturnRecord &>(that);
  return propNames_ == thatCasted.propNames_;
}

void SynthTrace::Record::toJSONInternal(JSONEmitter &json) const {
  std::string storage;
  llvh::raw_string_ostream os{storage};
  os << getType() << "Record";
  os.flush();

  json.emitKeyValue("type", storage);
  json.emitKeyValue("time", time_.count());
}

void SynthTrace::MarkerRecord::toJSONInternal(JSONEmitter &json) const {
  Record::toJSONInternal(json);
  json.emitKeyValue("tag", tag_);
}

void SynthTrace::CreateObjectRecord::toJSONInternal(JSONEmitter &json) const {
  Record::toJSONInternal(json);
  json.emitKeyValue("objID", objID_);
}

static std::string encodingName(bool isASCII) {
  return isASCII ? "ASCII" : "UTF-8";
}

void SynthTrace::CreateStringRecord::toJSONInternal(JSONEmitter &json) const {
  Record::toJSONInternal(json);
  json.emitKeyValue("objID", objID_);
  json.emitKeyValue("encoding", encodingName(ascii_));
  json.emitKeyValue("chars", llvh::StringRef(chars_.data(), chars_.size()));
}

void SynthTrace::CreatePropNameIDRecord::toJSONInternal(
    JSONEmitter &json) const {
  Record::toJSONInternal(json);
  json.emitKeyValue("objID", propNameID_);
  json.emitKeyValue("encoding", encodingName(ascii_));
  json.emitKeyValue("chars", llvh::StringRef(chars_.data(), chars_.size()));
}

void SynthTrace::CreateHostFunctionRecord::toJSONInternal(
    JSONEmitter &json) const {
  CreateObjectRecord::toJSONInternal(json);
  json.emitKeyValue("propNameID", propNameID_);
  json.emitKeyValue("parameterCount", paramCount_);
#ifdef HERMESVM_API_TRACE_DEBUG
  json.emitKeyValue("functionName", functionName_);
#endif
}

void SynthTrace::GetOrSetPropertyRecord::toJSONInternal(
    JSONEmitter &json) const {
  Record::toJSONInternal(json);
  json.emitKeyValue("objID", objID_);
  json.emitKeyValue("propID", propID_);
#ifdef HERMESVM_API_TRACE_DEBUG
  json.emitKeyValue("propName", propNameDbg_);
#endif
  json.emitKeyValue("value", encode(value_));
}

void SynthTrace::HasPropertyRecord::toJSONInternal(JSONEmitter &json) const {
  Record::toJSONInternal(json);
  json.emitKeyValue("objID", objID_);
  json.emitKeyValue("propID", propID_);
#ifdef HERMESVM_API_TRACE_DEBUG
  json.emitKeyValue("propName", propNameDbg_);
#endif
}

void SynthTrace::GetPropertyNamesRecord::toJSONInternal(
    JSONEmitter &json) const {
  Record::toJSONInternal(json);
  json.emitKeyValue("objID", objID_);
  json.emitKeyValue("propNamesID", propNamesID_);
}

void SynthTrace::CreateArrayRecord::toJSONInternal(JSONEmitter &json) const {
  Record::toJSONInternal(json);
  json.emitKeyValue("objID", objID_);
  json.emitKeyValue("length", length_);
}

void SynthTrace::ArrayReadOrWriteRecord::toJSONInternal(
    JSONEmitter &json) const {
  Record::toJSONInternal(json);
  json.emitKeyValue("objID", objID_);
  json.emitKeyValue("index", index_);
  json.emitKeyValue("value", encode(value_));
}

void SynthTrace::CallRecord::toJSONInternal(JSONEmitter &json) const {
  Record::toJSONInternal(json);
  json.emitKeyValue("functionID", functionID_);
  json.emitKeyValue("thisArg", encode(thisArg_));
  json.emitKey("args");
  json.openArray();
  for (const TraceValue &arg : args_) {
    json.emitValue(encode(arg));
  }
  json.closeArray();
}

void SynthTrace::BeginExecJSRecord::toJSONInternal(
    ::hermes::JSONEmitter &json) const {
  Record::toJSONInternal(json);
  json.emitKeyValue("sourceURL", sourceURL_);
  json.emitKeyValue("sourceHash", ::hermes::hashAsString(sourceHash_));
  json.emitKeyValue("sourceIsBytecode", sourceIsBytecode_);
}

void SynthTrace::ReturnMixin::toJSONInternal(JSONEmitter &json) const {
  json.emitKeyValue("retval", encode(retVal_));
}

void SynthTrace::EndExecJSRecord::toJSONInternal(
    ::hermes::JSONEmitter &json) const {
  MarkerRecord::toJSONInternal(json);
  ReturnMixin::toJSONInternal(json);
}

void SynthTrace::ReturnFromNativeRecord::toJSONInternal(
    ::hermes::JSONEmitter &json) const {
  Record::toJSONInternal(json);
  ReturnMixin::toJSONInternal(json);
}

void SynthTrace::ReturnToNativeRecord::toJSONInternal(
    ::hermes::JSONEmitter &json) const {
  Record::toJSONInternal(json);
  ReturnMixin::toJSONInternal(json);
}

void SynthTrace::GetOrSetPropertyNativeRecord::toJSONInternal(
    JSONEmitter &json) const {
  Record::toJSONInternal(json);
  json.emitKeyValue("hostObjectID", hostObjectID_);
  json.emitKeyValue("propNameID", propNameID_);
  json.emitKeyValue("propName", propName_);
}

void SynthTrace::GetPropertyNativeReturnRecord::toJSONInternal(
    JSONEmitter &json) const {
  Record::toJSONInternal(json);
  ReturnMixin::toJSONInternal(json);
}

void SynthTrace::SetPropertyNativeRecord::toJSONInternal(
    JSONEmitter &json) const {
  GetOrSetPropertyNativeRecord::toJSONInternal(json);
  json.emitKeyValue("value", encode(value_));
}

void SynthTrace::GetNativePropertyNamesRecord::toJSONInternal(
    JSONEmitter &json) const {
  Record::toJSONInternal(json);
  json.emitKeyValue("hostObjectID", hostObjectID_);
}

void SynthTrace::GetNativePropertyNamesReturnRecord::toJSONInternal(
    JSONEmitter &json) const {
  Record::toJSONInternal(json);
  json.emitKey("properties");
  json.openArray();
  for (const auto &prop : propNames_) {
    json.emitValue(prop);
  }
  json.closeArray();
}

const char *SynthTrace::nameFromReleaseUnused(::hermes::vm::ReleaseUnused ru) {
  switch (ru) {
    case ::hermes::vm::ReleaseUnused::kReleaseUnusedNone:
      return "none";
    case ::hermes::vm::ReleaseUnused::kReleaseUnusedOld:
      return "old";
    case ::hermes::vm::ReleaseUnused::kReleaseUnusedYoungOnFull:
      return "youngOnFull";
    case ::hermes::vm::ReleaseUnused::kReleaseUnusedYoungAlways:
      return "youngAlways";
  }
}

::hermes::vm::ReleaseUnused SynthTrace::releaseUnusedFromName(
    const char *rawName) {
  std::string name{rawName};
  if (name == "none") {
    return ::hermes::vm::ReleaseUnused::kReleaseUnusedNone;
  }
  if (name == "old") {
    return ::hermes::vm::ReleaseUnused::kReleaseUnusedOld;
  }
  if (name == "youngOnFull") {
    return ::hermes::vm::ReleaseUnused::kReleaseUnusedYoungOnFull;
  }
  if (name == "youngAlways") {
    return ::hermes::vm::ReleaseUnused::kReleaseUnusedYoungAlways;
  }
  throw std::invalid_argument("Name for RelaseUnused not recognized");
}

void SynthTrace::flushRecordsIfNecessary() {
  if (!json_ || records_.size() < kTraceRecordsToFlush) {
    return;
  }
  flushRecords();
}

void SynthTrace::flushRecords() {
  for (const std::unique_ptr<SynthTrace::Record> &rec : records_) {
    rec->toJSON(*json_);
  }
  records_.clear();
}

void SynthTrace::flushAndDisable(
    const ::hermes::vm::MockedEnvironment &env,
    const ::hermes::vm::GCExecTrace &gcTrace) {
  if (!json_) {
    return;
  }

  // First, flush any buffered records, and close the still-open "trace" array.
  flushRecords();
  json_->closeArray();

  // Env section.
  json_->emitKey("env");
  json_->openDict();
  json_->emitKeyValue("mathRandomSeed", env.mathRandomSeed);

  json_->emitKey("callsToDateNow");
  json_->openArray();
  for (uint64_t dateNow : env.callsToDateNow) {
    json_->emitValue(dateNow);
  }
  json_->closeArray();

  json_->emitKey("callsToNewDate");
  json_->openArray();
  for (uint64_t newDate : env.callsToNewDate) {
    json_->emitValue(newDate);
  }
  json_->closeArray();

  json_->emitKey("callsToDateAsFunction");
  json_->openArray();
  for (const std::string &dateAsFunc : env.callsToDateAsFunction) {
    json_->emitValue(dateAsFunc);
  }
  json_->closeArray();

  json_->emitKey("callsToHermesInternalGetInstrumentedStats");
  json_->openArray();
  for (const ::hermes::vm::MockedEnvironment::StatsTable &call :
       env.callsToHermesInternalGetInstrumentedStats) {
    json_->openDict();
    for (const auto &key : call.keys()) {
      auto val = call.lookup(key);
      if (val.isNum()) {
        json_->emitKeyValue(key, val.num());
      } else {
        json_->emitKeyValue(key, val.str());
      }
    }
    json_->closeDict();
  }
  json_->closeArray();
  json_->closeDict();

  // Now emit the history information, if we're in trace debug mode.
  gcTrace.emit(*json_);

  // Close the top level dictionary (the one opened in the ctor).
  json_->closeDict();

  // Now flush the stream, and reset the fields: further tracing doesn't make
  // sense.
  os().flush();
  json_.reset();
  traceStream_.reset();
}

llvh::raw_ostream &operator<<(
    llvh::raw_ostream &os,
    SynthTrace::RecordType type) {
#define CASE(t)                   \
  case SynthTrace::RecordType::t: \
    return os << #t
  switch (type) {
    CASE(BeginExecJS);
    CASE(EndExecJS);
    CASE(Marker);
    CASE(CreateObject);
    CASE(CreateString);
    CASE(CreatePropNameID);
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
    CASE(GetNativePropertyNames);
    CASE(GetNativePropertyNamesReturn);
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
  CASE(CreateString)
  CASE(CreatePropNameID)
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
  CASE(GetNativePropertyNames)
  CASE(GetNativePropertyNamesReturn)
#undef CASE
  return is;
}

} // namespace tracing
} // namespace hermes
} // namespace facebook

#endif
