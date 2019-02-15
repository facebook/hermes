// Copyright 2004-present Facebook. All Rights Reserved.

#pragma once

#include <jsi/adapter.h>
#include <jsi/jsi.h>
#include <mutex>

namespace facebook {
namespace jsi {

// Base class of any given ThreadSafeRuntimeImpl. This exists so that you
// can write thread safe code that's agnostic of the underlying Runtime.
class ThreadSafeRuntime : public Runtime {
 public:
  virtual void lock() const = 0;
  virtual void unlock() const = 0;
  virtual Runtime& getUnsafeRuntime() = 0;
};

namespace detail {
// RAII object to scope critical sections with. Essentially std::lock_guard,
// but this can be initialized with a const lockable.
template <typename Lockable>
struct lock_guard {
  explicit lock_guard(const Lockable& lockable) : lockable_(lockable) {
    lockable_.lock();
  }

  ~lock_guard() {
    lockable_.unlock();
  }

 private:
  const Lockable& lockable_;
};

// The actual implementation of a given ThreadSafeRuntime. It's parameterized
// by:
//
// - R: The actual Runtime type that this wraps
// - L: A lock type that has two members:
//   - void lock(const R&)
//   - void unlock(const R&)
template <typename R, typename L>
class ThreadSafeRuntimeImpl : public ThreadSafeRuntime {
 public:
  using Locker = lock_guard<ThreadSafeRuntimeImpl<R, L>>;

  template <typename... Args>
  ThreadSafeRuntimeImpl(Args&&... args)
      : runtime_(std::forward<Args>(args)...) {}

  ~ThreadSafeRuntimeImpl() {}

  std::shared_ptr<const jsi::PreparedJavaScript> prepareJavaScript(
      const std::shared_ptr<const Buffer>& buffer,
      std::string sourceURL) override {
    Locker locker(*this);
    return runtime_.prepareJavaScript(std::move(buffer), std::move(sourceURL));
  }

  void evaluatePreparedJavaScript(
      const std::shared_ptr<const jsi::PreparedJavaScript>& js) override {
    Locker locker(*this);
    runtime_.evaluatePreparedJavaScript(js);
  }

  void evaluateJavaScript(
      const std::shared_ptr<const jsi::Buffer>& buffer,
      const std::string& sourceURL) override {
    Locker locker(*this);
    runtime_.evaluateJavaScript(buffer, sourceURL);
  }

  Object global() override {
    Locker locker(*this);
    return runtime_.global();
  }

  std::string description() override {
    Locker locker(*this);
    return runtime_.description();
  }

  bool isInspectable() override {
    Locker locker(*this);
    return runtime_.isInspectable();
  }

  void lock() const override {
    lock_.lock(runtime_);
  }

  void unlock() const override {
    lock_.unlock(runtime_);
  }

  R& getUnsafeRuntime() override {
    return runtime_;
  }

 protected:
  PointerValue* cloneString(const Runtime::PointerValue* pv) override {
    Locker locker(*this);
    return runtime_.cloneString(pv);
  }
  PointerValue* cloneObject(const Runtime::PointerValue* pv) override {
    Locker locker(*this);
    return runtime_.cloneObject(pv);
  }
  PointerValue* clonePropNameID(const Runtime::PointerValue* pv) override {
    Locker locker(*this);
    return runtime_.clonePropNameID(pv);
  }

  PropNameID createPropNameIDFromAscii(const char* str, size_t length)
      override {
    Locker locker(*this);
    return runtime_.createPropNameIDFromAscii(str, length);
  }

  PropNameID createPropNameIDFromUtf8(const uint8_t* utf8, size_t length)
      override {
    Locker locker(*this);
    return runtime_.createPropNameIDFromUtf8(utf8, length);
  }

  PropNameID createPropNameIDFromString(const String& str) override {
    Locker locker(*this);
    return runtime_.createPropNameIDFromString(str);
  }

  std::string utf8(const PropNameID& propId) override {
    Locker locker(*this);
    return runtime_.utf8(propId);
  }

  bool compare(const PropNameID& a, const PropNameID& b) override {
    Locker locker(*this);
    return runtime_.compare(a, b);
  }

  String createStringFromAscii(const char* str, size_t length) override {
    Locker locker(*this);
    return runtime_.createStringFromAscii(str, length);
  }

  String createStringFromUtf8(const uint8_t* utf8, size_t length) override {
    Locker locker(*this);
    return runtime_.createStringFromUtf8(utf8, length);
  }

  std::string utf8(const String& s) override {
    Locker locker(*this);
    return runtime_.utf8(s);
  }

  Object createObject() override {
    Locker locker(*this);
    return runtime_.createObject();
  }

  Object createObject(std::shared_ptr<HostObject> ho) override {
    Locker locker(*this);
    return runtime_.createObject(
        std::make_shared<HostObjectAdapter>(*this, std::move(ho)));
  }

  std::shared_ptr<HostObject> getHostObject(const jsi::Object& o) override {
    Locker locker(*this);
    return o.getHostObject<HostObjectAdapter>(runtime_)->ho;
  }

  HostFunctionType& getHostFunction(const jsi::Function& f) override {
    Locker locker(*this);
    return runtime_.getHostFunction(f)
        .template target<HostFunctionAdapter>()
        ->hf;
  }

  Value getProperty(const Object& o, const PropNameID& name) override {
    Locker locker(*this);
    return runtime_.getProperty(o, name);
  }

  Value getProperty(const Object& o, const String& name) override {
    Locker locker(*this);
    return runtime_.getProperty(o, name);
  }

  bool hasProperty(const Object& o, const PropNameID& name) override {
    Locker locker(*this);
    return runtime_.hasProperty(o, name);
  }

  bool hasProperty(const Object& o, const String& name) override {
    Locker locker(*this);
    return runtime_.hasProperty(o, name);
  }

  void setPropertyValue(Object& o, const PropNameID& name, const Value& value)
      override {
    Locker locker(*this);
    return runtime_.setPropertyValue(o, name, value);
  }

  void setPropertyValue(Object& o, const String& name, const Value& value)
      override {
    Locker locker(*this);
    return runtime_.setPropertyValue(o, name, value);
  }

  bool isArray(const Object& o) const override {
    Locker locker(*this);
    return runtime_.isArray(o);
  }

  bool isArrayBuffer(const Object& o) const override {
    Locker locker(*this);
    return runtime_.isArrayBuffer(o);
  }

  bool isFunction(const Object& o) const override {
    Locker locker(*this);
    return runtime_.isFunction(o);
  }

  bool isHostObject(const jsi::Object& o) const override {
    Locker locker(*this);
    return runtime_.isHostObject(o);
  }

  bool isHostFunction(const jsi::Function& f) const override {
    Locker locker(*this);
    return runtime_.isHostFunction(f);
  }

  Array getPropertyNames(const Object& o) override {
    Locker locker(*this);
    return runtime_.getPropertyNames(o);
  }

  WeakObject createWeakObject(const Object& o) override {
    Locker locker(*this);
    return runtime_.createWeakObject(o);
  }

  Value lockWeakObject(const WeakObject& wo) override {
    Locker locker(*this);
    return runtime_.lockWeakObject(wo);
  }

  Array createArray(size_t length) override {
    Locker locker(*this);
    return runtime_.createArray(length);
  }

  size_t size(const Array& a) override {
    Locker locker(*this);
    return runtime_.size(a);
  }

  size_t size(const ArrayBuffer& a) override {
    Locker locker(*this);
    return runtime_.size(a);
  }

  uint8_t* data(const ArrayBuffer& a) override {
    Locker locker(*this);
    return runtime_.data(a);
  }

  Value getValueAtIndex(const Array& a, size_t i) override {
    Locker locker(*this);
    return runtime_.getValueAtIndex(a, i);
  }

  void setValueAtIndexImpl(Array& a, size_t i, const Value& value) override {
    Locker locker(*this);
    return runtime_.setValueAtIndexImpl(a, i, value);
  }

  Function createFunctionFromHostFunction(
      const PropNameID& name,
      unsigned int paramCount,
      HostFunctionType func) override {
    Locker locker(*this);
    return runtime_.createFunctionFromHostFunction(
        name, paramCount, HostFunctionAdapter{*this, std::move(func)});
  }

  Value call(
      const Function& f,
      const Value& jsThis,
      const Value* args,
      size_t count) override {
    Locker locker(*this);
    return runtime_.call(f, jsThis, args, count);
  }

  Value callAsConstructor(const Function& f, const Value* args, size_t count)
      override {
    Locker locker(*this);
    return runtime_.callAsConstructor(f, args, count);
  }

  ScopeState* pushScope() override {
    Locker locker(*this);
    return runtime_.pushScope();
  }

  void popScope(ScopeState* ptr) override {
    Locker locker(*this);
    return runtime_.popScope(ptr);
  }

  bool strictEquals(const String& a, const String& b) const override {
    Locker locker(*this);
    return runtime_.strictEquals(a, b);
  }

  bool strictEquals(const Object& a, const Object& b) const override {
    Locker locker(*this);
    return runtime_.strictEquals(a, b);
  }

  bool instanceOf(const Object& o, const Function& f) override {
    Locker locker(*this);
    return runtime_.instanceOf(o, f);
  }

 private:
  R runtime_;
  L lock_;
};

} // namespace detail
} // namespace jsi
} // namespace facebook
