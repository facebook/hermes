/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_CALLABLE_H
#define HERMES_VM_CALLABLE_H

#include "hermes/VM/ArrayStorage.h"
#include "hermes/VM/Domain.h"
#include "hermes/VM/Handle.h"
#include "hermes/VM/JSObject.h"
#include "hermes/VM/NativeArgs.h"
#include "hermes/VM/Runtime-inline.h"

extern "C" {
struct SHRuntime;
struct SHNativeFuncInfo;
}

namespace hermes {
namespace vm {

/// Environment storing all escaping variables for a function.
class Environment final
    : public VariableSizeRuntimeCell,
      private llvh::TrailingObjects<Environment, GCHermesValue> {
  friend TrailingObjects;
  friend void EnvironmentBuildMeta(const GCCell *cell, Metadata::Builder &mb);

  /// The parent lexical environment. This value will be nullptr if the
  /// parent is the global scope.
  GCPointer<Environment> parentEnvironment_{};

  /// Number of entries in the environment.
  AtomicIfConcurrentGC<uint32_t> size_;

 public:
  static const VTable vt;

  static constexpr CellKind getCellKind() {
    return CellKind::EnvironmentKind;
  }
  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::EnvironmentKind;
  }

  /// Create a new Environment.
  static HermesValue create(
      Runtime &runtime,
      Handle<Environment> parentEnvironment,
      uint32_t size) {
    auto *cell = runtime.makeAVariable<Environment>(
        allocationSize(size), runtime, parentEnvironment, size);
    return HermesValue::encodeObjectValue(cell);
  }

  static Environment *
  create(Runtime &runtime, Handle<Callable> parentFn, uint32_t size) {
    return runtime.makeAVariable<Environment>(
        allocationSize(size), runtime, parentFn, size);
  }

  /// \return the parent lexical environment. This value will be nullptr if the
  /// parent is the global scope.
  Environment *getParentEnvironment(PointerBase &runtime) const {
    return parentEnvironment_.get(runtime);
  }

  /// \return the number of HermesValue slots in this environment.
  uint32_t getSize() const {
    return size_.load(std::memory_order_relaxed);
  }

  GCHermesValue &slot(uint32_t index) {
    assert(index < getSize() && "invalid Environment slot index");
    return getSlots()[index];
  }

  /// Gets the amount of memory required by this object for a given size.
  static uint32_t allocationSize(uint32_t size) {
    return totalSizeToAlloc<GCHermesValue>(size);
  }

 public:
  /// \param parentEnvironment the parent lexical environment, or nullptr if the
  ///   parent is the global scope.
  /// \param size the number of entries in the environment.
  Environment(
      Runtime &runtime,
      Handle<Environment> parentEnvironment,
      uint32_t size)
      : parentEnvironment_(runtime, parentEnvironment.get(), runtime.getHeap()),
        size_(size) {
    // Initialize all slots to 'undefined'.
    GCHermesValue::uninitialized_fill(
        getSlots(),
        getSlots() + size,
        HermesValue::encodeUndefinedValue(),
        runtime.getHeap(),
        this);
  }

  /// Create an environment using the given function to retrieve the parent
  /// environment. \param parentFn the function whose environment is the parent
  /// of this one. \param size the number of entries in the environment.
  inline Environment(
      Runtime &runtime,
      Handle<Callable> parentFn,
      uint32_t size);

 private:
  /// \return a pointer to the array of HermesValue.
  GCHermesValue *getSlots() {
    return getTrailingObjects<GCHermesValue>();
  }
  const GCHermesValue *getSlots() const {
    return getTrailingObjects<GCHermesValue>();
  }

  /// Dummy function for static asserts that may need private fields.
  static inline void staticAsserts() {
    static_assert(sizeof(Environment) == sizeof(SHEnvironment));
    static_assert(
        offsetof(Environment, parentEnvironment_) ==
        offsetof(SHEnvironment, parentEnvironment));
    static_assert(
        offsetof(Environment, size_) == offsetof(SHEnvironment, size));
    llvm_unreachable("staticAsserts must never be called.");
  }
};

struct CallableVTable : public ObjectVTable {
  /// Create a new object instance to be passed as the 'this' argument when
  /// invoking the constructor. Overriding this method allows creation of
  /// different underlying native objects.
  CallResult<PseudoHandle<JSObject>> (*newObject)(
      Handle<Callable> selfHandle,
      Runtime &runtime,
      Handle<JSObject> parentHandle);

  /// Call the callable with arguments already on the stack.
  CallResult<PseudoHandle<>> (
      *call)(Handle<Callable> selfHandle, Runtime &runtime);
};

/// The abstract base for callable entities, specifically NativeFunction and
/// Function. It presents the ability to call a function with arguments already
/// on the stack. Subclasses implement this for native functions and
/// interpreted functions.
class Callable : public JSObject {
  using Super = JSObject;
  friend void CallableBuildMeta(const GCCell *cell, Metadata::Builder &mb);

  /// Environment containing all captured variables for the function.
  GCPointer<Environment> environment_{};

  /// Dummy function for static asserts that may need private fields.
  static inline void staticAsserts();

 public:
  static bool classof(const GCCell *cell) {
    return kindInRange(
        cell->getKind(),
        CellKind::CallableKind_first,
        CellKind::CallableKind_last);
  }

  const CallableVTable *getVT() const {
    return static_cast<const CallableVTable *>(GCCell::getVT());
  }

  /// \return the environment associated with this callable.
  Environment *getEnvironment(PointerBase &runtime) const {
    return environment_.get(runtime);
  }

  enum class WritablePrototype : uint8_t { No = 0, Yes = 1 };

  /// Initialize the name, length and prototype property. This method factors
  /// out common code for initialization of callable-s.
  /// First, defines the ".name" property. It is set to the empty string if
  /// \p name is \c INVALID_IDENTIFIER_ID.
  /// Then it defines the ".length" property to \p paramCount.
  /// If \p prototypeObjectHandle is not null, sets the ".prototype" property
  /// to that value, and also defines ".prototype.constructor" to point back
  /// to the callable object.
  /// \param name identifier id of the function name, or
  ///   \c INVALID_IDENTIFIER_ID.
  /// \param paramCount number of declared named parameters
  /// \param prototypeObjectHandle optional value for .prototype
  /// \param writablePrototype determines whether the prototype property will
  ///   be created as writable or read-only.
  static ExecutionStatus defineNameLengthAndPrototype(
      Handle<Callable> selfHandle,
      Runtime &runtime,
      SymbolID name,
      unsigned paramCount,
      Handle<JSObject> prototypeObjectHandle,
      WritablePrototype writablePrototype);

  /// \return true if \p fn is a generator function.
  static bool isGeneratorFunction(Callable *fn);

  /// \return true if \p fn is an async function.
  static bool isAsyncFunction(Callable *fn);

  /// Execute this function with no arguments. This is just a convenience
  /// helper method; it actually invokes the interpreter recursively.
  static CallResult<PseudoHandle<>> executeCall0(
      Handle<Callable> selfHandle,
      Runtime &runtime,
      Handle<> thisArgHandle,
      bool construct = false);

  /// Execute this function with one argument. This is just a convenience
  /// helper method; it actually invokes the interpreter recursively.
  static CallResult<PseudoHandle<>> executeCall1(
      Handle<Callable> selfHandle,
      Runtime &runtime,
      Handle<> thisArgHandle,
      HermesValue param1,
      bool construct = false);

  /// Execute this function with two arguments. This is just a convenience
  /// helper method; it actually invokes the interpreter recursively.
  static CallResult<PseudoHandle<>> executeCall2(
      Handle<Callable> selfHandle,
      Runtime &runtime,
      Handle<> thisArgHandle,
      HermesValue param1,
      HermesValue param2,
      bool construct = false);

  /// Execute this function with three arguments. This is just a convenience
  /// helper method; it actually invokes the interpreter recursively.
  static CallResult<PseudoHandle<>> executeCall3(
      Handle<Callable> selfHandle,
      Runtime &runtime,
      Handle<> thisArgHandle,
      HermesValue param1,
      HermesValue param2,
      HermesValue param3,
      bool construct = false);

  /// Execute this function with four arguments. This is just a convenience
  /// helper method; it actually invokes the interpreter recursively.
  static CallResult<PseudoHandle<>> executeCall4(
      Handle<Callable> selfHandle,
      Runtime &runtime,
      Handle<> thisArgHandle,
      HermesValue param1,
      HermesValue param2,
      HermesValue param3,
      HermesValue param4,
      bool construct = false);

  /// Execute this function with given this and newTarget, and a
  /// variable number of arguments taken from arrayLike.  This invokes
  /// the interpreter recursively.
  static CallResult<PseudoHandle<>> executeCall(
      Handle<Callable> selfHandle,
      Runtime &runtime,
      Handle<> newTarget,
      Handle<> thisArgument,
      Handle<JSObject> arrayLike);

  /// Calls CallableVTable::newObject.
  static CallResult<PseudoHandle<JSObject>> newObject(
      Handle<Callable> selfHandle,
      Runtime &runtime,
      Handle<JSObject> parentHandle) {
    return selfHandle->getVT()->newObject(selfHandle, runtime, parentHandle);
  }

  /// Calls CallableVTable::call.
  static CallResult<PseudoHandle<>> call(
      Handle<Callable> selfHandle,
      Runtime &runtime) {
    // Any call to a native or JS function could potentially allocate.
    // Move the heap to force raw pointer errors to come out whenever a call is
    // made.
    runtime.potentiallyMoveHeap();
    return selfHandle->getVT()->call(selfHandle, runtime);
  }

  /// Call the callable in construct mode with arguments already on the stack.
  /// Checks the return value of the called function. If it is an object, then
  /// it is returned, else the `this` value is returned.
  static CallResult<PseudoHandle<>>
  construct(Handle<Callable> selfHandle, Runtime &runtime, Handle<> thisVal) {
    auto result = call(selfHandle, runtime);
    if (LLVM_UNLIKELY(result == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    return (*result)->isObject()
        ? std::move(result)
        : CallResult<PseudoHandle<>>{PseudoHandle<>(thisVal)};
  }

  /// Create a new object and construct the new object of the given type
  /// by invoking \p selfHandle with construct=true.
  /// \param selfHandle the Callable from which to construct the new object.
  static CallResult<PseudoHandle<>> executeConstruct0(
      Handle<Callable> selfHandle,
      Runtime &runtime);

  /// Create a new object and construct the new object of the given type
  /// by invoking \p selfHandle with construct=true.
  /// \param selfHandle the Callable from which to construct the new object.
  /// \param param1 the first argument to the constructor.
  static CallResult<PseudoHandle<>> executeConstruct1(
      Handle<Callable> selfHandle,
      Runtime &runtime,
      Handle<> param1);

  /// If the own property ".length" is present and is a number, convert it to
  /// integer and return it. Otherwise return 0. Note that it is not guaranteed
  /// to be non-negative.
  /// Follows ES2018 19.2.3.2 5 and 6.
  static CallResult<double> extractOwnLengthProperty_RJS(
      Handle<Callable> selfHandle,
      Runtime &runtime);

  /// Define the length, name, prototype of this function, used when the
  /// creation has been delayed by lazy objects.
  static void defineLazyProperties(Handle<Callable> fn, Runtime &runtime);

  /// Create an object by calling newObject on \p selfHandle.
  /// The object can then be used as the "this" argument when calling
  /// \p selfHandle to construct an object.
  /// Retrieves the "prototype" property from \p selfHandle,
  /// and calls newObject() on it if it's an object,
  /// else calls newObject() on the built-in Object prototype object.
  static CallResult<PseudoHandle<JSObject>> createThisForConstruct_RJS(
      Handle<Callable> selfHandle,
      Runtime &runtime);

 protected:
  Callable(
      Runtime &runtime,
      JSObject *parent,
      HiddenClass *clazz,
      Handle<Environment> env)
      : JSObject(runtime, parent, clazz),
        environment_(runtime, *env, runtime.getHeap()) {}
  Callable(Runtime &runtime, JSObject *parent, HiddenClass *clazz)
      : JSObject(runtime, parent, clazz), environment_() {}

#ifdef HERMES_MEMORY_INSTRUMENTATION
  static std::string _snapshotNameImpl(GCCell *cell, GC &gc);
#endif

  /// Create a an instance of Object to be passed as the 'this' argument when
  /// invoking the constructor.
  static CallResult<PseudoHandle<JSObject>> _newObjectImpl(
      Handle<Callable> selfHandle,
      Runtime &runtime,
      Handle<JSObject> parentHandle);
};

void Callable::staticAsserts() {
  static_assert(sizeof(Callable) == sizeof(SHCallable));
  static_assert(
      offsetof(Callable, environment_) == offsetof(SHCallable, environment));
}

Environment::Environment(
    Runtime &runtime,
    Handle<Callable> parentFn,
    uint32_t size)
    : parentEnvironment_(
          runtime,
          // TODO: Consider keeping the parent as a compressed pointer.
          parentFn->getEnvironment(runtime),
          runtime.getHeap()),
      size_(size) {
  // Initialize all slots to 'undefined'.
  GCHermesValue::uninitialized_fill(
      getSlots(),
      getSlots() + size,
      HermesValue::encodeUndefinedValue(),
      runtime.getHeap(),
      this);
}

/// A function produced by Function.prototype.bind(). It packages a function
/// with values for some of its parameters.
class BoundFunction final : public Callable {
  friend void BoundFunctionBuildMeta(const GCCell *cell, Metadata::Builder &mb);

  /// The target function to call.
  GCPointer<Callable> target_;

  /// Storage for the arguments.
  GCPointer<ArrayStorage> argStorage_;

 public:
  using Super = Callable;
  static const CallableVTable vt;

  static constexpr CellKind getCellKind() {
    return CellKind::BoundFunctionKind;
  }
  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::BoundFunctionKind;
  }

  /// \return the target function.
  Callable *getTarget(PointerBase &runtime) const {
    return target_.get(runtime);
  }

  /// Creates a BoundFunction instance bound to \p target. It can throw if
  /// obtaining \c target.length throws.
  /// \param argCountWithThis number of arguments, including \c this. It can be
  ///     0 in which case there is no \c this.
  /// \param argsWithThis the arguments, \c this at index 0.
  static CallResult<HermesValue> create(
      Runtime &runtime,
      Handle<Callable> target,
      unsigned argCountWithThis,
      ConstArgIterator argsWithThis);

  /// Perform the actual call. This is a light-weight handler which is part of
  /// the private API - it is only used internally and by the interpreter.
  /// Other users of this class must use \c Callable::call().
  /// \param ip the caller's IP at the point of the call (used for preserving
  /// stack traces).
  static CallResult<PseudoHandle<>>
  _boundCall(BoundFunction *self, const Inst *ip, Runtime &runtime);

  /// Initialize the length and name and property of a lazily created bound
  /// function.
  static ExecutionStatus initializeLengthAndName_RJS(
      Handle<Callable> selfHandle,
      Runtime &runtime,
      Handle<Callable> target,
      unsigned argCount);

  /// \return the number of arguments, including the 'this' param.
  unsigned getArgCountWithThis(PointerBase &runtime) const {
    return argStorage_.getNonNull(runtime)->size();
  }

 public:
  BoundFunction(
      Runtime &runtime,
      Handle<JSObject> parent,
      Handle<HiddenClass> clazz,
      Handle<Callable> target,
      Handle<ArrayStorage> argStorage)
      : Callable(runtime, *parent, *clazz),
        target_(runtime, *target, runtime.getHeap()),
        argStorage_(runtime, *argStorage, runtime.getHeap()) {}

 private:
  /// Return a pointer to the stored arguments, including \c this. \c this is
  /// at index 0, followed by the rest.
  GCHermesValue *getArgsWithThis(PointerBase &runtime) {
    return argStorage_.getNonNull(runtime)->begin();
  }

  /// Create an instance of the object using the bound constructor.
  static CallResult<PseudoHandle<JSObject>> _newObjectImpl_RJS(
      Handle<Callable> selfHandle,
      Runtime &runtime,
      Handle<JSObject> parentHandle);

  /// Call the callable with arguments already on the stack.
  static CallResult<PseudoHandle<>> _callImpl(
      Handle<Callable> selfHandle,
      Runtime &runtime);
};

/// This class represents a native function callable from JavaScript with
/// context and the JavaScript arguments.
class NativeJSFunction : public Callable {
 protected:
  /// Pointer to the actual code.
  const NativeJSFunctionPtr functionPtr_;
  /// Pointer to the information describing this function.
  const SHNativeFuncInfo *functionInfo_;
  /// Pointer to the SHUnit that created this function.
  const SHUnit *unit_;

#ifdef HERMESVM_PROFILER_NATIVECALL
  /// How many times the function was called.
  uint32_t callCount_{0};
  /// Total duration of all calls.
  uint64_t callDuration_{0};
#endif

 public:
  using Super = Callable;
  static const CallableVTable vt;

  static constexpr CellKind getCellKind() {
    return CellKind::SHLegacyFunctionKind;
  }
  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::SHLegacyFunctionKind;
  }

  NativeJSFunctionPtr getFunctionPtr() const {
    return functionPtr_;
  }

  const SHNativeFuncInfo *getFunctionInfo() const {
    return functionInfo_;
  }

  const SHUnit *getUnit() const {
    return unit_;
  }

#ifdef HERMESVM_PROFILER_NATIVECALL
  uint32_t getCallCount() const {
    return callCount_;
  }
  uint32_t getCallDuration() const {
    return callDuration_;
  }
#else
  uint32_t getCallCount() const {
    return 0;
  }
  uint32_t getCallDuration() const {
    return 0;
  }
#endif

  /// This is a lightweight and unsafe wrapper intended to be used only by the
  /// interpreter. Its purpose is to avoid needlessly exposing the private
  /// fields.
  static CallResult<PseudoHandle<>> _nativeCall(
      NativeJSFunction *self,
      Runtime &runtime);

  /// Call originating from LegacyJS.
  static SHLegacyValue _legacyCall(SHRuntime *shr, NativeJSFunction *self) {
    return self->functionPtr_(shr);
  }

  /// Create an instance of SHLegacyFunction.
  /// \param parentHandle object to use as [[Prototype]].
  /// \param context the context to be passed to the function
  /// \param functionPtr the native function
  /// \param funcInfo pointer to the information describing the function.
  /// \param additionalSlotCount internal slots to reserve within the
  /// object (defaults to zero).
  static Handle<NativeJSFunction> create(
      Runtime &runtime,
      Handle<JSObject> parentHandle,
      NativeJSFunctionPtr functionPtr,
      const SHNativeFuncInfo *funcInfo,
      const SHUnit *unit,
      unsigned additionalSlotCount = 0);

  /// Create an instance of SHLegacyFunction.
  /// \param parentHandle object to use as [[Prototype]].
  /// \param parentEnvHandle the parent environment
  /// \param context the context to be passed to the function
  /// \param functionPtr the native function
  /// \param funcInfo pointer to the information describing the function.
  /// \param additionalSlotCount internal slots to reserve within the
  /// object (defaults to zero).
  static Handle<NativeJSFunction> create(
      Runtime &runtime,
      Handle<JSObject> parentHandle,
      Handle<Environment> parentEnvHandle,
      NativeJSFunctionPtr functionPtr,
      const SHNativeFuncInfo *funcInfo,
      const SHUnit *unit,
      unsigned additionalSlotCount = 0);

  /// Create a Function with the prototype property set to new Object(). The
  /// parent is inferred by the kind of the function.
  /// \param parentEnvHandle the parent environment
  /// \param context the context to be passed to the function
  /// \param functionPtr the native function
  /// \param funcInfo pointer to the information describing the function.
  /// \param additionalSlotCount internal slots to reserve within the
  /// object (defaults to zero).
  static Handle<NativeJSFunction> createWithInferredParent(
      Runtime &runtime,
      Handle<Environment> parentEnvHandle,
      NativeJSFunctionPtr functionPtr,
      const SHNativeFuncInfo *funcInfo,
      const SHUnit *unit,
      unsigned additionalSlotCount = 0);

  /// \return the value in an additional slot.
  /// \param index must be less than the \c additionalSlotCount passed to
  /// the create method.
  static SmallHermesValue getAdditionalSlotValue(
      NativeJSFunction *self,
      Runtime &runtime,
      unsigned index) {
    return JSObject::getInternalProperty(
        self, runtime, numOverlapSlots<NativeJSFunction>() + index);
  }

  /// Set the value in an additional slot.
  /// \param index must be less than the \c additionalSlotCount passed to
  /// the create method.
  static void setAdditionalSlotValue(
      NativeJSFunction *self,
      Runtime &runtime,
      unsigned index,
      SmallHermesValue value) {
    JSObject::setInternalProperty(
        self, runtime, numOverlapSlots<NativeJSFunction>() + index, value);
  }

 public:
  NativeJSFunction(
      Runtime &runtime,
      Handle<JSObject> parent,
      Handle<HiddenClass> clazz,
      NativeJSFunctionPtr functionPtr,
      const SHNativeFuncInfo *funcInfo,
      const SHUnit *unit)
      : Callable(runtime, *parent, *clazz),
        functionPtr_(functionPtr),
        functionInfo_(funcInfo),
        unit_(unit) {}
  NativeJSFunction(
      Runtime &runtime,
      Handle<JSObject> parent,
      Handle<HiddenClass> clazz,
      Handle<Environment> environment,
      NativeJSFunctionPtr functionPtr,
      const SHNativeFuncInfo *funcInfo,
      const SHUnit *unit)
      : Callable(runtime, *parent, *clazz, environment),
        functionPtr_(functionPtr),
        functionInfo_(funcInfo),
        unit_(unit) {}

 protected:
#ifdef HERMES_MEMORY_INSTRUMENTATION
  static std::string _snapshotNameImpl(GCCell *cell, GC &gc);
#endif

  /// Call the native function with arguments already on the stack.
  static CallResult<PseudoHandle<>> _callImpl(
      Handle<Callable> selfHandle,
      Runtime &runtime);

 private:
  /// Dummy function for static asserts that may need private fields.
  static inline void staticAsserts() {
    static_assert(sizeof(NativeJSFunction) == sizeof(SHNativeJSFunction));
    static_assert(
        offsetof(NativeJSFunction, functionPtr_) ==
        offsetof(SHNativeJSFunction, functionPtr));
  }
};

/// A pointer to native function.
typedef CallResult<HermesValue> (
    *NativeFunctionPtr)(void *context, Runtime &runtime, NativeArgs args);

/// This class represents a native function callable from JavaScript with
/// context and the JavaScript arguments.
class NativeFunction : public Callable {
 protected:
  /// Context to be passed to the native function.
  void *const context_;
  /// Pointer to the actual code.
  const NativeFunctionPtr functionPtr_;

#ifdef HERMESVM_PROFILER_NATIVECALL
  /// How many times the function was called.
  uint32_t callCount_{0};
  /// Total duration of all calls.
  uint64_t callDuration_{0};
#endif

 public:
  using Super = Callable;
  static const CallableVTable vt;

  static constexpr CellKind getCellKind() {
    return CellKind::NativeFunctionKind;
  }
  static bool classof(const GCCell *cell) {
    return kindInRange(
        cell->getKind(),
        CellKind::NativeFunctionKind_first,
        CellKind::NativeFunctionKind_last);
  }

  void *getContext() const {
    return context_;
  }

  NativeFunctionPtr getFunctionPtr() const {
    return functionPtr_;
  }

#ifdef HERMESVM_PROFILER_NATIVECALL
  uint32_t getCallCount() const {
    return callCount_;
  }
  uint32_t getCallDuration() const {
    return callDuration_;
  }
#else
  uint32_t getCallCount() const {
    return 0;
  }
  uint32_t getCallDuration() const {
    return 0;
  }
#endif

  /// This is a lightweight and unsafe wrapper intended to be used only by the
  /// interpreter. Its purpose is to avoid needlessly exposing the private
  /// fields.
  static CallResult<PseudoHandle<>> _nativeCall(
      NativeFunction *self,
      Runtime &runtime) {
    ScopedNativeDepthTracker depthTracker{runtime};
    if (LLVM_UNLIKELY(depthTracker.overflowed())) {
      return runtime.raiseStackOverflow(
          Runtime::StackOverflowKind::NativeStack);
    }

    auto newFrame = runtime.setCurrentFrameToTopOfStack();
    runtime.saveCallerIPInStackFrame();
    // Allocate the "reserved" registers in the new frame.
    if (LLVM_UNLIKELY(!runtime.checkAndAllocStack(
            StackFrameLayout::CalleeExtraRegistersAtStart,
            HermesValue::encodeUndefinedValue()))) {
      // Restore the stack before raising the overflow.
      runtime.restoreStackAndPreviousFrame(newFrame);
      return runtime.raiseStackOverflow(
          Runtime::StackOverflowKind::JSRegisterStack);
    }

#ifdef HERMESVM_PROFILER_NATIVECALL
    auto t1 = HERMESVM_RDTSC();
#endif

    auto res =
        self->functionPtr_(self->context_, runtime, newFrame.getNativeArgs());

#ifdef HERMESVM_PROFILER_NATIVECALL
    self->callDuration_ = HERMESVM_RDTSC() - t1;
    ++self->callCount_;
#endif
    runtime.restoreStackAndPreviousFrame(newFrame);
    if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    return createPseudoHandle(*res);
  }

  /// Create an instance of NativeFunction.
  /// \param parentHandle object to use as [[Prototype]].
  /// \param context the context to be passed to the function
  /// \param functionPtr the native function
  /// \param name the name property of the function.
  /// \param paramCount number of parameters (excluding `this`)
  /// \param prototypeObjectHandle if non-null, set as prototype property.
  /// \param additionalSlotCount internal slots to reserve within the
  /// object (defaults to zero).
  static Handle<NativeFunction> create(
      Runtime &runtime,
      Handle<JSObject> parentHandle,
      void *context,
      NativeFunctionPtr functionPtr,
      SymbolID name,
      unsigned paramCount,
      Handle<JSObject> prototypeObjectHandle,
      unsigned additionalSlotCount = 0);

  /// Create an instance of NativeFunction.
  /// \param parentHandle object to use as [[Prototype]].
  /// \param parentEnvHandle the parent environment
  /// \param context the context to be passed to the function
  /// \param functionPtr the native function
  /// \param name the name property of the function.
  /// \param paramCount number of parameters (excluding `this`)
  /// \param prototypeObjectHandle if non-null, set as prototype property.
  /// \param additionalSlotCount internal slots to reserve within the
  /// object (defaults to zero).
  static Handle<NativeFunction> create(
      Runtime &runtime,
      Handle<JSObject> parentHandle,
      Handle<Environment> parentEnvHandle,
      void *context,
      NativeFunctionPtr functionPtr,
      SymbolID name,
      unsigned paramCount,
      Handle<JSObject> prototypeObjectHandle,
      unsigned additionalSlotCount = 0);

  /// Create an instance of NativeFunction.
  /// The prototype property will be null.
  /// \param parentHandle object to use as [[Prototype]].
  /// \param context the context to be passed to the function
  /// \param functionPtr the native function
  /// \param name the name property of the function.
  /// \param paramCount number of parameters (excluding `this`)
  /// \param additionalSlotCount internal slots to reserve within the
  /// object (defaults to zero).
  static Handle<NativeFunction> createWithoutPrototype(
      Runtime &runtime,
      Handle<JSObject> parentHandle,
      void *context,
      NativeFunctionPtr functionPtr,
      SymbolID name,
      unsigned paramCount,
      unsigned additionalSlotCount = 0) {
    return create(
        runtime,
        parentHandle,
        context,
        functionPtr,
        name,
        paramCount,
        runtime.makeNullHandle<JSObject>(),
        additionalSlotCount);
  }

  /// Create an instance of NativeFunction
  /// The [[Prototype]] will be Function.prototype.
  /// The prototype property wil be null;
  /// \param context the context to be passed to the function
  /// \param functionPtr the native function
  /// \param name the name property of the function.
  /// \param paramCount number of parameters (excluding `this`)
  /// \param additionalSlotCount internal slots to reserve within the
  /// object (defaults to zero).
  static Handle<NativeFunction> createWithoutPrototype(
      Runtime &runtime,
      void *context,
      NativeFunctionPtr functionPtr,
      SymbolID name,
      unsigned paramCount,
      unsigned additionalSlotCount = 0) {
    return createWithoutPrototype(
        runtime,
        Handle<JSObject>::vmcast(&runtime.functionPrototype),
        context,
        functionPtr,
        name,
        paramCount,
        additionalSlotCount);
  }

  /// \return the value in an additional slot.
  /// \param index must be less than the \c additionalSlotCount passed to
  /// the create method.
  static SmallHermesValue getAdditionalSlotValue(
      NativeFunction *self,
      Runtime &runtime,
      unsigned index) {
    return JSObject::getInternalProperty(
        self, runtime, numOverlapSlots<NativeFunction>() + index);
  }

  /// Set the value in an additional slot.
  /// \param index must be less than the \c additionalSlotCount passed to
  /// the create method.
  static void setAdditionalSlotValue(
      NativeFunction *self,
      Runtime &runtime,
      unsigned index,
      SmallHermesValue value) {
    JSObject::setInternalProperty(
        self, runtime, numOverlapSlots<NativeFunction>() + index, value);
  }

 public:
  NativeFunction(
      Runtime &runtime,
      Handle<JSObject> parent,
      Handle<HiddenClass> clazz,
      void *context,
      NativeFunctionPtr functionPtr)
      : Callable(runtime, *parent, *clazz),
        context_(context),
        functionPtr_(functionPtr) {}
  NativeFunction(
      Runtime &runtime,
      Handle<JSObject> parent,
      Handle<HiddenClass> clazz,
      Handle<Environment> environment,
      void *context,
      NativeFunctionPtr functionPtr)
      : Callable(runtime, *parent, *clazz, environment),
        context_(context),
        functionPtr_(functionPtr) {}

 protected:
#ifdef HERMES_MEMORY_INSTRUMENTATION
  static std::string _snapshotNameImpl(GCCell *cell, GC &gc);
#endif

  /// Call the native function with arguments already on the stack.
  static CallResult<PseudoHandle<>> _callImpl(
      Handle<Callable> selfHandle,
      Runtime &runtime);

  /// We have to override this method because NativeFunction should not be
  /// used as constructor.
  /// Note: this may change in the future, in that case, we should create
  /// a subclass of NativeFunction for this restriction.
  static CallResult<PseudoHandle<JSObject>>
  _newObjectImpl(Handle<Callable>, Runtime &runtime, Handle<JSObject>);
};

/// A NativeFunction to be used as a constructor for native objects other than
/// Object.
class NativeConstructor final : public NativeFunction {
 public:
  /// A CreatorFunction is responsible for creating the 'this' object that the
  /// constructor function sees.
  /// \p proto is the '.prototype' property of the constructor and should be set
  /// as the __proto__ for the nascent object.
  /// \p context is the context pointer provided to the NativeConstructor.
  using CreatorFunction = CallResult<PseudoHandle<JSObject>>(
      Runtime &,
      Handle<JSObject> proto,
      void *context);

  /// Unifies signatures of various GCCells so that they may be stored
  /// in the NativeConstructor.
  /// Delegates to toCallResultPseudoHandleJSObject to convert various
  /// types to CallResult<PseudoHandle<JSObject>>.
  template <class NativeClass>
  static CallResult<PseudoHandle<JSObject>>
  creatorFunction(Runtime &runtime, Handle<JSObject> prototype, void *context);

  static const CallableVTable vt;

  static constexpr CellKind getCellKind() {
    return CellKind::NativeConstructorKind;
  }
  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::NativeConstructorKind;
  }

  /// Create an instance of NativeConstructor.
  /// \param parentHandle the __proto__ of the resulting constructor. Note this
  /// is NOT .prototype; use Callable::defineNameLengthAndPrototype to set that.
  /// \param context the context pointer to be passed to the native function
  /// \param functionPtr the native function
  /// \param paramCount number of parameters (excluding `this`)
  /// \param creator the function invoked to create the proposed 'this' object
  /// passed to the constructor
  /// \param targetKind the expected CellKind of objects produced by the
  /// constructor
  static PseudoHandle<NativeConstructor> create(
      Runtime &runtime,
      Handle<JSObject> parentHandle,
      void *context,
      NativeFunctionPtr functionPtr,
      unsigned paramCount,
      CreatorFunction *creator,
      CellKind targetKind) {
    auto *cell = runtime.makeAFixed<NativeConstructor>(
        runtime,
        parentHandle,
        runtime.getHiddenClassForPrototype(
            *parentHandle, numOverlapSlots<NativeConstructor>()),
        context,
        functionPtr,
        creator,
        targetKind);
    return JSObjectInit::initToPseudoHandle(runtime, cell);
  }

  /// Create an instance of NativeConstructor.
  /// \param parentHandle object to use as [[Prototype]].
  /// \param parentEnvHandle the parent environment
  /// \param context the context to be passed to the function
  /// \param functionPtr the native function
  static PseudoHandle<NativeConstructor> create(
      Runtime &runtime,
      Handle<JSObject> parentHandle,
      Handle<Environment> parentEnvHandle,
      void *context,
      NativeFunctionPtr functionPtr,
      CreatorFunction *creator,
      CellKind targetKind) {
    auto *cell = runtime.makeAFixed<NativeConstructor>(
        runtime,
        parentHandle,
        runtime.getHiddenClassForPrototype(
            *parentHandle, numOverlapSlots<NativeConstructor>()),
        parentEnvHandle,
        context,
        functionPtr,
        creator,
        targetKind);
    return JSObjectInit::initToPseudoHandle(runtime, cell);
  }

 private:
#ifndef NDEBUG
  /// Kind of the object returned by this native constructor.
  const CellKind targetKind_;
#endif

  /// Function used to create new object from this constructor.
  /// Typically passed by invoking NativeConstructor::creatorFunction<T>.
  CreatorFunction *const creator_;

 public:
  NativeConstructor(
      Runtime &runtime,
      Handle<JSObject> parent,
      Handle<HiddenClass> clazz,
      void *context,
      NativeFunctionPtr functionPtr,
      CreatorFunction *creator,
      CellKind targetKind)
      : NativeFunction(runtime, parent, clazz, context, functionPtr),
#ifndef NDEBUG
        targetKind_(targetKind),
#endif
        creator_(creator) {
  }

  NativeConstructor(
      Runtime &runtime,
      Handle<JSObject> parent,
      Handle<HiddenClass> clazz,
      Handle<Environment> parentEnvHandle,
      void *context,
      NativeFunctionPtr functionPtr,
      CreatorFunction *creator,
      CellKind targetKind)
      : NativeFunction(
            runtime,
            parent,
            clazz,
            parentEnvHandle,
            context,
            functionPtr),
#ifndef NDEBUG
        targetKind_(targetKind),
#endif
        creator_(creator) {
  }

 private:
  /// Create a an instance of an object from \c creator_ to be passed as the
  /// 'this' argument when invoking the constructor.
  static CallResult<PseudoHandle<JSObject>> _newObjectImpl(
      Handle<Callable> selfHandle,
      Runtime &runtime,
      Handle<JSObject> parentHandle) {
    auto nativeConsHandle = Handle<NativeConstructor>::vmcast(selfHandle);
    return nativeConsHandle->creator_(
        runtime, parentHandle, nativeConsHandle->getContext());
  }

#ifndef NDEBUG
  /// If construct=true, check that the constructor was called with a "this"
  /// of the correct type.
  static CallResult<PseudoHandle<>> _callImpl(
      Handle<Callable> selfHandle,
      Runtime &runtime);
#endif
};

/// An interpreted callable function with environment.
class JSFunction : public Callable {
  using Super = Callable;
  friend void JSFunctionBuildMeta(const GCCell *cell, Metadata::Builder &mb);

  /// CodeBlock to execute when called.
  CodeBlock *codeBlock_;

  static constexpr auto kHasFinalizer = HasFinalizer::No;

  /// JSFunctions must keep their domains alive.
  GCPointer<Domain> domain_;

 public:
  JSFunction(
      Runtime &runtime,
      Handle<Domain> domain,
      Handle<JSObject> parent,
      Handle<HiddenClass> clazz,
      Handle<Environment> environment,
      CodeBlock *codeBlock)
      : Callable(runtime, *parent, *clazz, environment),
        codeBlock_(codeBlock),
        domain_(runtime, *domain, runtime.getHeap()) {
    assert(
        !vt.finalize_ == (kHasFinalizer != HasFinalizer::Yes) &&
        "kHasFinalizer invalid value");
  }

 public:
  static const CallableVTable vt;

  static constexpr CellKind getCellKind() {
    return CellKind::JSFunctionKind;
  }
  static bool classof(const GCCell *cell) {
    return kindInRange(
        cell->getKind(),
        CellKind::CodeBlockFunctionKind_first,
        CellKind::CodeBlockFunctionKind_last);
  }

  /// Create a Function with the prototype property set to new Object().
  static PseudoHandle<JSFunction> create(
      Runtime &runtime,
      Handle<Domain> domain,
      Handle<JSObject> parentHandle,
      Handle<Environment> envHandle,
      CodeBlock *codeBlock);

  /// Create a Function with the prototype property set to new Object(). The
  /// parent is inferred by the CodeBlock's function header information.
  static PseudoHandle<JSFunction> createWithInferredParent(
      Runtime &runtime,
      Handle<Domain> domain,
      Handle<Environment> envHandle,
      CodeBlock *codeBlock);

  /// Create a Function with no environment and a CodeBlock simply returning
  /// undefined, with the prototype property auto-initialized to new Object().
  static PseudoHandle<JSFunction> create(
      Runtime &runtime,
      Handle<Domain> domain,
      Handle<JSObject> parentHandle) {
    return create(
        runtime,
        domain,
        parentHandle,
        Runtime::makeNullHandle<Environment>(),
        runtime.getEmptyCodeBlock());
  }

  /// A wrapper to convert a SH-style function to CallResult style.
  static CallResult<HermesValue> _jittedCall(
      JITCompiledFunctionPtr functionPtr,
      Runtime &runtime);

  /// Create a Function with no environment and a CodeBlock simply returning
  /// undefined, with the prototype property auto-initialized to new Object().
  static PseudoHandle<JSFunction> create(
      Runtime &runtime,
      Handle<JSObject> parentHandle) {
    return create(
        runtime, runtime.makeHandle(Domain::create(runtime)), parentHandle);
  }

  /// \return the code block containing the function code.
  CodeBlock *getCodeBlock() const {
    return codeBlock_;
  }

  RuntimeModule *getRuntimeModule() const {
    return getCodeBlock()->getRuntimeModule();
  }

  /// Add a source location for a function in a heap snapshot.
  /// \param snap The snapshot to add a location to.
  /// \param id The object id to annotate with the location.
  void addLocationToSnapshot(
      HeapSnapshot &snap,
      HeapSnapshot::NodeID id,
      GC &gc) const;

  /// A helper to call the interpreter on this code block.
  CallResult<HermesValue> _interpret(Runtime &runtime) {
    return runtime.interpretFunction(codeBlock_);
  }

 protected:
  /// Call the JavaScript function with arguments already on the stack.
  static CallResult<PseudoHandle<>> _callImpl(
      Handle<Callable> selfHandle,
      Runtime &runtime);

#ifdef HERMES_MEMORY_INSTRUMENTATION
  static std::string _snapshotNameImpl(GCCell *cell, GC &gc);
  static void
  _snapshotAddLocationsImpl(GCCell *cell, GC &gc, HeapSnapshot &snap);
  static void _snapshotAddEdgesImpl(GCCell *cell, GC &gc, HeapSnapshot &snap);
#endif
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_CALLABLE_H
