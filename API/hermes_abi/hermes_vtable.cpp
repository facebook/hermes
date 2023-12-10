/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes_abi/hermes_vtable.h"

#include "hermes_abi/hermes_abi.h"

#include "hermes/ADT/ManagedChunkedList.h"
#include "hermes/BCGen/HBC/BytecodeProviderFromSrc.h"
#include "hermes/Public/RuntimeConfig.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/JSArray.h"
#include "hermes/VM/JSArrayBuffer.h"
#include "hermes/VM/Runtime.h"
#include "hermes_abi/HermesABIHelpers.h"

#include "llvh/Support/ConvertUTF.h"

using namespace hermes;
using namespace facebook::hermes;

namespace {

/// A ManagedChunkedList element that indicates whether it's occupied based on
/// a refcount.
template <typename T>
class ManagedValue : public HermesABIManagedPointer {
  static void invalidate(HermesABIManagedPointer *ptr) {
    static_cast<ManagedValue<T> *>(ptr)->dec();
  }
  static constexpr HermesABIManagedPointerVTable vt{invalidate};

 public:
  ManagedValue() : HermesABIManagedPointer{&vt}, refCount_(0) {}

  /// Determine whether the element is occupied by inspecting the refcount.
  bool isFree() const {
    return refCount_.load(std::memory_order_relaxed) == 0;
  }

  /// Store a value and start the refcount at 1. After invocation, this
  /// instance is occupied with a value, and the "nextFree" methods should
  /// not be used until the value is released.
  template <typename... Args>
  void emplace(Args &&...args) {
    assert(isFree() && "Emplacing already occupied value");
    refCount_.store(1, std::memory_order_relaxed);
    new (&value_) T(std::forward<Args>(args)...);
  }

  /// Get the next free element. Must not be called when this instance is
  /// occupied with a value.
  ManagedValue<T> *getNextFree() {
    assert(isFree() && "Free pointer unusable while occupied");
    return nextFree_;
  }

  /// Set the next free element. Must not be called when this instance is
  /// occupied with a value.
  void setNextFree(ManagedValue<T> *nextFree) {
    assert(isFree() && "Free pointer unusable while occupied");
    nextFree_ = nextFree;
  }

  T &value() {
    assert(!isFree() && "Value not present");
    return value_;
  }

  const T &value() const {
    assert(!isFree() && "Value not present");
    return value_;
  }

  void inc() {
    // It is always safe to use relaxed operations for incrementing the
    // reference count, because the only operation that may occur concurrently
    // with it is decrementing the reference count, and we do not need to
    // enforce any ordering between the two.
    auto oldCount = refCount_.fetch_add(1, std::memory_order_relaxed);
    assert(oldCount && "Cannot resurrect a pointer");
    assert(oldCount + 1 != 0 && "Ref count overflow");
    (void)oldCount;
  }

  void dec() {
    // It is safe to use relaxed operations here because decrementing the
    // reference count is the only access that may be performed without proper
    // synchronisation. As a result, the only ordering we need to enforce when
    // decrementing is that the vtable pointer used to call \c invalidate is
    // loaded from before the decrement, in case the decrement ends up causing
    // this value to be freed. We get this ordering from the fact that the
    // vtable read and the reference count update form a load-store control
    // dependency, which preserves their ordering on any reasonable hardware.
    auto oldCount = refCount_.fetch_sub(1, std::memory_order_relaxed);
    assert(oldCount > 0 && "Ref count underflow");
    (void)oldCount;
  }

 private:
  std::atomic<uint32_t> refCount_;
  union {
    T value_;
    ManagedValue<T> *nextFree_;
  };
};

/// Helper functions to create a handle from a HermesABI reference.
template <typename T = vm::HermesValue>
vm::Handle<T> toHandle(HermesABIManagedPointer *value) {
  return vm::Handle<T>::vmcast(
      &static_cast<ManagedValue<vm::PinnedHermesValue> *>(value)->value());
}
vm::Handle<vm::JSObject> toHandle(HermesABIObject obj) {
  return toHandle<vm::JSObject>(obj.pointer);
}
vm::Handle<vm::StringPrimitive> toHandle(HermesABIString str) {
  return toHandle<vm::StringPrimitive>(str.pointer);
}
vm::Handle<vm::SymbolID> toHandle(HermesABISymbol sym) {
  return toHandle<vm::SymbolID>(sym.pointer);
}
vm::Handle<vm::SymbolID> toHandle(HermesABIPropNameID sym) {
  return toHandle<vm::SymbolID>(sym.pointer);
}
vm::Handle<vm::JSArray> toHandle(HermesABIArray arr) {
  return toHandle<vm::JSArray>(arr.pointer);
}
vm::Handle<vm::BigIntPrimitive> toHandle(HermesABIBigInt bi) {
  return toHandle<vm::BigIntPrimitive>(bi.pointer);
}
vm::Handle<vm::Callable> toHandle(HermesABIFunction fn) {
  return toHandle<vm::Callable>(fn.pointer);
}
vm::Handle<vm::JSArrayBuffer> toHandle(HermesABIArrayBuffer ab) {
  return toHandle<vm::JSArrayBuffer>(ab.pointer);
}

/// Helper function to convert a HermesABIValue to a HermesValue.
vm::HermesValue toHermesValue(const HermesABIValue &val) {
  switch (abi::getValueKind(val)) {
    case HermesABIValueKindUndefined:
      return vm::HermesValue::encodeUndefinedValue();
    case HermesABIValueKindNull:
      return vm::HermesValue::encodeNullValue();
    case HermesABIValueKindBoolean:
      return vm::HermesValue::encodeBoolValue(abi::getBoolValue(val));
    case HermesABIValueKindNumber:
      return vm::HermesValue::encodeUntrustedNumberValue(
          abi::getNumberValue(val));
    case HermesABIValueKindString:
    case HermesABIValueKindObject:
    case HermesABIValueKindSymbol:
    case HermesABIValueKindBigInt:
      return *toHandle<>(val.data.pointer);
    default:
      // This incoming value is either an error, or from a newer version of the
      // ABI, which violates our expectations.
      hermes_fatal("Value has an unexpected tag.");
  }
}

/// A thin wrapper around vm::Runtime to provide additional state for things
/// like pointer management. It is intended to provide a small number of helper
/// functions, with the core logic being kept in the actual API functions below,
/// which can directly manipulate the vm::Runtime.
class HermesABIRuntimeImpl : public HermesABIRuntime {
  static const HermesABIRuntimeVTable vtable;

 public:
  std::shared_ptr<::hermes::vm::Runtime> rt;
  ManagedChunkedList<ManagedValue<vm::PinnedHermesValue>> hermesValues;
  ManagedChunkedList<ManagedValue<vm::WeakRoot<vm::JSObject>>> weakHermesValues;

  /// This holds the message for cases where we throw a native exception.
  std::string nativeExceptionMessage{};

  explicit HermesABIRuntimeImpl(const hermes::vm::RuntimeConfig &runtimeConfig)
      : HermesABIRuntime{&vtable},
        rt(hermes::vm::Runtime::create(runtimeConfig)),
        hermesValues(runtimeConfig.getGCConfig().getOccupancyTarget(), 0.5),
        weakHermesValues(
            runtimeConfig.getGCConfig().getOccupancyTarget(),
            0.5) {
    // Add custom roots functions to the runtime to expose references retained
    // through the API as roots.
    rt->addCustomRootsFunction([this](vm::GC *, vm::RootAcceptor &acceptor) {
      hermesValues.forEach(
          [&acceptor](auto &element) { acceptor.accept(element.value()); });
    });
    rt->addCustomWeakRootsFunction(
        [this](vm::GC *, vm::WeakRootAcceptor &acceptor) {
          weakHermesValues.forEach([&acceptor](auto &element) {
            acceptor.acceptWeak(element.value());
          });
        });
  }

  ~HermesABIRuntimeImpl() {
    // Release the runtime to make sure that any remaining references held by
    // things like HostObject are freed.
    rt.reset();
    assert(hermesValues.sizeForTests() == 0 && "Dangling references.");
    assert(weakHermesValues.sizeForTests() == 0 && "Dangling references.");
  }

  /// Create a HermesABIValue from a HermesValue, including creating root
  /// entries for pointers if necessary.
  HermesABIValue createValue(vm::HermesValue hv) {
    switch (hv.getETag()) {
      case vm::HermesValue::ETag::Undefined:
        return abi::createUndefinedValue();
      case vm::HermesValue::ETag::Null:
        return abi::createNullValue();
      case vm::HermesValue::ETag::Bool:
        return abi::createBoolValue(hv.getBool());
      case vm::HermesValue::ETag::Symbol:
        return abi::createSymbolValue(&hermesValues.add(hv));
      case vm::HermesValue::ETag::Str1:
      case vm::HermesValue::ETag::Str2:
        return abi::createStringValue(&hermesValues.add(hv));
      case vm::HermesValue::ETag::BigInt1:
      case vm::HermesValue::ETag::BigInt2:
        return abi::createBigIntValue(&hermesValues.add(hv));
      case vm::HermesValue::ETag::Object1:
      case vm::HermesValue::ETag::Object2:
        return abi::createObjectValue(&hermesValues.add(hv));
      default:
        assert(hv.isNumber() && "No other types are permitted in the API.");
        return abi::createNumberValue(hv.getNumber());
    }
  }

  /// Convenience function to create a HermesABIValueOrError directly from a
  /// HermesValue.
  HermesABIValueOrError createValueOrError(vm::HermesValue hv) {
    return abi::createValueOrError(createValue(hv));
  }

  /// Create a HermesABIManagedPointer entry using the given HermesValue. This
  /// template should never be used directly, and is only useful so that we can
  /// define the pointer creation functions using a macro below.
  template <typename T>
  HermesABIManagedPointer *createPointerImpl(vm::HermesValue hv) {
    if constexpr (!std::is_same_v<T, HermesABIWeakObject>)
      return &hermesValues.add(hv);

    return &weakHermesValues.add(
        vm::WeakRoot<vm::JSObject>(vm::vmcast<vm::JSObject>(hv), *rt));
  }

  /// Define functions to create each of the Hermes ABI pointer types from a
  /// HermesValue.

#define DECLARE_HERMES_ABI_POINTER_HELPERS(name)                               \
  HermesABI##name create##name(vm::HermesValue hv) {                           \
    return abi::create##name(createPointerImpl<HermesABI##name>(hv));          \
  }                                                                            \
  HermesABI##name##OrError create##name##OrError(vm::HermesValue hv) {         \
    return abi::create##name##OrError(createPointerImpl<HermesABI##name>(hv)); \
  }
  HERMES_ABI_POINTER_TYPES(DECLARE_HERMES_ABI_POINTER_HELPERS)
#undef DECLARE_HERMES_ABI_POINTER_HELPERS

  /// Create a Handle<HermesValue> from a HermesABIValue. Note that the Handle
  /// will be cheaply constructed from the existing PinnedHermesValue, so the
  /// HermesABIValue must outlive the Handle.
  vm::Handle<> makeHandle(const HermesABIValue &val) {
    switch (abi::getValueKind(val)) {
      case HermesABIValueKindUndefined:
        return vm::Runtime::getUndefinedValue();
      case HermesABIValueKindNull:
        return vm::Runtime::getNullValue();
      case HermesABIValueKindBoolean:
        return vm::Runtime::getBoolValue(abi::getBoolValue(val));
      case HermesABIValueKindNumber:
        return rt->makeHandle(vm::HermesValue::encodeUntrustedNumberValue(
            abi::getNumberValue(val)));
      case HermesABIValueKindString:
      case HermesABIValueKindObject:
      case HermesABIValueKindSymbol:
      case HermesABIValueKindBigInt:
        return toHandle<>(val.data.pointer);
      default:
        // This incoming value is either an error, or from a newer version of
        // the ABI, which violates our expectations.
        hermes_fatal("Value has an unexpected tag.");
    }
  }

  /// Convert the error associated with the given error code \p err into a VM
  /// exception.
  vm::ExecutionStatus raiseError(HermesABIErrorCode err) {
    if (err == HermesABIErrorCodeJSError)
      return vm::ExecutionStatus::EXCEPTION;

    if (err == HermesABIErrorCodeNativeException) {
      auto msg = std::exchange(nativeExceptionMessage, {});

      // Treat the error message as UTF-8 and convert it to UTF-16 before
      // passing it into the VM.
      llvh::SmallVector<llvh::UTF16, 8> u16msg;
      if (!llvh::convertUTF8ToUTF16String(msg, u16msg))
        return rt->raiseError("<invalid utf-8 exception message>");

      static_assert(
          sizeof(llvh::UTF16) == sizeof(char16_t),
          "Cannot safely cast UTF16 to char16_t.");
      return rt->raiseError(
          vm::UTF16Ref{(char16_t *)u16msg.data(), u16msg.size()});
    }

    return rt->raiseError("<unknown native exception>");
  }
};

/// Convenience function to cast the given HermesABIRuntime to a
/// HermesABIRuntimeImpl.
HermesABIRuntimeImpl *impl(HermesABIRuntime *abiRt) {
  return static_cast<HermesABIRuntimeImpl *>(abiRt);
}

/// Helper function to write the given StringRef \p ref to the buffer \p buf.
/// Terminates if the size of the buffer cannot be grown sufficiently to hold
/// the resulting string.
/// TODO: Revisit the termination behavior. It may be preferable to report an
/// error or truncate the string depending on the usage.
void writeToBuf(HermesABIGrowableBuffer *buf, llvh::StringRef ref) {
  // Grow the buffer if necessary.
  if (buf->size < ref.size())
    buf->vtable->grow_to(buf, ref.size());

  // In the unlikely case that we failed to allocate enough space for the
  // message, fatal.
  if (buf->size < ref.size())
    hermes_fatal("Failed to allocate buffer to return string");

  // Copy the message into the buffer and adjust the buffer's available space.
  memcpy(buf->data, ref.data(), ref.size());
  buf->used = ref.size();
}

HermesABIRuntime *make_hermes_runtime(const HermesABIRuntimeConfig *config) {
  return new HermesABIRuntimeImpl({});
}

void release_hermes_runtime(HermesABIRuntime *abiRt) {
  delete impl(abiRt);
}

HermesABIValue get_and_clear_js_error_value(HermesABIRuntime *abiRt) {
  auto *hart = impl(abiRt);
  auto thrownValue = hart->rt->getThrownValue();

  // In debug builds, assert if there is no currently thrown value. In release
  // builds, return undefined so the behaviour is reasonable.
  assert(!thrownValue.isEmpty() && "Retrieving a non-existent error.");

  auto ret = thrownValue.isEmpty() ? abi::createUndefinedValue()
                                   : hart->createValue(thrownValue);
  hart->rt->clearThrownValue();
  return ret;
}

void get_and_clear_native_exception_message(
    HermesABIRuntime *abiRt,
    HermesABIGrowableBuffer *buf) {
  auto *hart = impl(abiRt);
  writeToBuf(buf, hart->nativeExceptionMessage);
  hart->nativeExceptionMessage.clear();
  hart->nativeExceptionMessage.shrink_to_fit();
}

void set_js_error_value(HermesABIRuntime *abiRt, const HermesABIValue *val) {
  impl(abiRt)->rt->setThrownValue(toHermesValue(*val));
}
void set_native_exception_message(
    HermesABIRuntime *abiRt,
    const uint8_t *message,
    size_t length) {
  impl(abiRt)->nativeExceptionMessage.assign((const char *)message, length);
}

constexpr HermesABIRuntimeVTable HermesABIRuntimeImpl::vtable = {
    release_hermes_runtime,
    get_and_clear_js_error_value,
    get_and_clear_native_exception_message,
    set_js_error_value,
    set_native_exception_message,
};

} // namespace

extern "C" {
const HermesABIVTable *get_hermes_abi_vtable() {
  static constexpr HermesABIVTable abiVtable = {
      make_hermes_runtime,
  };
  return &abiVtable;
}
} // extern "C"
