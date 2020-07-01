/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SYNTHTRACE_H
#define HERMES_SYNTHTRACE_H

#ifdef HERMESVM_API_TRACE

#include "hermes/Public/RuntimeConfig.h"
#include "hermes/Support/JSONEmitter.h"
#include "hermes/Support/StringSetVector.h"
#include "hermes/VM/GCExecTrace.h"
#include "hermes/VM/HermesValue.h"
#include "hermes/VM/MockedEnvironment.h"
#include "hermes/VM/Operations.h"
#include "hermes/VM/SymbolID.h"

#include <chrono>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

namespace llvh {
// Forward declaration to avoid including llvm headers.
class raw_ostream;
} // namespace llvh

namespace facebook {
namespace hermes {
namespace tracing {

/// A SynthTrace is a list of events that occur in a run of a JS file by a
/// runtime that uses JSI.
/// It can be serialized into JSON and written to a llvh::raw_ostream.
class SynthTrace {
 public:
  using ObjectID = uint64_t;

  /// A tagged union representing different types available in the trace.
  /// We use a an API very similar to HermesValue, but:
  ///   a) also represent the JSI type PropNameID, and
  ///   b) the "payloads" for some the types (Objects, Strings, and PropNameIDs)
  ///      are unique ObjectIDs, rather than actual values.
  /// (This could probably become a std::variant when we could use C++17.)
  class TraceValue {
   public:
    bool isUndefined() const {
      return tag_ == Tag::Undefined;
    }

    bool isNull() const {
      return tag_ == Tag::Null;
    }

    bool isNumber() const {
      return tag_ == Tag::Number;
    }

    bool isBool() const {
      return tag_ == Tag::Bool;
    }

    bool isObject() const {
      return tag_ == Tag::Object;
    }

    bool isString() const {
      return tag_ == Tag::String;
    }

    bool isPropNameID() const {
      return tag_ == Tag::PropNameID;
    }

    bool isUID() const {
      return isObject() || isString() || isPropNameID();
    }

    static TraceValue encodeUndefinedValue() {
      return TraceValue(Tag::Undefined);
    }

    static TraceValue encodeNullValue() {
      return TraceValue(Tag::Null);
    }

    static TraceValue encodeBoolValue(bool value) {
      return TraceValue(value);
    }

    static TraceValue encodeNumberValue(double value) {
      return TraceValue(value);
    }

    static TraceValue encodeObjectValue(uint64_t uid) {
      return TraceValue(Tag::Object, uid);
    }

    static TraceValue encodeStringValue(uint64_t uid) {
      return TraceValue(Tag::String, uid);
    }

    static TraceValue encodePropNameIDValue(uint64_t uid) {
      return TraceValue(Tag::PropNameID, uid);
    }

    bool operator==(const TraceValue &that) const;

    ObjectID getUID() const {
      assert(isUID());
      return val_.uid;
    }

    bool getBool() const {
      assert(isBool());
      return val_.b;
    }

    double getNumber() const {
      assert(isNumber());
      return val_.n;
    }

   private:
    enum class Tag {
      Undefined,
      Null,
      Bool,
      Number,
      Object,
      String,
      PropNameID
    };

    explicit TraceValue(Tag tag) : tag_(tag) {}
    TraceValue(bool b) : tag_(Tag::Bool) {
      val_.b = b;
    }
    TraceValue(double n) : tag_(Tag::Number) {
      val_.n = n;
    }
    TraceValue(Tag tag, uint64_t uid) : tag_(tag) {
      val_.uid = uid;
    }

    Tag tag_;
    union {
      bool b;
      double n;
      ObjectID uid;
    } val_;
  };

  /// A TimePoint is a time when some event occurred.
  using TimePoint = std::chrono::steady_clock::time_point;
  using TimeSinceStart = std::chrono::milliseconds;

  static constexpr size_t kHashNumBytes = 20;

  /// RecordType is a tag used to differentiate which type of record it is.
  /// There should be a unique tag for each record type.
  enum class RecordType {
    BeginExecJS,
    EndExecJS,
    Marker,
    CreateObject,
    CreateString,
    CreatePropNameID,
    CreateHostObject,
    CreateHostFunction,
    GetProperty,
    SetProperty,
    HasProperty,
    GetPropertyNames,
    CreateArray,
    ArrayRead,
    ArrayWrite,
    CallFromNative,
    ConstructFromNative,
    ReturnFromNative,
    ReturnToNative,
    CallToNative,
    GetPropertyNative,
    GetPropertyNativeReturn,
    SetPropertyNative,
    SetPropertyNativeReturn,
    GetNativePropertyNames,
    GetNativePropertyNamesReturn,
  };

  /// A Record is one element of a trace.
  struct Record {
    /// The time at which this event occurred with respect to the start of
    /// execution.
    /// NOTE: This is not compared in the \c operator= in order for tests to
    /// pass.
    const TimeSinceStart time_;
    explicit Record() = delete;
    explicit Record(TimeSinceStart time) : time_(time) {}
    virtual ~Record() = default;

    /// Write out a serialization of this Record.
    /// \param json An emitter connected to an ostream which will write out
    ///   JSON.
    void toJSON(::hermes::JSONEmitter &json) const;
    virtual RecordType getType() const = 0;

    // If \p val is an object (that is,  an Object or String), push its
    // decoding onto objs.
    static void pushIfTrackedValue(
        const TraceValue &val,
        std::vector<ObjectID> &objs) {
      if (val.isUID()) {
        objs.push_back(val.getUID());
      }
    }

    /// \return A list of object ids that are defined by this record.
    /// Defined means that the record would produce that object,
    /// string, or PropNameID as a locally accessible value if it were
    /// executed.
    virtual std::vector<ObjectID> defs() const {
      return {};
    }

    /// \return A list of object ids that are used by this record.
    /// Used means that the record would use that object, string, or
    /// PropNameID as a value if it were executed.
    /// If a record uses an object id, then some preceding record
    /// (either in the same function invocation, or somewhere
    /// globally) must provide a definition.
    virtual std::vector<ObjectID> uses() const {
      return {};
    }

   protected:
    /// Compare records for equality. Derived classes should override this, call
    /// their parent, and mark any public versions as "final".
    virtual bool operator==(const Record &that) const {
      return getType() == that.getType();
    }

    /// Emit JSON fields into \p os, excluding the closing curly brace.
    /// NOTE: This is overridable, and non-abstract children should call the
    /// parent.
    virtual void toJSONInternal(::hermes::JSONEmitter &json) const;
  };

  /// If \p traceStream is non-null, the trace will be written to that
  /// stream.  Otherwise, no trace is written.
  explicit SynthTrace(
      ObjectID globalObjID,
      const ::hermes::vm::RuntimeConfig &conf,
      std::unique_ptr<llvh::raw_ostream> traceStream = nullptr);

  template <typename T, typename... Args>
  void emplace_back(Args &&... args) {
    records_.emplace_back(new T(std::forward<Args>(args)...));
    flushRecordsIfNecessary();
  }

  const std::vector<std::unique_ptr<Record>> &records() const {
    return records_;
  }

  ObjectID globalObjID() const {
    return globalObjID_;
  }

  /// Given a trace value, turn it into its typed string.
  static std::string encode(TraceValue value);
  /// Encode an undefined JS value for the trace.
  static TraceValue encodeUndefined();
  /// Encode a null JS value for the trace.
  static TraceValue encodeNull();
  /// Encode a boolean JS value for the trace.
  static TraceValue encodeBool(bool value);
  /// Encodes a numeric value for the trace.
  static TraceValue encodeNumber(double value);
  /// Encodes an object for the trace as a unique id.
  static TraceValue encodeObject(ObjectID objID);
  /// Encodes a string for the trace as a unique id.
  static TraceValue encodeString(ObjectID objID);
  /// Encodes a PropNameID for the trace as a unique id.
  static TraceValue encodePropNameID(ObjectID objID);

  /// Decodes a string into a trace value.
  static TraceValue decode(const std::string &);

  /// The version of the Synth Benchmark
  constexpr static uint32_t synthVersion() {
    return 3;
  }

  static const char *nameFromReleaseUnused(::hermes::vm::ReleaseUnused ru);
  static ::hermes::vm::ReleaseUnused releaseUnusedFromName(const char *name);

 private:
  llvh::raw_ostream &os() const {
    return (*traceStream_);
  }

  /// If we're tracing to a file, and the number of accumulated
  /// records has reached the limit kTraceRecordsToFlush, below,
  /// flush the records to the file, and reset the accumulated records
  /// to be empty.
  void flushRecordsIfNecessary();

  /// Assumes we're tracing to a file; flush accumulated records to
  /// the file, and reset the accumulated records to be empty.
  void flushRecords();

  static constexpr unsigned kTraceRecordsToFlush = 100;

  /// If we're tracing to a file, pointer to a stream onto
  /// traceFilename_.  Null otherwise.
  std::unique_ptr<llvh::raw_ostream> traceStream_;
  /// If we're tracing to a file, pointer to a JSONEmitter writting
  /// into *traceStream_.  Null otherwise.
  std::unique_ptr<::hermes::JSONEmitter> json_;
  /// The records currently being accumulated in the trace.  If we are
  /// tracing to a file, these will be only the records not yet
  /// written to the file.
  std::vector<std::unique_ptr<Record>> records_;
  /// The id of the global object.
  const ObjectID globalObjID_;

 public:
  /// @name Record classes
  /// @{

  /// A MarkerRecord is an event that simply records an interesting event that
  /// is not necessarily meaningful to the interpreter. It comes with a tag that
  /// says what type of marker it was.
  struct MarkerRecord : public Record {
    static constexpr RecordType type{RecordType::Marker};
    const std::string tag_;
    explicit MarkerRecord(TimeSinceStart time, const std::string &tag)
        : Record(time), tag_(tag) {}
    RecordType getType() const override {
      return type;
    }

   protected:
    void toJSONInternal(::hermes::JSONEmitter &json) const override;
    bool operator==(const Record &that) const override;
  };

  /// A BeginExecJSRecord is an event where execution begins of JS source
  /// code. This is not necessarily the first record, since native code can
  /// inject values into the VM before any source code is run.
  struct BeginExecJSRecord final : public Record {
    static constexpr RecordType type{RecordType::BeginExecJS};
    explicit BeginExecJSRecord(
        TimeSinceStart time,
        std::string sourceURL,
        ::hermes::SHA1 sourceHash,
        bool sourceIsBytecode)
        : Record(time),
          sourceURL_(std::move(sourceURL)),
          sourceHash_(std::move(sourceHash)),
          sourceIsBytecode_(sourceIsBytecode) {}

    RecordType getType() const override {
      return type;
    }

    const std::string &sourceURL() const {
      return sourceURL_;
    }

    const ::hermes::SHA1 &sourceHash() const {
      return sourceHash_;
    }

    bool operator==(const Record &that) const override;

   private:
    void toJSONInternal(::hermes::JSONEmitter &json) const override;

    /// The URL providing the source file mapping for the file being executed.
    /// Can be empty.
    std::string sourceURL_;

    /// A hash of the source that was executed. The source hash must match up
    /// when the file is replayed.
    /// The hash is optional, and will be all zeros if not provided.
    ::hermes::SHA1 sourceHash_;

    /// Whether the input file was source or bytecode.
    bool sourceIsBytecode_;
  };

  struct ReturnMixin {
    const TraceValue retVal_;

    explicit ReturnMixin(TraceValue value) : retVal_(value) {}

    void toJSONInternal(::hermes::JSONEmitter &json) const;
    bool operator==(const ReturnMixin &that) const;
  };

  /// A EndExecJSRecord is an event where execution of JS source code stops.
  /// This does not mean that the source code will never be entered again, just
  /// that it has an entered a phase where it is waiting for native code to call
  /// into the JS. This event is not guaranteed to be the last event, for the
  /// aforementioned reason. The logged retVal is the result of the evaluation
  /// ("undefined" in the majority of cases).
  struct EndExecJSRecord final : public MarkerRecord, public ReturnMixin {
    static constexpr RecordType type{RecordType::EndExecJS};
    EndExecJSRecord(TimeSinceStart time, TraceValue retVal)
        : MarkerRecord(time, "end_global_code"), ReturnMixin(retVal) {}

    RecordType getType() const override {
      return type;
    }
    bool operator==(const Record &that) const final;
    virtual void toJSONInternal(::hermes::JSONEmitter &json) const final;
    std::vector<ObjectID> defs() const override {
      auto defs = MarkerRecord::defs();
      pushIfTrackedValue(retVal_, defs);
      return defs;
    }
  };

  /// A CreateObjectRecord is an event where an empty object is created by the
  /// native code.
  struct CreateObjectRecord : public Record {
    static constexpr RecordType type{RecordType::CreateObject};
    const ObjectID objID_;

    explicit CreateObjectRecord(TimeSinceStart time, ObjectID objID)
        : Record(time), objID_(objID) {}

    bool operator==(const Record &that) const override;

    void toJSONInternal(::hermes::JSONEmitter &json) const override;
    RecordType getType() const override {
      return type;
    }

    std::vector<ObjectID> defs() const override {
      return {objID_};
    }

    std::vector<ObjectID> uses() const override {
      return {};
    }
  };

  /// A CreateStringRecord is an event where a jsi::String (and thus a
  /// Hermes StringPrimitive) is created by the native code.
  struct CreateStringRecord : public Record {
    static constexpr RecordType type{RecordType::CreateString};
    const ObjectID objID_;
    std::string chars_;
    bool ascii_;

    // General UTF-8.
    CreateStringRecord(
        TimeSinceStart time,
        ObjectID objID,
        const uint8_t *chars,
        size_t length)
        : Record(time),
          objID_(objID),
          chars_(reinterpret_cast<const char *>(chars), length),
          ascii_(false) {}
    // Ascii.
    CreateStringRecord(
        TimeSinceStart time,
        ObjectID objID,
        const char *chars,
        size_t length)
        : Record(time), objID_(objID), chars_(chars, length), ascii_(true) {}

    bool operator==(const Record &that) const override;

    void toJSONInternal(::hermes::JSONEmitter &json) const override;
    RecordType getType() const override {
      return type;
    }

    std::vector<ObjectID> defs() const override {
      return {objID_};
    }

    std::vector<ObjectID> uses() const override {
      return {};
    }
  };

  /// A CreatePropNameIDRecord is an event where a jsi::PropNameID is
  /// created by the native code.
  struct CreatePropNameIDRecord : public Record {
    static constexpr RecordType type{RecordType::CreatePropNameID};
    const ObjectID propNameID_;
    std::string chars_;
    bool ascii_;

    // General UTF-8.
    CreatePropNameIDRecord(
        TimeSinceStart time,
        ObjectID propNameID,
        const uint8_t *chars,
        size_t length)
        : Record(time),
          propNameID_(propNameID),
          chars_(reinterpret_cast<const char *>(chars), length),
          ascii_(false) {}
    // Ascii.
    CreatePropNameIDRecord(
        TimeSinceStart time,
        ObjectID propNameID,
        const char *chars,
        size_t length)
        : Record(time),
          propNameID_(propNameID),
          chars_(chars, length),
          ascii_(true) {}
    // std::string
    CreatePropNameIDRecord(
        TimeSinceStart time,
        ObjectID propNameID,
        std::string &&chars,
        bool isAscii)
        : Record(time),
          propNameID_(propNameID),
          chars_(std::move(chars)),
          ascii_(isAscii) {}

    bool operator==(const Record &that) const override;

    void toJSONInternal(::hermes::JSONEmitter &json) const override;
    RecordType getType() const override {
      return type;
    }

    std::vector<ObjectID> defs() const override {
      return {propNameID_};
    }

    std::vector<ObjectID> uses() const override {
      return {};
    }
  };

  struct CreateHostObjectRecord final : public CreateObjectRecord {
    static constexpr RecordType type{RecordType::CreateHostObject};
    using CreateObjectRecord::CreateObjectRecord;
    RecordType getType() const override {
      return type;
    }
  };

  struct CreateHostFunctionRecord final : public CreateObjectRecord {
    static constexpr RecordType type{RecordType::CreateHostFunction};
    uint32_t propNameID_;
#ifdef HERMESVM_API_TRACE_DEBUG
    const std::string functionName_;
#endif
    const unsigned paramCount_;

    CreateHostFunctionRecord(
        TimeSinceStart time,
        ObjectID objID,
        ObjectID propNameID,
#ifdef HERMESVM_API_TRACE_DEBUG
        std::string functionName,
#endif
        unsigned paramCount)
        : CreateObjectRecord(time, objID),
          propNameID_(propNameID),
#ifdef HERMESVM_API_TRACE_DEBUG
          functionName_(std::move(functionName)),
#endif
          paramCount_(paramCount) {
    }

    bool operator==(const Record &that) const override;

    void toJSONInternal(::hermes::JSONEmitter &json) const override;

    RecordType getType() const override {
      return type;
    }

    std::vector<ObjectID> uses() const override {
      return {propNameID_};
    }
  };

  struct GetOrSetPropertyRecord : public Record {
    const ObjectID objID_;
    // This may be the ID of a String or a PropNameID.
    const ObjectID propID_{0};
#ifdef HERMESVM_API_TRACE_DEBUG
    std::string propNameDbg_;
#endif
    const TraceValue value_;

    GetOrSetPropertyRecord(
        TimeSinceStart time,
        ObjectID objID,
        ObjectID propID,
#ifdef HERMESVM_API_TRACE_DEBUG
        const std::string &propNameDbg,
#endif
        TraceValue value)
        : Record(time),
          objID_(objID),
          propID_(propID),
#ifdef HERMESVM_API_TRACE_DEBUG
          propNameDbg_(propNameDbg),
#endif
          value_(value) {
    }

    bool operator==(const Record &that) const final;

    std::vector<ObjectID> uses() const override {
      return {objID_, propID_};
    }

    void toJSONInternal(::hermes::JSONEmitter &json) const override;
  };

  /// A GetPropertyRecord is an event where native code accesses the property
  /// of a JS object.
  struct GetPropertyRecord : public GetOrSetPropertyRecord {
    static constexpr RecordType type{RecordType::GetProperty};
    using GetOrSetPropertyRecord::GetOrSetPropertyRecord;
    RecordType getType() const override {
      return type;
    }
    std::vector<ObjectID> defs() const override {
      auto defs = GetOrSetPropertyRecord::defs();
      pushIfTrackedValue(value_, defs);
      return defs;
    }
  };

  /// A SetPropertyRecord is an event where native code writes to the property
  /// of a JS object.
  struct SetPropertyRecord : public GetOrSetPropertyRecord {
    static constexpr RecordType type{RecordType::SetProperty};
    using GetOrSetPropertyRecord::GetOrSetPropertyRecord;
    RecordType getType() const override {
      return type;
    }
    std::vector<ObjectID> uses() const override {
      auto uses = GetOrSetPropertyRecord::uses();
      pushIfTrackedValue(value_, uses);
      return uses;
    }
  };

  /// A HasPropertyRecord is an event where native code queries whether a
  /// property exists on an object.  (We don't care about the result because
  /// it cannot influence the trace.)
  struct HasPropertyRecord final : public Record {
    static constexpr RecordType type{RecordType::HasProperty};
    const ObjectID objID_;
#ifdef HERMESVM_API_TRACE_DEBUG
    std::string propNameDbg_;
#endif
    // This may be the ID of a String or a PropNameID.
    const ObjectID propID_{0};

    HasPropertyRecord(
        TimeSinceStart time,
        ObjectID objID,
        ObjectID propID
#ifdef HERMESVM_API_TRACE_DEBUG
        ,
        const std::string &propNameDbg
#endif
        )
        : Record(time),
          objID_(objID),
#ifdef HERMESVM_API_TRACE_DEBUG
          propNameDbg_(propNameDbg),
#endif
          propID_(propID) {
    }

    bool operator==(const Record &that) const final;

    void toJSONInternal(::hermes::JSONEmitter &json) const override;
    RecordType getType() const override {
      return type;
    }
    std::vector<ObjectID> uses() const override {
      return {objID_, propID_};
    }
  };

  struct GetPropertyNamesRecord final : public Record {
    static constexpr RecordType type{RecordType::GetPropertyNames};
    const ObjectID objID_;
    // Since getPropertyNames always returns an array, this can be an object id
    // rather than a TraceValue.
    const ObjectID propNamesID_;

    explicit GetPropertyNamesRecord(
        TimeSinceStart time,
        ObjectID objID,
        ObjectID propNamesID)
        : Record(time), objID_(objID), propNamesID_(propNamesID) {}

    bool operator==(const Record &that) const final;

    void toJSONInternal(::hermes::JSONEmitter &json) const override;
    RecordType getType() const override {
      return type;
    }
    std::vector<ObjectID> defs() const override {
      return {propNamesID_};
    }
    std::vector<ObjectID> uses() const override {
      return {objID_};
    }
  };

  /// A CreateArrayRecord is an event where a new array is created of a specific
  /// length.
  struct CreateArrayRecord final : public Record {
    static constexpr RecordType type{RecordType::CreateArray};
    const ObjectID objID_;
    const size_t length_;

    explicit CreateArrayRecord(
        TimeSinceStart time,
        ObjectID objID,
        size_t length)
        : Record(time), objID_(objID), length_(length) {}

    bool operator==(const Record &that) const final;

    void toJSONInternal(::hermes::JSONEmitter &json) const override;
    RecordType getType() const override {
      return type;
    }
    std::vector<ObjectID> defs() const override {
      return {objID_};
    }
  };

  struct ArrayReadOrWriteRecord : public Record {
    const ObjectID objID_;
    const size_t index_;
    const TraceValue value_;

    explicit ArrayReadOrWriteRecord(
        TimeSinceStart time,
        ObjectID objID,
        size_t index,
        TraceValue value)
        : Record(time), objID_(objID), index_(index), value_(value) {}

    bool operator==(const Record &that) const final;

    void toJSONInternal(::hermes::JSONEmitter &json) const override;
    std::vector<ObjectID> uses() const override {
      return {objID_};
    }
  };

  /// An ArrayReadRecord is an event where a value was read from an index
  /// of an array.
  /// It is modeled separately from GetProperty because it is more efficient to
  /// read from a numeric index on an array than a string.
  struct ArrayReadRecord final : public ArrayReadOrWriteRecord {
    static constexpr RecordType type{RecordType::ArrayRead};
    using ArrayReadOrWriteRecord::ArrayReadOrWriteRecord;
    RecordType getType() const override {
      return type;
    }
    std::vector<ObjectID> defs() const override {
      auto defs = ArrayReadOrWriteRecord::defs();
      pushIfTrackedValue(value_, defs);
      return defs;
    }
  };

  /// An ArrayWriteRecord is an event where a value was written into an index
  /// of an array.
  struct ArrayWriteRecord final : public ArrayReadOrWriteRecord {
    static constexpr RecordType type{RecordType::ArrayWrite};
    using ArrayReadOrWriteRecord::ArrayReadOrWriteRecord;
    RecordType getType() const override {
      return type;
    }
    std::vector<ObjectID> uses() const override {
      auto uses = ArrayReadOrWriteRecord::uses();
      pushIfTrackedValue(value_, uses);
      return uses;
    }
  };

  struct CallRecord : public Record {
    /// The functionID_ is the id of the function JS object that is called from
    /// JS.
    const ObjectID functionID_;
    const TraceValue thisArg_;
    /// The arguments given to a call (excluding the this parameter),
    /// already JSON stringified.
    const std::vector<TraceValue> args_;

    explicit CallRecord(
        TimeSinceStart time,
        ObjectID functionID,
        TraceValue thisArg,
        const std::vector<TraceValue> &args)
        : Record(time),
          functionID_(functionID),
          thisArg_(thisArg),
          args_(args) {}

    bool operator==(const Record &that) const final;

    void toJSONInternal(::hermes::JSONEmitter &json) const override;
    std::vector<ObjectID> uses() const override {
      // The function is used regardless of direction.
      return {functionID_};
    }

   protected:
    std::vector<ObjectID> getArgTrackedIDs() const {
      std::vector<ObjectID> objs;
      pushIfTrackedValue(thisArg_, objs);
      for (const auto &arg : args_) {
        pushIfTrackedValue(arg, objs);
      }
      return objs;
    }
  };

  /// A CallFromNativeRecord is an event where native code calls into a JS
  /// function.
  struct CallFromNativeRecord : public CallRecord {
    static constexpr RecordType type{RecordType::CallFromNative};
    using CallRecord::CallRecord;
    RecordType getType() const override {
      return type;
    }
    std::vector<ObjectID> uses() const override {
      auto uses = CallRecord::uses();
      auto objs = CallRecord::getArgTrackedIDs();
      uses.insert(uses.end(), objs.begin(), objs.end());
      return uses;
    }
  };

  /// A ConstructFromNativeRecord is the same as \c CallFromNativeRecord, except
  /// the function is called with the new operator.
  struct ConstructFromNativeRecord final : public CallFromNativeRecord {
    static constexpr RecordType type{RecordType::ConstructFromNative};
    using CallFromNativeRecord::CallFromNativeRecord;
    RecordType getType() const override {
      return type;
    }
  };

  /// A ReturnFromNativeRecord is an event where a native function returns to a
  /// JS caller.
  /// It pairs with \c CallToNativeRecord.
  struct ReturnFromNativeRecord final : public Record, public ReturnMixin {
    static constexpr RecordType type{RecordType::ReturnFromNative};
    ReturnFromNativeRecord(TimeSinceStart time, TraceValue retVal)
        : Record(time), ReturnMixin(retVal) {}
    RecordType getType() const override {
      return type;
    }
    std::vector<ObjectID> uses() const override {
      auto uses = Record::uses();
      pushIfTrackedValue(retVal_, uses);
      return uses;
    }
    bool operator==(const Record &that) const final;
    void toJSONInternal(::hermes::JSONEmitter &json) const override;
  };

  /// A ReturnToNativeRecord is an event where a JS function returns to a native
  /// caller.
  /// It pairs with \c CallFromNativeRecord.
  struct ReturnToNativeRecord final : public Record, public ReturnMixin {
    static constexpr RecordType type{RecordType::ReturnToNative};
    ReturnToNativeRecord(TimeSinceStart time, TraceValue retVal)
        : Record(time), ReturnMixin(retVal) {}
    RecordType getType() const override {
      return type;
    }
    std::vector<ObjectID> defs() const override {
      auto defs = Record::defs();
      pushIfTrackedValue(retVal_, defs);
      return defs;
    }
    bool operator==(const Record &that) const final;
    void toJSONInternal(::hermes::JSONEmitter &json) const override;
  };

  /// A CallToNativeRecord is an event where JS code calls into a natively
  /// defined function.
  struct CallToNativeRecord final : public CallRecord {
    static constexpr RecordType type{RecordType::CallToNative};
    using CallRecord::CallRecord;
    RecordType getType() const override {
      return type;
    }
    std::vector<ObjectID> defs() const override {
      auto defs = CallRecord::defs();
      auto objs = CallRecord::getArgTrackedIDs();
      defs.insert(defs.end(), objs.begin(), objs.end());
      return defs;
    }
  };

  struct GetOrSetPropertyNativeRecord : public Record {
    const ObjectID hostObjectID_;
    const ObjectID propNameID_;
    const std::string propName_;

    GetOrSetPropertyNativeRecord(
        TimeSinceStart time,
        ObjectID hostObjectID,
        ObjectID propNameID,
        const std::string &propName)
        : Record(time),
          hostObjectID_(hostObjectID),
          propNameID_(propNameID),
          propName_(propName) {}

    void toJSONInternal(::hermes::JSONEmitter &json) const override;
    std::vector<ObjectID> defs() const override {
      return {propNameID_};
    }
    std::vector<ObjectID> uses() const override {
      return {hostObjectID_};
    }

   protected:
    bool operator==(const Record &that) const override;
  };

  /// A GetPropertyNativeRecord is an event where JS tries to access a property
  /// on a native object.
  /// This needs to be modeled as a call with no arguments, since native code
  /// can arbitrarily affect the JS heap during the accessor.
  struct GetPropertyNativeRecord final : public GetOrSetPropertyNativeRecord {
    static constexpr RecordType type{RecordType::GetPropertyNative};
    using GetOrSetPropertyNativeRecord::GetOrSetPropertyNativeRecord;
    RecordType getType() const override {
      return type;
    }
    bool operator==(const Record &that) const final;
  };

  struct GetPropertyNativeReturnRecord final : public Record,
                                               public ReturnMixin {
    static constexpr RecordType type{RecordType::GetPropertyNativeReturn};
    GetPropertyNativeReturnRecord(TimeSinceStart time, TraceValue retVal)
        : Record(time), ReturnMixin(retVal) {}
    RecordType getType() const override {
      return type;
    }
    std::vector<ObjectID> uses() const override {
      auto uses = Record::uses();
      pushIfTrackedValue(retVal_, uses);
      return uses;
    }
    bool operator==(const Record &that) const final;

   protected:
    void toJSONInternal(::hermes::JSONEmitter &json) const override;
  };

  /// A SetPropertyNativeRecord is an event where JS code writes to the property
  /// of a Native object.
  /// This needs to be modeled as a call with one argument, since native code
  /// can arbitrarily affect the JS heap during the accessor.
  struct SetPropertyNativeRecord final : public GetOrSetPropertyNativeRecord {
    static constexpr RecordType type{RecordType::SetPropertyNative};
    TraceValue value_;

    SetPropertyNativeRecord(
        TimeSinceStart time,
        ObjectID hostObjectID,
        ObjectID propNameID,
        const std::string &propName,
        TraceValue value)
        : GetOrSetPropertyNativeRecord(
              time,
              hostObjectID,
              propNameID,
              propName),
          value_(value) {}

    bool operator==(const Record &that) const final;
    void toJSONInternal(::hermes::JSONEmitter &json) const override;
    RecordType getType() const override {
      return type;
    }
    std::vector<ObjectID> defs() const override {
      auto defs = GetOrSetPropertyNativeRecord::defs();
      pushIfTrackedValue(value_, defs);
      return defs;
    }
  };

  /// A SetPropertyNativeReturnRecord needs to record no extra information
  struct SetPropertyNativeReturnRecord final : public Record {
    static constexpr RecordType type{RecordType::SetPropertyNativeReturn};
    using Record::Record;
    RecordType getType() const override {
      return type;
    }
    bool operator==(const Record &that) const final {
      // Since there are no fields to compare, any two will always be the same.
      return Record::operator==(that);
    }
  };

  /// A GetNativePropertyNamesRecord records an event where JS asked for a list
  /// of property names available on a host object. It records the object, and
  /// the returned list of property names.
  struct GetNativePropertyNamesRecord : public Record {
    static constexpr RecordType type{RecordType::GetNativePropertyNames};
    const ObjectID hostObjectID_;

    explicit GetNativePropertyNamesRecord(
        TimeSinceStart time,
        ObjectID hostObjectID)
        : Record(time), hostObjectID_(hostObjectID) {}

    RecordType getType() const override {
      return type;
    }

    void toJSONInternal(::hermes::JSONEmitter &json) const override;

    std::vector<ObjectID> uses() const override {
      return {hostObjectID_};
    }

    bool operator==(const Record &that) const override;
  };

  /// A GetNativePropertyNamesReturnRecord records what property names were
  /// returned by the GetNativePropertyNames query.
  struct GetNativePropertyNamesReturnRecord final : public Record {
    static constexpr RecordType type{RecordType::GetNativePropertyNamesReturn};
    const std::vector<std::string> propNames_;

    explicit GetNativePropertyNamesReturnRecord(
        TimeSinceStart time,
        const std::vector<std::string> &propNames)
        : Record(time), propNames_(propNames) {}

    RecordType getType() const override {
      return type;
    }

    void toJSONInternal(::hermes::JSONEmitter &json) const override;

    bool operator==(const Record &that) const override;
  };

  /// Completes writing of the trace to the trace stream.  If writing
  /// to a file, disables further writing to the file, or accumulation
  /// of data.
  void flushAndDisable(
      const ::hermes::vm::MockedEnvironment &env,
      const ::hermes::vm::GCExecTrace &gcTrace);
};

llvh::raw_ostream &operator<<(
    llvh::raw_ostream &os,
    SynthTrace::RecordType type);
std::istream &operator>>(std::istream &is, SynthTrace::RecordType &type);

} // namespace tracing
} // namespace hermes
} // namespace facebook

#endif // HERMESVM_API_TRACE

#endif // HERMES_SYNTHTRACE_H
