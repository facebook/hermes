/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes_abi/HermesABIRuntimeWrapper.h"

#include "hermes_abi/HermesABIHelpers.h"
#include "hermes_abi/hermes_abi.h"

#include "hermes/ADT/ManagedChunkedList.h"

#include <atomic>

#if __has_builtin(__builtin_unreachable)
#define BUILTIN_UNREACHABLE __builtin_unreachable()
#elif defined(_MSC_VER)
#define BUILTIN_UNREACHABLE __assume(false)
#else
#define BUILTIN_UNREACHABLE assert(false);
#endif

using namespace facebook::jsi;
using namespace facebook::hermes;

namespace {

/// Define a helper macro to throw an exception for unimplemented methods. The
/// actual throw is kept in a separate function because throwing generates a lot
/// of code.
[[noreturn]] void throwUnimplementedImpl(const char *name) {
  throw JSINativeException(std::string("Unimplemented function ") + name);
}

#define THROW_UNIMPLEMENTED() throwUnimplementedImpl(__func__)

/// An implementation of HermesABIGrowableBuffer that uses a string as its
/// internal storage. This can be used to conveniently construct a std::string
/// from ABI functions that return strings.
class StringByteBuffer : public HermesABIGrowableBuffer {
  std::string buf_;

  static void grow_to(HermesABIGrowableBuffer *buf, size_t sz) {
    auto *self = static_cast<StringByteBuffer *>(buf);
    // The API specifies that providing a smaller size is a no-op.
    if (sz < self->size)
      return;
    self->buf_.resize(sz);
    self->data = (uint8_t *)self->buf_.data();
    self->size = sz;
  }

  static constexpr HermesABIGrowableBufferVTable vt{
      grow_to,
  };

 public:
  explicit StringByteBuffer() : HermesABIGrowableBuffer{&vt, nullptr, 0, 0} {
    // Make the small string storage available for use without needing to call
    // grow_by.
    buf_.resize(buf_.capacity());
    data = (uint8_t *)buf_.data();
    size = buf_.size();
  }

  std::string get() && {
    // Trim off any unused bytes at the end.
    buf_.resize(used);
    return std::move(buf_);
  }
};

/// Helper class to save and restore a value on exiting a scope.
template <typename T>
class SaveAndRestore {
  T &target_;
  T oldVal_;

 public:
  SaveAndRestore(T &target) : target_(target), oldVal_(target) {}
  ~SaveAndRestore() {
    target_ = oldVal_;
  }
};

/// An implementation of jsi::Runtime on top of the Hermes C-API.
class HermesABIRuntimeWrapper : public Runtime {
  class ManagedPointerHolder;

  /// The primary vtable for the C-API implementation that this runtime wraps.
  const HermesABIVTable *abiVtable_;

  /// The vtable for the runtime instance owned by this wrapper. This is cached
  /// here to make it slightly more convenient and efficient to access.
  const HermesABIRuntimeVTable *vtable_;

  /// The runtime object for the Hermes C-API implementation.
  HermesABIRuntime *abiRt_;

  /// The list of pointers currently retained through JSI. This manages the
  /// lifetime of the underlying ABI pointers so that they are released when the
  /// corresponding JSI pointer is released.
  hermes::ManagedChunkedList<ManagedPointerHolder> managedPointers_;

  /// Whether we are currently processing a JSError. This is used to detect
  /// recursive invocations of the JSError constructor and prevent them from
  /// causing a stack overflow.
  bool activeJSError_ = false;

  /// A ManagedChunkedList element that indicates whether it's occupied based on
  /// a refcount.
  /// TODO: Replace jsi::PointerValue with something like
  ///       HermesABIManagedPointer, so we can directly invalidate values.
  class ManagedPointerHolder : public PointerValue {
    std::atomic<uint32_t> refCount_;
    union {
      HermesABIManagedPointer *managedPointer_;
      ManagedPointerHolder *nextFree_;
    };

   public:
    ManagedPointerHolder() : refCount_(0) {}

    /// Determine whether the element is occupied by inspecting the refcount.
    bool isFree() const {
      return refCount_.load(std::memory_order_relaxed) == 0;
    }

    /// Store a value and start the refcount at 1. After invocation, this
    /// instance is occupied with a value, and the "nextFree" methods should
    /// not be used until the value is released.
    void emplace(HermesABIManagedPointer *managedPointer) {
      assert(isFree() && "Emplacing already occupied value");
      refCount_.store(1, std::memory_order_relaxed);
      managedPointer_ = managedPointer;
    }

    /// Get the next free element. Must not be called when this instance is
    /// occupied with a value.
    ManagedPointerHolder *getNextFree() {
      assert(isFree() && "Free pointer unusable while occupied");
      return nextFree_;
    }

    /// Set the next free element. Must not be called when this instance is
    /// occupied with a value.
    void setNextFree(ManagedPointerHolder *nextFree) {
      assert(isFree() && "Free pointer unusable while occupied");
      nextFree_ = nextFree;
    }

    HermesABIManagedPointer *getManagedPointer() const {
      assert(!isFree() && "Value not present");
      return managedPointer_;
    }

    void invalidate() override {
      dec();
    }

    void inc() {
      // See comments in hermes_abi.cpp for why we use relaxed operations here.
      auto oldCount = refCount_.fetch_add(1, std::memory_order_relaxed);
      assert(oldCount && "Cannot resurrect a pointer");
      assert(oldCount + 1 != 0 && "Ref count overflow");
      (void)oldCount;
    }

    void dec() {
      // See comments in hermes_abi.cpp for why we use relaxed operations here.
      auto oldCount = refCount_.fetch_sub(1, std::memory_order_relaxed);
      assert(oldCount > 0 && "Ref count underflow");
      // This was the last decrement of this holder, so we can invalidate the
      // underlying pointer.
      if (oldCount == 1)
        abi::releasePointer(managedPointer_);
    }
  };

  /// Convert the error code returned by the Hermes C-API into a C++ exception.
  [[noreturn]] void throwError(HermesABIErrorCode err) {
    if (err == HermesABIErrorCodeJSError) {
      // We have to get and clear the error regardless of whether it is used.
      auto errVal = intoJSIValue(vtable_->get_and_clear_js_error_value(abiRt_));

      // If we are already in the process of creating a JSError, it means that
      // something in JSError's constructor is throwing. We cannot handle this
      // gracefully, so bail.
      if (activeJSError_)
        throw JSINativeException("Error thrown while handling error.");

      // Record the fact that we are in the process of creating a JSError.
      SaveAndRestore<bool> s(activeJSError_);
      activeJSError_ = true;
      throw JSError(*this, std::move(errVal));
    } else if (err == HermesABIErrorCodeNativeException) {
      StringByteBuffer buf;
      vtable_->get_and_clear_native_exception_message(abiRt_, &buf);
      throw JSINativeException(std::move(buf).get());
    }

    throw JSINativeException("ABI threw an unknown error.");
  }

  /// Define some helper functions for each pointer type.
  /// 1. intoJSI*Pointer*: Take ownership of the given ABI pointer and produce a
  /// JSI Pointer that will now manage its lifetime. If the operand may contain
  /// an error, check for and convert the exception.
  /// 2. toABI*Pointer*: Create an ABI pointer that aliases the given JSI
  /// pointer. The ABI pointer will be invalidated once the JSI pointer is
  /// released.
  /// 3. unwrap: Unwrap the given ABI pointer, checking for errors.

#define DECLARE_POINTER_CONVERSIONS(name)                             \
  name intoJSI##name(const HermesABI##name &p) {                      \
    return make<name>(&managedPointers_.add(p.pointer));              \
  }                                                                   \
  name intoJSI##name(const HermesABI##name##OrError &p) {             \
    return intoJSI##name(unwrap(p));                                  \
  }                                                                   \
  HermesABI##name toABI##name(const name &p) const {                  \
    return abi::create##name(                                         \
        static_cast<const ManagedPointerHolder *>(getPointerValue(p)) \
            ->getManagedPointer());                                   \
  }                                                                   \
  HermesABI##name unwrap(const HermesABI##name##OrError &p) {         \
    if (abi::isError(p))                                              \
      throwError(abi::getError(p));                                   \
    return abi::get##name(p);                                         \
  }

  HERMES_ABI_POINTER_TYPES(DECLARE_POINTER_CONVERSIONS)
#undef DECLARE_POINTER_CONVERSIONS

  /// Define unwrap functions for each primitive type. These check their operand
  /// for errors and return the contained primitive if they do not.
  void unwrap(const HermesABIVoidOrError &v) {
    if (abi::isError(v))
      throwError(abi::getError(v));
  }
  bool unwrap(const HermesABIBoolOrError &p) {
    if (abi::isError(p))
      throwError(abi::getError(p));
    return abi::getBool(p);
  }
  uint8_t *unwrap(const HermesABIUint8PtrOrError &p) {
    if (abi::isError(p))
      throwError(abi::getError(p));
    return abi::getUint8Ptr(p);
  }
  size_t unwrap(const HermesABISizeTOrError &p) {
    if (abi::isError(p))
      throwError(abi::getError(p));
    return abi::getSizeT(p);
  }

  /// Take ownership of the given value \p v and wrap it in a jsi::Value that
  /// will now manage its lifetime.
  Value intoJSIValue(const HermesABIValue &v) {
    switch (abi::getValueKind(v)) {
      case HermesABIValueKindUndefined:
        return Value::undefined();
      case HermesABIValueKindNull:
        return Value::null();
      case HermesABIValueKindBoolean:
        return Value(abi::getBoolValue(v));
      case HermesABIValueKindNumber:
        return Value(abi::getNumberValue(v));
      case HermesABIValueKindString:
        return make<String>(&managedPointers_.add(abi::getPointerValue(v)));
      case HermesABIValueKindObject:
        return make<Object>(&managedPointers_.add(abi::getPointerValue(v)));
      case HermesABIValueKindSymbol:
        return make<Symbol>(&managedPointers_.add(abi::getPointerValue(v)));
      case HermesABIValueKindBigInt:
        return make<BigInt>(&managedPointers_.add(abi::getPointerValue(v)));
      default:
        // We aren't able to construct an equivalent jsi::Value, just release
        // the value that was passed in.
        abi::releaseValue(v);
        throw JSINativeException("ABI returned an unknown value kind.");
    }
  }

  /// Helper function to take ownership of an ABI value as a JSI value and
  /// report any exceptions.
  Value intoJSIValue(const HermesABIValueOrError &val) {
    if (abi::isError(val))
      throwError(abi::getError(val));
    return intoJSIValue(abi::getValue(val));
  }

  /// Convert the given jsi::Value \p v to an ABI value. The ABI value will be
  /// invalidated once the jsi::Value is released.
  static HermesABIValue toABIValue(const Value &v) {
    if (v.isUndefined())
      return abi::createUndefinedValue();
    if (v.isNull())
      return abi::createNullValue();
    if (v.isBool())
      return abi::createBoolValue(v.getBool());
    if (v.isNumber())
      return abi::createNumberValue(v.getNumber());

    HermesABIManagedPointer *mp =
        static_cast<const ManagedPointerHolder *>(getPointerValue(v))
            ->getManagedPointer();
    if (v.isString())
      return abi::createStringValue(mp);
    if (v.isObject())
      return abi::createObjectValue(mp);
    if (v.isSymbol())
      return abi::createSymbolValue(mp);
    if (v.isBigInt())
      return abi::createBigIntValue(mp);

    BUILTIN_UNREACHABLE;
  }

 public:
  HermesABIRuntimeWrapper(const HermesABIVTable *vtable)
      : abiVtable_(vtable), managedPointers_(0.5, 0.5) {
    abiRt_ = abiVtable_->make_hermes_runtime(nullptr);
    vtable_ = abiRt_->vt;
  }
  ~HermesABIRuntimeWrapper() override {
    vtable_->release(abiRt_);
    assert(managedPointers_.sizeForTests() == 0 && "Dangling references.");
  }

  Value evaluateJavaScript(
      const std::shared_ptr<const Buffer> &buffer,
      const std::string &sourceURL) override {
    THROW_UNIMPLEMENTED();
  }

  std::shared_ptr<const PreparedJavaScript> prepareJavaScript(
      const std::shared_ptr<const Buffer> &buffer,
      std::string sourceURL) override {
    THROW_UNIMPLEMENTED();
  }

  Value evaluatePreparedJavaScript(
      const std::shared_ptr<const PreparedJavaScript> &js) override {
    THROW_UNIMPLEMENTED();
  }

  bool drainMicrotasks(int maxMicrotasksHint = -1) override {
    THROW_UNIMPLEMENTED();
  }

  Object global() override {
    THROW_UNIMPLEMENTED();
  }

  std::string description() override {
    return "HermesABIRuntimeWrapper";
  }

  bool isInspectable() override {
    THROW_UNIMPLEMENTED();
  }

  Instrumentation &instrumentation() override {
    THROW_UNIMPLEMENTED();
  }

 protected:
  PointerValue *cloneSymbol(const Runtime::PointerValue *pv) override {
    THROW_UNIMPLEMENTED();
  }
  PointerValue *cloneBigInt(const Runtime::PointerValue *pv) override {
    THROW_UNIMPLEMENTED();
  }
  PointerValue *cloneString(const Runtime::PointerValue *pv) override {
    THROW_UNIMPLEMENTED();
  }
  PointerValue *cloneObject(const Runtime::PointerValue *pv) override {
    THROW_UNIMPLEMENTED();
  }
  PointerValue *clonePropNameID(const Runtime::PointerValue *pv) override {
    THROW_UNIMPLEMENTED();
  }

  PropNameID createPropNameIDFromAscii(const char *str, size_t length)
      override {
    THROW_UNIMPLEMENTED();
  }
  PropNameID createPropNameIDFromUtf8(const uint8_t *utf8, size_t length)
      override {
    THROW_UNIMPLEMENTED();
  }
  PropNameID createPropNameIDFromString(const String &str) override {
    THROW_UNIMPLEMENTED();
  }
  PropNameID createPropNameIDFromSymbol(const Symbol &sym) override {
    THROW_UNIMPLEMENTED();
  }
  std::string utf8(const PropNameID &) override {
    THROW_UNIMPLEMENTED();
  }
  bool compare(const PropNameID &, const PropNameID &) override {
    THROW_UNIMPLEMENTED();
  }

  std::string symbolToString(const Symbol &) override {
    THROW_UNIMPLEMENTED();
  }

  BigInt createBigIntFromInt64(int64_t) override {
    THROW_UNIMPLEMENTED();
  }
  BigInt createBigIntFromUint64(uint64_t) override {
    THROW_UNIMPLEMENTED();
  }
  bool bigintIsInt64(const BigInt &) override {
    THROW_UNIMPLEMENTED();
  }
  bool bigintIsUint64(const BigInt &) override {
    THROW_UNIMPLEMENTED();
  }
  uint64_t truncate(const BigInt &) override {
    THROW_UNIMPLEMENTED();
  }
  String bigintToString(const BigInt &, int) override {
    THROW_UNIMPLEMENTED();
  }

  String createStringFromAscii(const char *str, size_t length) override {
    THROW_UNIMPLEMENTED();
  }
  String createStringFromUtf8(const uint8_t *utf8, size_t length) override {
    THROW_UNIMPLEMENTED();
  }
  std::string utf8(const String &) override {
    THROW_UNIMPLEMENTED();
  }

  Object createObject() override {
    THROW_UNIMPLEMENTED();
  }
  Object createObject(std::shared_ptr<HostObject> ho) override {
    THROW_UNIMPLEMENTED();
  }
  std::shared_ptr<HostObject> getHostObject(const Object &) override {
    THROW_UNIMPLEMENTED();
  }
  HostFunctionType &getHostFunction(const Function &) override {
    THROW_UNIMPLEMENTED();
  }

  bool hasNativeState(const Object &) override {
    THROW_UNIMPLEMENTED();
  }
  std::shared_ptr<NativeState> getNativeState(const Object &) override {
    THROW_UNIMPLEMENTED();
  }
  void setNativeState(const Object &, std::shared_ptr<NativeState> state)
      override {
    THROW_UNIMPLEMENTED();
  }

  Value getProperty(const Object &, const PropNameID &name) override {
    THROW_UNIMPLEMENTED();
  }
  Value getProperty(const Object &, const String &name) override {
    THROW_UNIMPLEMENTED();
  }
  bool hasProperty(const Object &, const PropNameID &name) override {
    THROW_UNIMPLEMENTED();
  }
  bool hasProperty(const Object &, const String &name) override {
    THROW_UNIMPLEMENTED();
  }
  void setPropertyValue(
      const Object &,
      const PropNameID &name,
      const Value &value) override {
    THROW_UNIMPLEMENTED();
  }
  void setPropertyValue(const Object &, const String &name, const Value &value)
      override {
    THROW_UNIMPLEMENTED();
  }

  bool isArray(const Object &) const override {
    THROW_UNIMPLEMENTED();
  }
  bool isArrayBuffer(const Object &) const override {
    THROW_UNIMPLEMENTED();
  }
  bool isFunction(const Object &) const override {
    THROW_UNIMPLEMENTED();
  }
  bool isHostObject(const Object &) const override {
    THROW_UNIMPLEMENTED();
  }
  bool isHostFunction(const Function &) const override {
    THROW_UNIMPLEMENTED();
  }
  Array getPropertyNames(const Object &) override {
    THROW_UNIMPLEMENTED();
  }

  WeakObject createWeakObject(const Object &) override {
    THROW_UNIMPLEMENTED();
  }
  Value lockWeakObject(const WeakObject &) override {
    THROW_UNIMPLEMENTED();
  }

  Array createArray(size_t length) override {
    THROW_UNIMPLEMENTED();
  }
  ArrayBuffer createArrayBuffer(
      std::shared_ptr<MutableBuffer> buffer) override {
    THROW_UNIMPLEMENTED();
  }
  size_t size(const Array &) override {
    THROW_UNIMPLEMENTED();
  }
  size_t size(const ArrayBuffer &) override {
    THROW_UNIMPLEMENTED();
  }
  uint8_t *data(const ArrayBuffer &) override {
    THROW_UNIMPLEMENTED();
  }
  Value getValueAtIndex(const Array &, size_t i) override {
    THROW_UNIMPLEMENTED();
  }
  void setValueAtIndexImpl(const Array &, size_t i, const Value &value)
      override {
    THROW_UNIMPLEMENTED();
  }

  Function createFunctionFromHostFunction(
      const PropNameID &name,
      unsigned int paramCount,
      HostFunctionType func) override {
    THROW_UNIMPLEMENTED();
  }
  Value call(
      const Function &,
      const Value &jsThis,
      const Value *args,
      size_t count) override {
    THROW_UNIMPLEMENTED();
  }
  Value callAsConstructor(const Function &, const Value *args, size_t count)
      override {
    THROW_UNIMPLEMENTED();
  }

  bool strictEquals(const Symbol &a, const Symbol &b) const override {
    THROW_UNIMPLEMENTED();
  }
  bool strictEquals(const BigInt &a, const BigInt &b) const override {
    THROW_UNIMPLEMENTED();
  }
  bool strictEquals(const String &a, const String &b) const override {
    THROW_UNIMPLEMENTED();
  }
  bool strictEquals(const Object &a, const Object &b) const override {
    THROW_UNIMPLEMENTED();
  }

  bool instanceOf(const Object &o, const Function &f) override {
    THROW_UNIMPLEMENTED();
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
