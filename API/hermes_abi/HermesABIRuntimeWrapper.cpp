/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes_abi/HermesABIRuntimeWrapper.h"

#include "hermes_abi/hermes_abi.h"

using namespace facebook::jsi;

namespace {

/// Define a helper macro to throw an exception for unimplemented methods. The
/// actual throw is kept in a separate function because throwing generates a lot
/// of code.
[[noreturn]] void throwUnimplementedImpl(const char *name) {
  throw JSINativeException(std::string("Unimplemented function ") + name);
}

#define throwUnimplemented() throwUnimplementedImpl(__func__)

/// An implementation of jsi::Runtime on top of the Hermes C-API.
class HermesABIRuntimeWrapper : public Runtime {
  /// The primary vtable for the C-API implementation that this runtime wraps.
  const HermesABIVTable *abiVtable_;

  /// The vtable for the runtime instance owned by this wrapper. This is cached
  /// here to make it slightly more convenient and efficient to access.
  const HermesABIRuntimeVTable *vtable_;

  /// The runtime object for the Hermes C-API implementation.
  HermesABIRuntime *abiRt_;

 public:
  HermesABIRuntimeWrapper(const HermesABIVTable *vtable) : abiVtable_(vtable) {
    abiRt_ = abiVtable_->make_hermes_runtime(nullptr);
    vtable_ = abiRt_->vt;
  }
  ~HermesABIRuntimeWrapper() override {
    vtable_->release(abiRt_);
  }

  Value evaluateJavaScript(
      const std::shared_ptr<const Buffer> &buffer,
      const std::string &sourceURL) override {
    throwUnimplemented();
  }

  std::shared_ptr<const PreparedJavaScript> prepareJavaScript(
      const std::shared_ptr<const Buffer> &buffer,
      std::string sourceURL) override {
    throwUnimplemented();
  }

  Value evaluatePreparedJavaScript(
      const std::shared_ptr<const PreparedJavaScript> &js) override {
    throwUnimplemented();
  }

  bool drainMicrotasks(int maxMicrotasksHint = -1) override {
    throwUnimplemented();
  }

  Object global() override {
    throwUnimplemented();
  }

  std::string description() override {
    return "HermesABIRuntimeWrapper";
  }

  bool isInspectable() override {
    throwUnimplemented();
  }

  Instrumentation &instrumentation() override {
    throwUnimplemented();
  }

 protected:
  PointerValue *cloneSymbol(const Runtime::PointerValue *pv) override {
    throwUnimplemented();
  }
  PointerValue *cloneBigInt(const Runtime::PointerValue *pv) override {
    throwUnimplemented();
  }
  PointerValue *cloneString(const Runtime::PointerValue *pv) override {
    throwUnimplemented();
  }
  PointerValue *cloneObject(const Runtime::PointerValue *pv) override {
    throwUnimplemented();
  }
  PointerValue *clonePropNameID(const Runtime::PointerValue *pv) override {
    throwUnimplemented();
  }

  PropNameID createPropNameIDFromAscii(const char *str, size_t length)
      override {
    throwUnimplemented();
  }
  PropNameID createPropNameIDFromUtf8(const uint8_t *utf8, size_t length)
      override {
    throwUnimplemented();
  }
  PropNameID createPropNameIDFromString(const String &str) override {
    throwUnimplemented();
  }
  PropNameID createPropNameIDFromSymbol(const Symbol &sym) override {
    throwUnimplemented();
  }
  std::string utf8(const PropNameID &) override {
    throwUnimplemented();
  }
  bool compare(const PropNameID &, const PropNameID &) override {
    throwUnimplemented();
  }

  std::string symbolToString(const Symbol &) override {
    throwUnimplemented();
  }

  BigInt createBigIntFromInt64(int64_t) override {
    throwUnimplemented();
  }
  BigInt createBigIntFromUint64(uint64_t) override {
    throwUnimplemented();
  }
  bool bigintIsInt64(const BigInt &) override {
    throwUnimplemented();
  }
  bool bigintIsUint64(const BigInt &) override {
    throwUnimplemented();
  }
  uint64_t truncate(const BigInt &) override {
    throwUnimplemented();
  }
  String bigintToString(const BigInt &, int) override {
    throwUnimplemented();
  }

  String createStringFromAscii(const char *str, size_t length) override {
    throwUnimplemented();
  }
  String createStringFromUtf8(const uint8_t *utf8, size_t length) override {
    throwUnimplemented();
  }
  std::string utf8(const String &) override {
    throwUnimplemented();
  }

  Object createObject() override {
    throwUnimplemented();
  }
  Object createObject(std::shared_ptr<HostObject> ho) override {
    throwUnimplemented();
  }
  std::shared_ptr<HostObject> getHostObject(const Object &) override {
    throwUnimplemented();
  }
  HostFunctionType &getHostFunction(const Function &) override {
    throwUnimplemented();
  }

  bool hasNativeState(const Object &) override {
    throwUnimplemented();
  }
  std::shared_ptr<NativeState> getNativeState(const Object &) override {
    throwUnimplemented();
  }
  void setNativeState(const Object &, std::shared_ptr<NativeState> state)
      override {
    throwUnimplemented();
  }

  Value getProperty(const Object &, const PropNameID &name) override {
    throwUnimplemented();
  }
  Value getProperty(const Object &, const String &name) override {
    throwUnimplemented();
  }
  bool hasProperty(const Object &, const PropNameID &name) override {
    throwUnimplemented();
  }
  bool hasProperty(const Object &, const String &name) override {
    throwUnimplemented();
  }
  void setPropertyValue(
      const Object &,
      const PropNameID &name,
      const Value &value) override {
    throwUnimplemented();
  }
  void setPropertyValue(const Object &, const String &name, const Value &value)
      override {
    throwUnimplemented();
  }

  bool isArray(const Object &) const override {
    throwUnimplemented();
  }
  bool isArrayBuffer(const Object &) const override {
    throwUnimplemented();
  }
  bool isFunction(const Object &) const override {
    throwUnimplemented();
  }
  bool isHostObject(const Object &) const override {
    throwUnimplemented();
  }
  bool isHostFunction(const Function &) const override {
    throwUnimplemented();
  }
  Array getPropertyNames(const Object &) override {
    throwUnimplemented();
  }

  WeakObject createWeakObject(const Object &) override {
    throwUnimplemented();
  }
  Value lockWeakObject(const WeakObject &) override {
    throwUnimplemented();
  }

  Array createArray(size_t length) override {
    throwUnimplemented();
  }
  ArrayBuffer createArrayBuffer(
      std::shared_ptr<MutableBuffer> buffer) override {
    throwUnimplemented();
  }
  size_t size(const Array &) override {
    throwUnimplemented();
  }
  size_t size(const ArrayBuffer &) override {
    throwUnimplemented();
  }
  uint8_t *data(const ArrayBuffer &) override {
    throwUnimplemented();
  }
  Value getValueAtIndex(const Array &, size_t i) override {
    throwUnimplemented();
  }
  void setValueAtIndexImpl(const Array &, size_t i, const Value &value)
      override {
    throwUnimplemented();
  }

  Function createFunctionFromHostFunction(
      const PropNameID &name,
      unsigned int paramCount,
      HostFunctionType func) override {
    throwUnimplemented();
  }
  Value call(
      const Function &,
      const Value &jsThis,
      const Value *args,
      size_t count) override {
    throwUnimplemented();
  }
  Value callAsConstructor(const Function &, const Value *args, size_t count)
      override {
    throwUnimplemented();
  }

  bool strictEquals(const Symbol &a, const Symbol &b) const override {
    throwUnimplemented();
  }
  bool strictEquals(const BigInt &a, const BigInt &b) const override {
    throwUnimplemented();
  }
  bool strictEquals(const String &a, const String &b) const override {
    throwUnimplemented();
  }
  bool strictEquals(const Object &a, const Object &b) const override {
    throwUnimplemented();
  }

  bool instanceOf(const Object &o, const Function &f) override {
    throwUnimplemented();
  }

  void setExternalMemoryPressure(const Object &obj, size_t amount) override {}
};

} // namespace

namespace facebook {
namespace hermes {
std::unique_ptr<facebook::jsi::Runtime> makeHermesABIRuntimeWrapper(
    const HermesABIVTable *vtable) {
  return std::make_unique<HermesABIRuntimeWrapper>(vtable);
}
} // namespace hermes
} // namespace facebook
