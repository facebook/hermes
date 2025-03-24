/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes_abi/HermesABIRuntimeWrapper.h"

#include "hermes_abi/HermesABIHelpers.h"
#include "hermes_abi/hermes_abi.h"
#include "jsi/jsilib.h"

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

  static void try_grow_to(HermesABIGrowableBuffer *buf, size_t sz) {
    auto *self = static_cast<StringByteBuffer *>(buf);
    // The API specifies that providing a smaller size is a no-op.
    if (sz < self->size)
      return;
    self->buf_.resize(sz);
    self->data = (uint8_t *)self->buf_.data();
    self->size = sz;
  }

  static constexpr HermesABIGrowableBufferVTable vt{
      try_grow_to,
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

/// An implementation of HermesABIBuffer that wraps a jsi::Buffer.
class BufferWrapper : public HermesABIBuffer {
  std::shared_ptr<const Buffer> buf_;

  static void release(HermesABIBuffer *buf) {
    delete static_cast<const BufferWrapper *>(buf);
  }
  static constexpr HermesABIBufferVTable vt{
      release,
  };

 public:
  explicit BufferWrapper(std::shared_ptr<const Buffer> buf)
      : HermesABIBuffer{&vt, buf->data(), buf->size()}, buf_(std::move(buf)) {}
};

class MutableBufferWrapper : public HermesABIMutableBuffer {
  std::shared_ptr<MutableBuffer> buf_;

  static void release(HermesABIMutableBuffer *buf) {
    delete static_cast<const MutableBufferWrapper *>(buf);
  }
  static constexpr HermesABIMutableBufferVTable vt{
      release,
  };

 public:
  explicit MutableBufferWrapper(std::shared_ptr<MutableBuffer> buf)
      : HermesABIMutableBuffer{&vt, buf->data(), buf->size()},
        buf_(std::move(buf)) {}
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

    void invalidate() noexcept override {
      dec();
    }

    void inc() noexcept {
      // See comments in hermes_abi.cpp for why we use relaxed operations here.
      auto oldCount = refCount_.fetch_add(1, std::memory_order_relaxed);
      assert(oldCount && "Cannot resurrect a pointer");
      assert(oldCount + 1 != 0 && "Ref count overflow");
      (void)oldCount;
    }

    void dec() noexcept {
      // See comments in hermes_abi.cpp for why we use relaxed operations here.
      auto oldCount = refCount_.fetch_sub(1, std::memory_order_relaxed);
      assert(oldCount > 0 && "Ref count underflow");
      // This was the last decrement of this holder, so we can invalidate the
      // underlying pointer.
      if (oldCount == 1)
        abi::releasePointer(managedPointer_);
    }
  };

  /// Invoke the given \p fn and return the result. If an exception occurs,
  /// catch it and report it through the ABI, using \p wrapErr to construct the
  /// appropriate ABI type that will store the error code.
  /// This is meant to be used by HostFunction and HostObject, to catch any
  /// exceptions thrown in the body of the executed user code and convert it
  /// into something that can be reported through the ABI.
  template <typename T, size_t N, typename Fn>
  T abiRethrow(
      T (*wrapErr)(HermesABIErrorCode),
      const char (&where)[N],
      Fn fn) {
    try {
      return fn();
    } catch (const JSError &e) {
      // Caught a JSError, retrieve its value and set the reported error.
      auto abiVal = toABIValue(e.value());
      vtable_->set_js_error_value(abiRt_, &abiVal);
      return wrapErr(HermesABIErrorCodeJSError);
    } catch (const std::exception &e) {
      // For all other native exceptions, register a native exception message
      // with the location where this error occurred.
      std::string what{"Exception in "};
      what.append(where, N - 1).append(": ").append(e.what());
      vtable_->set_native_exception_message(
          abiRt_, (const uint8_t *)what.c_str(), what.size());
      return wrapErr(HermesABIErrorCodeNativeException);
    } catch (...) {
      // Unknown exception, register a generic message.
      std::string err{"An unknown exception occurred in "};
      err.append(where, N - 1);
      vtable_->set_native_exception_message(
          abiRt_, (const uint8_t *)err.c_str(), err.size());
      return wrapErr(HermesABIErrorCodeNativeException);
    }
  }

  /// Implement HermesABIHostFunction for this JSI wrapper. This manages calling
  /// and releasing the underlying C++ JSI HostFunction through the ABI.
  class HostFunctionWrapper : public HermesABIHostFunction {
    /// The runtime wrapper that created this host function. It has to be stored
    /// so that it can be passed when invoking the HostFunction.
    HermesABIRuntimeWrapper &rtw_;

    /// The C++ function that is being wrapped.
    HostFunctionType hf_;

    static HermesABIValueOrError call(
        HermesABIHostFunction *hf,
        HermesABIRuntime *,
        const HermesABIValue *thisArg,
        const HermesABIValue *args,
        size_t count) {
      auto *self = static_cast<HostFunctionWrapper *>(hf);
      auto &rtw = self->rtw_;
      return rtw.abiRethrow(abi::createValueOrError, "HostFunction", [&] {
        // Convert the arguments from HermesABIValue to jsi::Value. This has to
        // be done under abiRethrow because cloneToJSIValue may throw if the
        // function was invoked with a value that is not known to this version
        // of JSI (potentially because it is from a newer version of the ABI).
        std::vector<Value> jsiArgs;
        jsiArgs.reserve(count);
        for (size_t i = 0; i < count; ++i)
          jsiArgs.emplace_back(rtw.cloneToJSIValue(args[i]));

        auto jsiThisArg = rtw.cloneToJSIValue(*thisArg);

        // Call the user provided function and convert the result back to a
        // HermesABIValue. Note that the resulting value must be cloned because
        // the returned jsi::Value will go out of scope after this function
        // returns.
        return abi::createValueOrError(rtw.cloneToABIValue(
            self->hf_(rtw, jsiThisArg, jsiArgs.data(), count)));
      });
    }

    static void release(HermesABIHostFunction *hf) {
      delete static_cast<HostFunctionWrapper *>(hf);
    }

   public:
    static constexpr HermesABIHostFunctionVTable vt{
        release,
        call,
    };

    HostFunctionWrapper(HermesABIRuntimeWrapper &rt, HostFunctionType hf)
        : HermesABIHostFunction{&vt}, rtw_{rt}, hf_{std::move(hf)} {}

    HostFunctionType &getHostFunction() {
      return hf_;
    }
  };

  /// Implement HermesABIHostObject for this JSI wrapper. This wraps a
  /// jsi::HostObject to allow the ABI to interact with it.
  class HostObjectWrapper : public HermesABIHostObject {
    /// The runtime wrapper that created this host object. It has to be stored
    /// so that it can be passed when invoking the HostObject's methods.
    HermesABIRuntimeWrapper &rtw_;

    /// The HostObject being managed by this HostObjectWrapper.
    std::shared_ptr<HostObject> ho_;

    /// Invoke the jsi::HostObject's get method and return the result,
    /// converting to/from the ABI representation.
    static HermesABIValueOrError
    get(HermesABIHostObject *ho, HermesABIRuntime *, HermesABIPropNameID name) {
      auto *self = static_cast<HostObjectWrapper *>(ho);
      auto &rtw = self->rtw_;
      return rtw.abiRethrow(abi::createValueOrError, "HostObject::get", [&] {
        auto jsiName = rtw.cloneToJSIPropNameID(name);
        return abi::createValueOrError(
            rtw.cloneToABIValue(self->ho_->get(rtw, jsiName)));
      });
    }

    /// Invoke the jsi::HostObject's set method, converting any resulting C++
    /// exception to an ABI error.
    static HermesABIVoidOrError set(
        HermesABIHostObject *ho,
        HermesABIRuntime *,
        HermesABIPropNameID name,
        const HermesABIValue *value) {
      auto *self = static_cast<HostObjectWrapper *>(ho);
      auto &rtw = self->rtw_;
      return rtw.abiRethrow(abi::createVoidOrError, "HostObject::set", [&] {
        auto jsiName = rtw.cloneToJSIPropNameID(name);
        // Note that cloneToJSIValue can throw, so it is important to have it in
        // the abiRethrow block.
        auto jsiValue = rtw.cloneToJSIValue(*value);
        self->ho_->set(rtw, jsiName, jsiValue);
        return abi::createVoidOrError();
      });
    }

    /// Implement HermesABIPropNameIDList for this JSI wrapper. This wraps the
    /// std::vector<PropNameID> returned by getting the properties from a
    /// jsi::HostObject, and exposes them to the ABI.
    class PropNameIDListWrapper : public HermesABIPropNameIDList {
      /// The jsi::PropNameIDs owned by this wrapper. These are never accessed,
      /// they are only kept to manage the lifetime of the PropNameIDs. This
      /// incurs some additional memory cost since we will keep both vectors
      /// alive, however, we happen to know that the PropNameIDListWrapper will
      /// be destroyed soon after being created, so the cost is minimal, and
      /// this is faster since it saves us needing to clone and release each
      /// ABIPropNameID.
      std::vector<PropNameID> jsiPropsVec_;

      /// The PropNameIDs in jsiPropsVec_ converted into HermesABIPropNameIDs.
      /// These are just aliases, so they are alive as long as the above vector
      /// is retained.
      /// TODO: Eliminate this field once the pointer representations are
      ///       unified.
      std::vector<HermesABIPropNameID> abiPropsVec_;

      static void release(HermesABIPropNameIDList *self) {
        delete static_cast<PropNameIDListWrapper *>(self);
      }
      static constexpr HermesABIPropNameIDListVTable vt{
          release,
      };

     public:
      /// Construct a PropNameIDListWrapper from a vector of jsi::PropNameIDs,
      /// and a vector of HermesABIPropNameIDs that alias them.
      PropNameIDListWrapper(
          std::vector<PropNameID> jsiPropsVec,
          std::vector<HermesABIPropNameID> abiPropsVec) {
        vtable = &vt;
        jsiPropsVec_ = std::move(jsiPropsVec);
        abiPropsVec_ = std::move(abiPropsVec);
        props = abiPropsVec_.data();
        size = abiPropsVec_.size();
      }
    };

    /// Invoke the jsi::HostObject's getOwnPropertyNames method and return the
    /// result in ABI representation.
    static HermesABIPropNameIDListPtrOrError get_own_keys(
        HermesABIHostObject *ho,
        HermesABIRuntime *) {
      auto *self = static_cast<HostObjectWrapper *>(ho);
      auto &rtw = self->rtw_;

      return rtw.abiRethrow(
          abi::createPropNameIDListPtrOrError,
          "HostObject::getPropertyNames",
          [&] {
            auto res = self->ho_->getPropertyNames(rtw);
            // Create a vector of ABIPropNameIDs while keeping the ownership
            // with the jsi::PropNameID.
            std::vector<HermesABIPropNameID> v;
            for (auto &p : res)
              v.push_back(rtw.toABIPropNameID(p));
            return abi::createPropNameIDListPtrOrError(
                new PropNameIDListWrapper(std::move(res), std::move(v)));
          });
    }

    static void release(HermesABIHostObject *ho) {
      delete static_cast<HostObjectWrapper *>(ho);
    }

   public:
    static constexpr HermesABIHostObjectVTable vt{
        release,
        get,
        set,
        get_own_keys,
    };

    HostObjectWrapper(
        HermesABIRuntimeWrapper &rt,
        std::shared_ptr<HostObject> ho)
        : HermesABIHostObject{&vt}, rtw_{rt}, ho_{std::move(ho)} {}

    std::shared_ptr<HostObject> getHostObject() const {
      return ho_;
    }
  };

  /// Implement HermesABINativeState for this wrapper. This wraps a
  /// jsi::NativeState, allowing it to be exposed through the ABI.
  class NativeStateWrapper : public HermesABINativeState {
    /// The jsi::NativeState managed by this wrapper.
    std::shared_ptr<NativeState> nativeState_;

    static void release(HermesABINativeState *self) {
      delete static_cast<NativeStateWrapper *>(self);
    }

   public:
    static constexpr HermesABINativeStateVTable vt{
        release,
    };

    NativeStateWrapper(std::shared_ptr<NativeState> nativeState)
        : HermesABINativeState{&vt}, nativeState_{std::move(nativeState)} {}

    std::shared_ptr<NativeState> getNativeState() const {
      return nativeState_;
    }
  };

  PointerValue *clone(const PointerValue *pv) {
    // TODO: Evaluate whether to keep this null check. It is currently here for
    //       compatibility with hermes' API, but it is odd that it is the only
    //       API that allows null.
    if (!pv)
      return nullptr;

    auto *nonConst = const_cast<PointerValue *>(pv);
    static_cast<ManagedPointerHolder *>(nonConst)->inc();
    return nonConst;
  }

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

  PropNameID cloneToJSIPropNameID(HermesABIPropNameID name) {
    return intoJSIPropNameID(vtable_->clone_propnameid(abiRt_, name));
  }

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

  /// Create a jsi::Value from the given HermesABIValue without taking ownership
  /// of it. This will clone any underlying pointers if needed.
  Value cloneToJSIValue(const HermesABIValue &v) {
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
        return intoJSIString(
            vtable_->clone_string(abiRt_, abi::getStringValue(v)));
      case HermesABIValueKindObject:
        return intoJSIObject(
            vtable_->clone_object(abiRt_, abi::getObjectValue(v)));
      case HermesABIValueKindSymbol:
        return intoJSISymbol(
            vtable_->clone_symbol(abiRt_, abi::getSymbolValue(v)));
      case HermesABIValueKindBigInt:
        return intoJSIBigInt(
            vtable_->clone_bigint(abiRt_, abi::getBigIntValue(v)));
      default:
        // Don't release the value here, since we didn't take ownership.
        throw JSINativeException("ABI returned an unknown value kind.");
    }
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

  /// Convert the given jsi::Value \p v to an ABI value. The resulting ABI value
  /// will need to be explicitly released.
  HermesABIValue cloneToABIValue(const Value &v) {
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
      return abi::createStringValue(
          vtable_->clone_string(abiRt_, abi::createString(mp)));
    if (v.isObject())
      return abi::createObjectValue(
          vtable_->clone_object(abiRt_, abi::createObject(mp)));
    if (v.isSymbol())
      return abi::createSymbolValue(
          vtable_->clone_symbol(abiRt_, abi::createSymbol(mp)));
    if (v.isBigInt())
      return abi::createBigIntValue(
          vtable_->clone_bigint(abiRt_, abi::createBigInt(mp)));

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
    auto *bw = new BufferWrapper(buffer);
    if (abiVtable_->is_hermes_bytecode(buffer->data(), buffer->size()))
      return intoJSIValue(vtable_->evaluate_hermes_bytecode(
          abiRt_, bw, sourceURL.c_str(), sourceURL.size()));

    return intoJSIValue(vtable_->evaluate_javascript_source(
        abiRt_, bw, sourceURL.c_str(), sourceURL.size()));
  }

  std::shared_ptr<const PreparedJavaScript> prepareJavaScript(
      const std::shared_ptr<const Buffer> &buffer,
      std::string sourceURL) override {
    return std::make_shared<const SourceJavaScriptPreparation>(
        buffer, std::move(sourceURL));
  }

  Value evaluatePreparedJavaScript(
      const std::shared_ptr<const PreparedJavaScript> &js) override {
    assert(dynamic_cast<const SourceJavaScriptPreparation *>(js.get()));
    auto sjp = std::static_pointer_cast<const SourceJavaScriptPreparation>(js);
    return evaluateJavaScript(sjp, sjp->sourceURL());
  }

  void queueMicrotask(const Function & /*callback*/) override {
    THROW_UNIMPLEMENTED();
  }

  bool drainMicrotasks(int maxMicrotasksHint = -1) override {
    return unwrap(vtable_->drain_microtasks(abiRt_, maxMicrotasksHint));
  }

  Object global() override {
    return intoJSIObject(vtable_->get_global_object(abiRt_));
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
    return clone(pv);
  }
  PointerValue *cloneBigInt(const Runtime::PointerValue *pv) override {
    return clone(pv);
  }
  PointerValue *cloneString(const Runtime::PointerValue *pv) override {
    return clone(pv);
  }
  PointerValue *cloneObject(const Runtime::PointerValue *pv) override {
    return clone(pv);
  }
  PointerValue *clonePropNameID(const Runtime::PointerValue *pv) override {
    return clone(pv);
  }

  PropNameID createPropNameIDFromAscii(const char *str, size_t length)
      override {
    return createPropNameIDFromString(createStringFromAscii(str, length));
  }
  PropNameID createPropNameIDFromUtf8(const uint8_t *utf8, size_t length)
      override {
    return createPropNameIDFromString(createStringFromUtf8(utf8, length));
  }
  PropNameID createPropNameIDFromString(const String &str) override {
    return intoJSIPropNameID(
        vtable_->create_propnameid_from_string(abiRt_, toABIString(str)));
  }
  PropNameID createPropNameIDFromSymbol(const Symbol &sym) override {
    return intoJSIPropNameID(
        vtable_->create_propnameid_from_symbol(abiRt_, toABISymbol(sym)));
  }
  std::string utf8(const PropNameID &name) override {
    StringByteBuffer buffer;
    vtable_->get_utf8_from_propnameid(abiRt_, toABIPropNameID(name), &buffer);
    return std::move(buffer).get();
  }
  bool compare(const PropNameID &a, const PropNameID &b) override {
    return vtable_->prop_name_id_equals(
        abiRt_, toABIPropNameID(a), toABIPropNameID(b));
  }

  std::string symbolToString(const Symbol &sym) override {
    StringByteBuffer buffer;
    vtable_->get_utf8_from_symbol(abiRt_, toABISymbol(sym), &buffer);
    return std::move(buffer).get();
  }

  BigInt createBigIntFromInt64(int64_t value) override {
    return intoJSIBigInt(vtable_->create_bigint_from_int64(abiRt_, value));
  }
  BigInt createBigIntFromUint64(uint64_t value) override {
    return intoJSIBigInt(vtable_->create_bigint_from_uint64(abiRt_, value));
  }
  bool bigintIsInt64(const BigInt &bigint) override {
    return vtable_->bigint_is_int64(abiRt_, toABIBigInt(bigint));
  }
  bool bigintIsUint64(const BigInt &bigint) override {
    return vtable_->bigint_is_uint64(abiRt_, toABIBigInt(bigint));
  }
  uint64_t truncate(const BigInt &bigint) override {
    return vtable_->bigint_truncate_to_uint64(abiRt_, toABIBigInt(bigint));
  }
  String bigintToString(const BigInt &bigint, int radix) override {
    // Note that the ABI takes the radix as unsigned, but it is safe to pass in
    // the signed value without a check because values <2 or >36 will be
    // rejected anyway.
    return intoJSIString(vtable_->bigint_to_string(
        abiRt_, toABIBigInt(bigint), (unsigned)radix));
  }

  String createStringFromAscii(const char *str, size_t length) override {
    // For now, there is no ASCII specific function, so just use the UTF-8 one.
    return createStringFromUtf8(reinterpret_cast<const uint8_t *>(str), length);
  }
  String createStringFromUtf8(const uint8_t *utf8, size_t length) override {
    return intoJSIString(
        vtable_->create_string_from_utf8(abiRt_, utf8, length));
  }
  std::string utf8(const String &str) override {
    StringByteBuffer buffer;
    vtable_->get_utf8_from_string(abiRt_, toABIString(str), &buffer);
    return std::move(buffer).get();
  }

  Object createObject() override {
    return intoJSIObject(vtable_->create_object(abiRt_));
  }
  Object createObject(std::shared_ptr<HostObject> ho) override {
    return intoJSIObject(vtable_->create_object_from_host_object(
        abiRt_, new HostObjectWrapper(*this, std::move(ho))));
  }
  std::shared_ptr<HostObject> getHostObject(const Object &o) override {
    return static_cast<HostObjectWrapper *>(
               vtable_->get_host_object(abiRt_, toABIObject(o)))
        ->getHostObject();
  }
  HostFunctionType &getHostFunction(const Function &f) override {
    return static_cast<HostFunctionWrapper *>(
               vtable_->get_host_function(abiRt_, toABIFunction(f)))
        ->getHostFunction();
  }

  bool hasNativeState(const Object &obj) override {
    // Check if there is a native state, and that it was created by this
    // wrapper. Checking the vtable allows us to safely share the
    // HermesABIRuntime with other users in the future.
    if (auto *ns = vtable_->get_native_state(abiRt_, toABIObject(obj)))
      return ns->vtable == &NativeStateWrapper::vt;
    return false;
  }
  std::shared_ptr<NativeState> getNativeState(const Object &obj) override {
    auto *ns = vtable_->get_native_state(abiRt_, toABIObject(obj));
    return static_cast<NativeStateWrapper *>(ns)->getNativeState();
  }
  void setNativeState(const Object &obj, std::shared_ptr<NativeState> state)
      override {
    unwrap(vtable_->set_native_state(
        abiRt_, toABIObject(obj), new NativeStateWrapper(std::move(state))));
  }

  Value getProperty(const Object &obj, const PropNameID &name) override {
    return intoJSIValue(vtable_->get_object_property_from_propnameid(
        abiRt_, toABIObject(obj), toABIPropNameID(name)));
  }
  Value getProperty(const Object &obj, const String &name) override {
    // Create an ABI value that is just a reference to \p name, without taking
    // ownership of it.
    auto abiKey = abi::createStringValue(toABIString(name));
    return intoJSIValue(vtable_->get_object_property_from_value(
        abiRt_, toABIObject(obj), &abiKey));
  }
  bool hasProperty(const Object &obj, const PropNameID &name) override {
    return unwrap(vtable_->has_object_property_from_propnameid(
        abiRt_, toABIObject(obj), toABIPropNameID(name)));
  }
  bool hasProperty(const Object &obj, const String &name) override {
    // Create an ABI value that is just a reference to \p name, without taking
    // ownership of it.
    auto abiKey = abi::createStringValue(toABIString(name));
    return unwrap(vtable_->has_object_property_from_value(
        abiRt_, toABIObject(obj), &abiKey));
  }
  void setPropertyValue(
      const Object &obj,
      const PropNameID &name,
      const Value &value) override {
    auto abiVal = toABIValue(value);
    unwrap(vtable_->set_object_property_from_propnameid(
        abiRt_, toABIObject(obj), toABIPropNameID(name), &abiVal));
  }
  void setPropertyValue(
      const Object &obj,
      const String &name,
      const Value &value) override {
    // Create an ABI value that is just a reference to \p name, without taking
    // ownership of it.
    auto abiKey = abi::createStringValue(toABIString(name));
    auto abiVal = toABIValue(value);
    unwrap(vtable_->set_object_property_from_value(
        abiRt_, toABIObject(obj), &abiKey, &abiVal));
  }

  bool isArray(const Object &obj) const override {
    return vtable_->object_is_array(abiRt_, toABIObject(obj));
  }
  bool isArrayBuffer(const Object &obj) const override {
    return vtable_->object_is_arraybuffer(abiRt_, toABIObject(obj));
  }
  bool isFunction(const Object &obj) const override {
    return vtable_->object_is_function(abiRt_, toABIObject(obj));
  }
  bool isHostObject(const Object &obj) const override {
    // Check if it is a HostObject and was created by this C++ wrapper. Checking
    // the vtable allows us to safely share the HermesABIRuntime with other
    // users in the future.
    if (auto *ho = vtable_->get_host_object(abiRt_, toABIObject(obj)))
      return ho->vtable == &HostObjectWrapper::vt;
    return false;
  }
  bool isHostFunction(const Function &fn) const override {
    // Check if it is a HostFunction and was created by this C++ wrapper.
    // Checking the vtable allows us to safely share the HermesABIRuntime with
    // other users in the future.
    if (auto *hf = vtable_->get_host_function(abiRt_, toABIFunction(fn)))
      return hf->vtable == &HostFunctionWrapper::vt;
    return false;
  }
  Array getPropertyNames(const Object &obj) override {
    return intoJSIArray(
        vtable_->get_object_property_names(abiRt_, toABIObject(obj)));
  }

  WeakObject createWeakObject(const Object &obj) override {
    return intoJSIWeakObject(
        vtable_->create_weak_object(abiRt_, toABIObject(obj)));
  }
  Value lockWeakObject(const WeakObject &wo) override {
    return intoJSIValue(vtable_->lock_weak_object(abiRt_, toABIWeakObject(wo)));
  }

  Array createArray(size_t length) override {
    return intoJSIArray(vtable_->create_array(abiRt_, length));
  }
  ArrayBuffer createArrayBuffer(
      std::shared_ptr<MutableBuffer> buffer) override {
    return intoJSIArrayBuffer(vtable_->create_arraybuffer_from_external_data(
        abiRt_, new MutableBufferWrapper(std::move(buffer))));
  }
  size_t size(const Array &arr) override {
    return vtable_->get_array_length(abiRt_, toABIArray(arr));
  }
  size_t size(const ArrayBuffer &ab) override {
    return unwrap(vtable_->get_arraybuffer_size(abiRt_, toABIArrayBuffer(ab)));
  }
  uint8_t *data(const ArrayBuffer &ab) override {
    return unwrap(vtable_->get_arraybuffer_data(abiRt_, toABIArrayBuffer(ab)));
  }
  Value getValueAtIndex(const Array &arr, size_t i) override {
    if (i >= arr.length(*this))
      throw JSINativeException("Array index out of bounds.");

    auto abiKey = abi::createNumberValue(i);
    return intoJSIValue(vtable_->get_object_property_from_value(
        abiRt_, toABIObject(arr), &abiKey));
  }
  void setValueAtIndexImpl(const Array &arr, size_t i, const Value &value)
      override {
    if (i >= arr.length(*this))
      throw JSINativeException("Array index out of bounds.");

    auto abiKey = abi::createNumberValue(i);
    auto abiVal = toABIValue(value);
    unwrap(vtable_->set_object_property_from_value(
        abiRt_, toABIObject(arr), &abiKey, &abiVal));
  }

  Function createFunctionFromHostFunction(
      const PropNameID &name,
      unsigned int paramCount,
      HostFunctionType func) override {
    return intoJSIFunction(vtable_->create_function_from_host_function(
        abiRt_,
        toABIPropNameID(name),
        paramCount,
        new HostFunctionWrapper(*this, func)));
  }
  Value call(
      const Function &fn,
      const Value &jsThis,
      const Value *args,
      size_t count) override {
    // Convert the arguments from jsi::Value to HermesABIValue to make the call.
    // TODO: Remove this conversion once they have the same representation.
    std::vector<HermesABIValue> abiArgs;
    abiArgs.reserve(count);
    for (size_t i = 0; i < count; ++i)
      abiArgs.push_back(toABIValue(args[i]));
    HermesABIValue abiThis = toABIValue(jsThis);
    return intoJSIValue(vtable_->call(
        abiRt_, toABIFunction(fn), &abiThis, abiArgs.data(), abiArgs.size()));
  }
  Value callAsConstructor(const Function &fn, const Value *args, size_t count)
      override {
    // Convert the arguments from jsi::Value to HermesABIValue to make the call.
    // TODO: Remove this conversion once they have the same representation.
    std::vector<HermesABIValue> abiArgs;
    abiArgs.reserve(count);
    for (size_t i = 0; i < count; ++i)
      abiArgs.push_back(toABIValue(args[i]));
    return intoJSIValue(vtable_->call_as_constructor(
        abiRt_, toABIFunction(fn), abiArgs.data(), abiArgs.size()));
  }

  bool strictEquals(const Symbol &a, const Symbol &b) const override {
    return vtable_->strict_equals_symbol(
        abiRt_, toABISymbol(a), toABISymbol(b));
  }
  bool strictEquals(const BigInt &a, const BigInt &b) const override {
    return vtable_->strict_equals_bigint(
        abiRt_, toABIBigInt(a), toABIBigInt(b));
  }
  bool strictEquals(const String &a, const String &b) const override {
    return vtable_->strict_equals_string(
        abiRt_, toABIString(a), toABIString(b));
  }
  bool strictEquals(const Object &a, const Object &b) const override {
    return vtable_->strict_equals_object(
        abiRt_, toABIObject(a), toABIObject(b));
  }

  bool instanceOf(const Object &o, const Function &f) override {
    return unwrap(
        vtable_->instance_of(abiRt_, toABIObject(o), toABIFunction(f)));
  }

  void setExternalMemoryPressure(const Object &obj, size_t amount) override {
    unwrap(vtable_->set_object_external_memory_pressure(
        abiRt_, toABIObject(obj), amount));
  }
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
