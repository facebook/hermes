/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes_abi/HermesABIRuntime.h"

#include "hermes_abi/hermes_abi.h"

using namespace facebook::jsi;

namespace {

[[noreturn]] void throwJSINativeException(std::string err) {
  throw JSINativeException(err);
}
#define throwUnimplemented() \
  throwJSINativeException(std::string("Unimplemented function ") + __func__)

class HermesABIRuntime : public Runtime {
  HermesABIContext *ctx_;
  const HermesABIVTable *vtable_;

 public:
  HermesABIRuntime(const ::hermes::vm::RuntimeConfig &runtimeConfig) {
    vtable_ = get_hermes_abi_vtable();
    ctx_ = vtable_->make_hermes_runtime();
  }
  ~HermesABIRuntime() override {
    vtable_->release_hermes_runtime(ctx_);
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
    throwUnimplemented();
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
};

} // namespace

namespace facebook::hermes {
std::unique_ptr<facebook::jsi::Runtime> makeHermesABIRuntime(
    const ::hermes::vm::RuntimeConfig &runtimeConfig) {
  return std::make_unique<HermesABIRuntime>(runtimeConfig);
}
} // namespace facebook::hermes
