/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_HOST_MODEL_H
#define HERMES_VM_HOST_MODEL_H

#include "hermes/VM/Callable.h"

namespace hermes {
namespace vm {

/// A function to clean up the context used by a native function
typedef void (*FinalizeNativeFunctionPtr)(void *context);

/// This class represents a host function callable from JavaScript.
/// This is nearly the same as a NativeFunction, except an additional
/// function pointer can do cleanup when the FinalizableNativeFunction is
/// finalized.
class FinalizableNativeFunction final : public NativeFunction {
  FinalizeNativeFunctionPtr finalizePtr_;

 public:
  static CallableVTable vt;

  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::FinalizableNativeFunctionKind;
  }

  /// Create an instance of FinalizableNativeFunction.
  /// \param functionPtr the host function
  /// \param finalizePtr the finalizer function
  /// \param paramCount number of parameters (excluding `this`)
  static CallResult<HermesValue> createWithoutPrototype(
      Runtime *runtime,
      void *context,
      NativeFunctionPtr functionPtr,
      FinalizeNativeFunctionPtr finalizePtr,
      SymbolID name,
      unsigned paramCount);

  void *getContext() {
    return context_;
  }

 protected:
  FinalizableNativeFunction(
      Runtime *runtime,
      Handle<JSObject> parent,
      Handle<HiddenClass> clazz,
      void *context,
      NativeFunctionPtr functionPtr,
      FinalizeNativeFunctionPtr finalizePtr)
      : NativeFunction(
            runtime,
            &vt.base.base,
            *parent,
            *clazz,
            context,
            functionPtr),
        finalizePtr_(finalizePtr) {}

  ~FinalizableNativeFunction() {
    finalizePtr_(context_);
  }

  static void _finalizeImpl(GCCell *cell, GC *) {
    auto *self = vmcast<FinalizableNativeFunction>(cell);
    // Destruct the object.
    self->~FinalizableNativeFunction();
  }
};

/// When a HostObject is created, this proxy provides the business
/// logic for the HostObject's implementation.
class HostObjectProxy {
 public:
  // This is called when the object is finalized.
  virtual ~HostObjectProxy();

  // This is called to fetch a property value by name.
  virtual CallResult<HermesValue> get(SymbolID) = 0;

  // This is called to set a property value by name.  It will return
  // \c ExecutionStatus, and set the runtime's thrown value as appropriate.
  virtual CallResult<bool> set(SymbolID, HermesValue) = 0;

  // This is called to query names of properties.  In case of failure it will
  // return \c ExecutionStatus::EXCEPTION, and set the runtime's thrown Value
  // as appropriate.
  virtual CallResult<Handle<JSArray>> getHostPropertyNames() = 0;
};

class HostObject final : public JSObject {
 public:
  static ObjectVTable vt;

  static const PropStorage::size_type NEEDED_PROPERTY_SLOTS =
      JSObject::NEEDED_PROPERTY_SLOTS;

  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::HostObjectKind;
  }

  /// Create an instance of HostObject with no prototype.
  static CallResult<HermesValue> createWithoutPrototype(
      Runtime *runtime,
      std::shared_ptr<HostObjectProxy> proxy);

  CallResult<HermesValue> get(SymbolID name) {
    return proxy_->get(name);
  }

  CallResult<bool> set(SymbolID name, HermesValue value) {
    return proxy_->set(name, value);
  }

  CallResult<Handle<JSArray>> getHostPropertyNames() {
    return proxy_->getHostPropertyNames();
  }

  const std::shared_ptr<HostObjectProxy> &getProxy() const {
    return proxy_;
  }

 private:
  HostObject(
      Runtime *runtime,
      JSObject *parent,
      HiddenClass *clazz,
      std::shared_ptr<HostObjectProxy> proxy)
      : JSObject(runtime, &vt.base, parent, clazz), proxy_(proxy) {}

  static void _finalizeImpl(GCCell *cell, GC *gc);

  std::shared_ptr<HostObjectProxy> proxy_;
};

} // namespace vm
} // namespace hermes

#endif
