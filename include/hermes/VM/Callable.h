/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

namespace hermes {
namespace vm {

/// Environment storing all escaping variables for a function.
class Environment final
    : public VariableSizeRuntimeCell,
      private llvm::TrailingObjects<Environment, GCHermesValue> {
  friend TrailingObjects;
  friend void EnvironmentBuildMeta(const GCCell *cell, Metadata::Builder &mb);

  /// The parent lexical environment. This value will be nullptr if the
  /// parent is the global scope.
  GCPointer<Environment> parentEnvironment_{};

  /// Number of entries in the environment.
  const uint32_t size_;

 public:
#ifdef HERMESVM_SERIALIZE
  friend void EnvironmentSerialize(Serializer &s, const GCCell *cell);
  friend void EnvironmentDeserialize(Deserializer &d, CellKind kind);
#endif

  static VTable vt;

  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::EnvironmentKind;
  }

  /// Create a new Environment.
  static CallResult<HermesValue> create(
      Runtime *runtime,
      Handle<Environment> parentEnvironment,
      uint32_t size) {
    void *mem = runtime->alloc</*fixedSize*/ false>(allocationSize(size));
    return HermesValue::encodeObjectValue(
        new (mem) Environment(runtime, parentEnvironment, size));
  }

  /// \return the parent lexical environment. This value will be nullptr if the
  /// parent is the global scope.
  Environment *getParentEnvironment(Runtime *runtime) const {
    return parentEnvironment_.get(runtime);
  }

  /// \return the number of HermesValue slots in this environment.
  uint32_t getSize() const {
    return size_;
  }

  GCHermesValue &slot(uint32_t index) {
    assert(index < size_ && "invalid Environment slot index");
    return getSlots()[index];
  }

  /// Gets the amount of memory required by this object for a given size.
  static uint32_t allocationSize(uint32_t size) {
    return totalSizeToAlloc<GCHermesValue>(size);
  }

 private:
  /// \param parentEnvironment the parent lexical environment, or nullptr if the
  ///   parent is the global scope.
  /// \param size the number of entries in the environment.
  Environment(
      Runtime *runtime,
      Handle<Environment> parentEnvironment,
      uint32_t size)
      : VariableSizeRuntimeCell(&runtime->getHeap(), &vt, allocationSize(size)),
        parentEnvironment_(
            runtime,
            parentEnvironment.get(),
            &runtime->getHeap()),
        size_(size) {
    // Initialize all slots to 'undefined'.
    GCHermesValue::fill(
        getSlots(), getSlots() + size, HermesValue::encodeUndefinedValue());
  }

#ifdef HERMESVM_SERIALIZE
  /// Fast constructor used by Deserializer, do not take \p parentEnvironment as
  /// an argument. Don't initialize slots.
  Environment(Runtime *runtime, uint32_t size)
      : VariableSizeRuntimeCell(&runtime->getHeap(), &vt, allocationSize(size)),
        size_(size) {}
#endif

  /// \return a pointer to the array of HermesValue.
  GCHermesValue *getSlots() {
    return getTrailingObjects<GCHermesValue>();
  }
  const GCHermesValue *getSlots() const {
    return getTrailingObjects<GCHermesValue>();
  }
};

struct CallableVTable {
  ObjectVTable base;

  /// Create a new object instance to be passed as the 'this' argument when
  /// invoking the constructor. Overriding this method allows creation of
  /// different underlying native objects.
  CallResult<HermesValue> (*newObject)(
      Handle<Callable> selfHandle,
      Runtime *runtime,
      Handle<JSObject> parentHandle);

  /// Call the callable with arguments already on the stack.
  CallResult<HermesValue> (
      *call)(Handle<Callable> selfHandle, Runtime *runtime);
};

/// The abstract base for callable entities, specifically NativeFunction and
/// Function. It presents the ability to call a function with arguments already
/// on the stack. Subclasses implement this for native funcitions and
/// interpreted functions.
class Callable : public JSObject {
  using Super = JSObject;
  friend void CallableBuildMeta(const GCCell *cell, Metadata::Builder &mb);

  /// Environment containing all captured variables for the function.
  GCPointer<Environment> environment_{};

 public:
#ifdef HERMESVM_SERIALIZE
  /// Fast constructor used by deserializer.
  Callable(Deserializer &d, const VTable *vt);

  friend void serializeCallableImpl(
      Serializer &s,
      const GCCell *cell,
      unsigned overlapSlots);
#endif

  static bool classof(const GCCell *cell) {
    return kindInRange(
        cell->getKind(),
        CellKind::CallableKind_first,
        CellKind::CallableKind_last);
  }

  const CallableVTable *getVT() const {
    return reinterpret_cast<const CallableVTable *>(GCCell::getVT());
  }

  /// \return the environment associated with this callable.
  Environment *getEnvironment(Runtime *runtime) const {
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
  /// \param strictMode the function is a strict mode function; used to
  ///   populate the .arguments and .caller field correctly.
  static ExecutionStatus defineNameLengthAndPrototype(
      Handle<Callable> selfHandle,
      Runtime *runtime,
      SymbolID name,
      unsigned paramCount,
      Handle<JSObject> prototypeObjectHandle,
      WritablePrototype writablePrototype,
      bool strictMode);

  /// Execute this function with no arguments. This is just a convenience
  /// helper method; it actually invokes the interpreter recursively.
  static CallResult<HermesValue> executeCall0(
      Handle<Callable> selfHandle,
      Runtime *runtime,
      Handle<> thisArgHandle,
      bool construct = false);

  /// Execute this function with one argument. This is just a convenience
  /// helper method; it actually invokes the interpreter recursively.
  static CallResult<HermesValue> executeCall1(
      Handle<Callable> selfHandle,
      Runtime *runtime,
      Handle<> thisArgHandle,
      HermesValue param1,
      bool construct = false);

  /// Execute this function with two arguments. This is just a convenience
  /// helper method; it actually invokes the interpreter recursively.
  static CallResult<HermesValue> executeCall2(
      Handle<Callable> selfHandle,
      Runtime *runtime,
      Handle<> thisArgHandle,
      HermesValue param1,
      HermesValue param2,
      bool construct = false);

  /// Execute this function with three arguments. This is just a convenience
  /// helper method; it actually invokes the interpreter recursively.
  static CallResult<HermesValue> executeCall3(
      Handle<Callable> selfHandle,
      Runtime *runtime,
      Handle<> thisArgHandle,
      HermesValue param1,
      HermesValue param2,
      HermesValue param3,
      bool construct = false);

  /// Execute this function with four arguments. This is just a convenience
  /// helper method; it actually invokes the interpreter recursively.
  static CallResult<HermesValue> executeCall4(
      Handle<Callable> selfHandle,
      Runtime *runtime,
      Handle<> thisArgHandle,
      HermesValue param1,
      HermesValue param2,
      HermesValue param3,
      HermesValue param4,
      bool construct = false);

  /// Execute this function with given this and newTarget, and a
  /// variable number of arguments taken from arrayLike.  This invokes
  /// the interpreter recursively.
  static CallResult<HermesValue> executeCall(
      Handle<Callable> selfHandle,
      Runtime *runtime,
      Handle<> newTarget,
      Handle<> thisArgument,
      Handle<JSObject> arrayLike);

  /// Calls CallableVTable::newObject.
  static CallResult<HermesValue> newObject(
      Handle<Callable> selfHandle,
      Runtime *runtime,
      Handle<JSObject> parentHandle) {
    return selfHandle->getVT()->newObject(selfHandle, runtime, parentHandle);
  }

  /// Calls CallableVTable::call.
  static CallResult<HermesValue> call(
      Handle<Callable> selfHandle,
      Runtime *runtime) {
    // Any call to a native or JS function could potentially allocate.
    // Move the heap to force raw pointer errors to come out whenever a call is
    // made.
    runtime->potentiallyMoveHeap();
    return selfHandle->getVT()->call(selfHandle, runtime);
  }

  /// Call the callable in contruct mode with arguments already on the stack.
  /// Checks the return value of the called function. If it is an object, then
  /// it is returned, else the `this` value is returned.
  static CallResult<HermesValue>
  construct(Handle<Callable> selfHandle, Runtime *runtime, Handle<> thisVal) {
    auto result = call(selfHandle, runtime);
    if (LLVM_UNLIKELY(result == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    return result->isObject() ? result : thisVal.getHermesValue();
  }

  /// Create a new object and construct the new object of the given type
  /// by invoking \p selfHandle with construct=true.
  /// \param selfHandle the Callable from which to construct the new object.
  static CallResult<HermesValue> executeConstruct0(
      Handle<Callable> selfHandle,
      Runtime *runtime);

  /// Create a new object and construct the new object of the given type
  /// by invoking \p selfHandle with construct=true.
  /// \param selfHandle the Callable from which to construct the new object.
  /// \param param1 the first argument to the constructor.
  static CallResult<HermesValue> executeConstruct1(
      Handle<Callable> selfHandle,
      Runtime *runtime,
      Handle<> param1);

  /// If the own property ".length" is present and is a number, convert it to
  /// integer and return it. Otherwise return 0. Note that it is not guaranteed
  /// to be non-negative.
  /// Follows ES2018 19.2.3.2 5 and 6.
  static CallResult<double> extractOwnLengthProperty_RJS(
      Handle<Callable> selfHandle,
      Runtime *runtime);

  /// Define the length, name, prototype of this function, used when the
  /// creation has been delayed by lazy objects.
  static void defineLazyProperties(Handle<Callable> fn, Runtime *runtime);

  /// Create an object by calling newObject on \p selfHandle.
  /// The object can then be used as the "this" argument when calling
  /// \p selfHandle to construct an object.
  /// Retrieves the "prototype" property from \p selfHandle,
  /// and calls newObject() on it if it's an object,
  /// else calls newObject() on the built-in Object prototype object.
  static CallResult<HermesValue> createThisForConstruct(
      Handle<Callable> selfHandle,
      Runtime *runtime);

 protected:
  Callable(
      Runtime *runtime,
      const VTable *vt,
      JSObject *parent,
      HiddenClass *clazz,
      Handle<Environment> env)
      : JSObject(runtime, vt, parent, clazz),
        environment_(runtime, *env, &runtime->getHeap()) {}
  Callable(
      Runtime *runtime,
      const VTable *vt,
      JSObject *parent,
      HiddenClass *clazz)
      : JSObject(runtime, vt, parent, clazz), environment_() {}

  static std::string _snapshotNameImpl(GCCell *cell, GC *gc);

  /// Create a an instance of Object to be passed as the 'this' argument when
  /// invoking the constructor.
  static CallResult<HermesValue> _newObjectImpl(
      Handle<Callable> selfHandle,
      Runtime *runtime,
      Handle<JSObject> parentHandle);
};

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
  static CallableVTable vt;

  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::BoundFunctionKind;
  }

  /// \return the target function.
  Callable *getTarget(Runtime *runtime) const {
    return target_.get(runtime);
  }

  /// Creates a BoundFunction instance bound to \p target. It can throw if
  /// obtaining \c target.length throws.
  /// \param argCountWithThis number of arguments, including \c this. It can be
  ///     0 in which case there is no \c this.
  /// \param argsWithThis the arguments, \c this at index 0.
  static CallResult<HermesValue> create(
      Runtime *runtime,
      Handle<Callable> target,
      unsigned argCountWithThis,
      const PinnedHermesValue *argsWithThis);

  /// Perform the actual call. This is a light-weight handler which is part of
  /// the private API - it is only used internally and by the interpreter.
  /// Other users of this class must use \c Callable::call().
  /// \param ip the caller's IP at the point of the call (used for preserving
  /// stack traces).
  static CallResult<HermesValue>
  _boundCall(BoundFunction *self, const Inst *ip, Runtime *runtime);

  /// Intialize the length and name and property of a lazily created bound
  /// function.
  static ExecutionStatus initializeLengthAndName(
      Handle<Callable> selfHandle,
      Runtime *runtime,
      Handle<Callable> target,
      unsigned argCount);

  /// \return the number of arguments, including the 'this' param.
  unsigned getArgCountWithThis(Runtime *runtime) const {
    return argStorage_.get(runtime)->size();
  }

 private:
#ifdef HERMESVM_SERIALIZE
  explicit BoundFunction(Deserializer &d);

  friend void BoundFunctionSerialize(Serializer &s, const GCCell *cell);
  friend void BoundFunctionDeserialize(Deserializer &d, CellKind kind);
#endif

  BoundFunction(
      Runtime *runtime,
      JSObject *parent,
      HiddenClass *clazz,
      Handle<Callable> target,
      Handle<ArrayStorage> argStorage)
      : Callable(runtime, &vt.base.base, parent, clazz),
        target_(runtime, *target, &runtime->getHeap()),
        argStorage_(runtime, *argStorage, &runtime->getHeap()) {}

  /// Return a pointer to the stored arguments, including \c this. \c this is
  /// at index 0, followed by the rest.
  GCHermesValue *getArgsWithThis(Runtime *runtime) {
    return argStorage_.get(runtime)->begin();
  }

  /// Create an instance of the object using the bound constructor.
  static CallResult<HermesValue> _newObjectImpl(
      Handle<Callable> selfHandle,
      Runtime *runtime,
      Handle<JSObject> parentHandle);

  /// Call the callable with arguments already on the stack.
  static CallResult<HermesValue> _callImpl(
      Handle<Callable> selfHandle,
      Runtime *runtime);
};

/// A pointer to native function.
typedef CallResult<HermesValue> (
    *NativeFunctionPtr)(void *context, Runtime *runtime, NativeArgs args);

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
#ifdef HERMESVM_SERIALIZE
  NativeFunction(
      Deserializer &d,
      const VTable *vt,
      void *context,
      NativeFunctionPtr functionPtr);

  static void serializeNativeFunctionImpl(
      Serializer &s,
      const GCCell *cell,
      unsigned overlapSlots);
  friend void NativeFunctionSerialize(Serializer &s, const GCCell *cell);
#endif

  using Super = Callable;
  static CallableVTable vt;

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
  static CallResult<HermesValue> _nativeCall(
      NativeFunction *self,
      Runtime *runtime) {
    ScopedNativeDepthTracker depthTracker{runtime};
    if (LLVM_UNLIKELY(depthTracker.overflowed())) {
      return runtime->raiseStackOverflow(
          Runtime::StackOverflowKind::NativeStack);
    }

    auto newFrame = runtime->setCurrentFrameToTopOfStack();
    runtime->saveCallerIPInStackFrame();
    // Allocate the "reserved" registers in the new frame.
    runtime->allocStack(
        StackFrameLayout::CalleeExtraRegistersAtStart,
        HermesValue::encodeUndefinedValue());

#ifdef HERMESVM_PROFILER_NATIVECALL
    auto t1 = HERMESVM_RDTSC();
#endif

    auto res =
        self->functionPtr_(self->context_, runtime, newFrame.getNativeArgs());

#ifdef HERMESVM_PROFILER_NATIVECALL
    self->callDuration_ = HERMESVM_RDTSC() - t1;
    ++self->callCount_;
#endif
    runtime->restoreStackAndPreviousFrame(newFrame);
    return res;
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
      Runtime *runtime,
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
      Runtime *runtime,
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
      Runtime *runtime,
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
        Handle<JSObject>(runtime),
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
      Runtime *runtime,
      void *context,
      NativeFunctionPtr functionPtr,
      SymbolID name,
      unsigned paramCount,
      unsigned additionalSlotCount = 0) {
    return createWithoutPrototype(
        runtime,
        Handle<JSObject>::vmcast(&runtime->functionPrototype),
        context,
        functionPtr,
        name,
        paramCount,
        additionalSlotCount);
  }

  /// \return the value in an additional slot.
  /// \param index must be less than the \c additionalSlotCount passed to
  /// the create method.
  static HermesValue getAdditionalSlotValue(
      NativeFunction *self,
      Runtime *runtime,
      unsigned index) {
    return JSObject::getInternalProperty(
        self,
        runtime,
        numOverlapSlots<NativeFunction>() + ANONYMOUS_PROPERTY_SLOTS + index);
  }

  /// Set the value in an additional slot.
  /// \param index must be less than the \c additionalSlotCount passed to
  /// the create method.
  static void setAdditionalSlotValue(
      JSObject *self,
      Runtime *runtime,
      unsigned index,
      HermesValue value) {
    return JSObject::setInternalProperty(
        self,
        runtime,
        numOverlapSlots<NativeFunction>() + ANONYMOUS_PROPERTY_SLOTS + index,
        value);
  }

 protected:
  NativeFunction(
      Runtime *runtime,
      const VTable *vtp,
      JSObject *parent,
      HiddenClass *clazz,
      void *context,
      NativeFunctionPtr functionPtr)
      : Callable(runtime, vtp, parent, clazz),
        context_(context),
        functionPtr_(functionPtr) {}
  NativeFunction(
      Runtime *runtime,
      const VTable *vtp,
      JSObject *parent,
      HiddenClass *clazz,
      Handle<Environment> environment,
      void *context,
      NativeFunctionPtr functionPtr)
      : Callable(runtime, vtp, parent, clazz, environment),
        context_(context),
        functionPtr_(functionPtr) {}

  static std::string _snapshotNameImpl(GCCell *cell, GC *gc);

  /// Call the native function with arguments already on the stack.
  static CallResult<HermesValue> _callImpl(
      Handle<Callable> selfHandle,
      Runtime *runtime);

  /// We have to override this method because NativeFunction should not be
  /// used as constructor.
  /// Note: this may change in the future, in that case, we should create
  /// a subclass of NativeFunction for this restriction.
  static CallResult<HermesValue>
  _newObjectImpl(Handle<Callable>, Runtime *runtime, Handle<JSObject>);
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
      Runtime *,
      Handle<JSObject> proto,
      void *context);

  /// Unifies signatures of various GCCells so that they may be stored
  /// in the NativeConstructor.
  /// Delegates to toCallResultPseudoHandleJSObject to convert various
  /// types to CallResult<PseudoHandle<JSObject>>.
  template <class NativeClass>
  static CallResult<PseudoHandle<JSObject>>
  creatorFunction(Runtime *runtime, Handle<JSObject> prototype, void *context) {
    (void)context;
    return toCallResultPseudoHandleJSObject(
        NativeClass::create(runtime, prototype));
  }

#ifdef HERMESVM_SERIALIZE
  NativeConstructor(
      Deserializer &d,
      void *context,
      NativeFunctionPtr functionPtr,
      CellKind targetKind,
      CreatorFunction *creatorFunction);

  friend void NativeConstructorSerialize(Serializer &s, const GCCell *cell);
#endif

  static const CallableVTable vt;

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
      Runtime *runtime,
      Handle<JSObject> parentHandle,
      void *context,
      NativeFunctionPtr functionPtr,
      unsigned paramCount,
      CreatorFunction *creator,
      CellKind targetKind) {
    void *mem = runtime->alloc(cellSize<NativeConstructor>());
    return createPseudoHandle(new (mem) NativeConstructor(
        runtime,
        *parentHandle,
        runtime->getHiddenClassForPrototypeRaw(
            *parentHandle,
            numOverlapSlots<NativeConstructor>() + ANONYMOUS_PROPERTY_SLOTS),
        context,
        functionPtr,
        creator,
        targetKind));
  }

  /// Create an instance of NativeConstructor.
  /// \param parentHandle object to use as [[Prototype]].
  /// \param parentEnvHandle the parent environment
  /// \param context the context to be passed to the function
  /// \param functionPtr the native function
  static PseudoHandle<NativeConstructor> create(
      Runtime *runtime,
      Handle<JSObject> parentHandle,
      Handle<Environment> parentEnvHandle,
      void *context,
      NativeFunctionPtr functionPtr,
      CreatorFunction *creator,
      CellKind targetKind) {
    void *mem = runtime->alloc(cellSize<NativeConstructor>());
    return createPseudoHandle(new (mem) NativeConstructor(
        runtime,
        *parentHandle,
        runtime->getHiddenClassForPrototypeRaw(
            *parentHandle,
            numOverlapSlots<NativeConstructor>() + ANONYMOUS_PROPERTY_SLOTS),
        parentEnvHandle,
        context,
        functionPtr,
        creator,
        targetKind));
  }

 private:
#ifndef NDEBUG
  /// Kind of the object returned by this native constructor.
  const CellKind targetKind_;
#endif

  /// Function used to create new object from this constructor.
  /// Typically passed by invoking NativeConstructor::creatorFunction<T>.
  CreatorFunction *const creator_;

  NativeConstructor(
      Runtime *runtime,
      JSObject *parent,
      HiddenClass *clazz,
      void *context,
      NativeFunctionPtr functionPtr,
      CreatorFunction *creator,
      CellKind targetKind)
      : NativeFunction(
            runtime,
            &vt.base.base,
            parent,
            clazz,
            context,
            functionPtr),
#ifndef NDEBUG
        targetKind_(targetKind),
#endif
        creator_(creator) {
  }

  NativeConstructor(
      Runtime *runtime,
      JSObject *parent,
      HiddenClass *clazz,
      Handle<Environment> parentEnvHandle,
      void *context,
      NativeFunctionPtr functionPtr,
      CreatorFunction *creator,
      CellKind targetKind)
      : NativeFunction(
            runtime,
            &vt.base.base,
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

  /// Create a an instance of an object from \c creator_ to be passed as the
  /// 'this' argument when invoking the constructor.
  static CallResult<HermesValue> _newObjectImpl(
      Handle<Callable> selfHandle,
      Runtime *runtime,
      Handle<JSObject> parentHandle) {
    auto nativeConsHandle = Handle<NativeConstructor>::vmcast(selfHandle);
    auto res = nativeConsHandle->creator_(
        runtime, parentHandle, nativeConsHandle->getContext());
    if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    return res->getHermesValue();
  }

#ifndef NDEBUG
  /// If construct=true, check that the constructor was called with a "this"
  /// of the correct type.
  static CallResult<HermesValue> _callImpl(
      Handle<Callable> selfHandle,
      Runtime *runtime);
#endif

  template <class From>
  static CallResult<PseudoHandle<JSObject>> toCallResultPseudoHandleJSObject(
      PseudoHandle<From> &&other) {
    return PseudoHandle<JSObject>{std::move(other)};
  }

  template <class From>
  static CallResult<PseudoHandle<JSObject>> toCallResultPseudoHandleJSObject(
      CallResult<PseudoHandle<From>> &&other) {
    return std::move(other);
  }

  template <class From>
  static CallResult<PseudoHandle<JSObject>> toCallResultPseudoHandleJSObject(
      CallResult<Handle<From>> other) {
    if (LLVM_UNLIKELY(other == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    return PseudoHandle<JSObject>{*other};
  }

  template <class From>
  static CallResult<PseudoHandle<JSObject>> toCallResultPseudoHandleJSObject(
      Handle<From> other) {
    return PseudoHandle<JSObject>{other};
  }
};

/// An interpreted callable function with environment.
class JSFunction : public Callable {
  using Super = Callable;
  friend void FunctionBuildMeta(const GCCell *cell, Metadata::Builder &mb);

  /// CodeBlock to execute when called.
  CodeBlock *codeBlock_;

  static constexpr auto kHasFinalizer = HasFinalizer::No;

  /// JSFunctions must keep their domains alive.
  GCPointer<Domain> domain_;

 protected:
#ifdef HERMESVM_SERIALIZE
  JSFunction(Deserializer &d, const VTable *vt);

  friend void serializeFunctionImpl(
      Serializer &s,
      const GCCell *cell,
      unsigned overlapSlots);
  friend void FunctionDeserialize(Deserializer &d, CellKind kind);
#endif

  JSFunction(
      Runtime *runtime,
      const VTable *vtp,
      Domain *domain,
      JSObject *parent,
      HiddenClass *clazz,
      Handle<Environment> environment,
      CodeBlock *codeBlock)
      : Callable(runtime, vtp, parent, clazz, environment),
        codeBlock_(codeBlock),
        domain_(runtime, domain, &runtime->getHeap()) {
    assert(
        !vt.base.base.finalize_ == (kHasFinalizer != HasFinalizer::Yes) &&
        "kHasFinalizer invalid value");
  }

  JSFunction(
      Runtime *runtime,
      Domain *domain,
      JSObject *parent,
      HiddenClass *clazz,
      Handle<Environment> environment,
      CodeBlock *codeBlock)
      : JSFunction(
            runtime,
            &vt.base.base,
            domain,
            parent,
            clazz,
            environment,
            codeBlock) {}

 public:
  static CallableVTable vt;

  static bool classof(const GCCell *cell) {
    return kindInRange(
        cell->getKind(),
        CellKind::CodeBlockFunctionKind_first,
        CellKind::CodeBlockFunctionKind_last);
  }

  /// Create a Function with the prototype property set to new Object().
  static PseudoHandle<JSFunction> create(
      Runtime *runtime,
      Handle<Domain> domain,
      Handle<JSObject> parentHandle,
      Handle<Environment> envHandle,
      CodeBlock *codeBlock);

  /// Create a Function with no environment and a CodeBlock simply returning
  /// undefined, with the prototype property auto-initialized to new Object().
  static PseudoHandle<JSFunction> create(
      Runtime *runtime,
      Handle<Domain> domain,
      Handle<JSObject> parentHandle) {
    return create(
        runtime,
        domain,
        parentHandle,
        Runtime::makeNullHandle<Environment>(),
        runtime->getEmptyCodeBlock());
  }

  /// Create a Function with no environment and a CodeBlock simply returning
  /// undefined, with the prototype property auto-initialized to new Object().
  static PseudoHandle<JSFunction> create(
      Runtime *runtime,
      Handle<JSObject> parentHandle) {
    return create(
        runtime, runtime->makeHandle(Domain::create(runtime)), parentHandle);
  }

  /// \return the code block containing the function code.
  CodeBlock *getCodeBlock() const {
    return codeBlock_;
  }

  RuntimeModule *getRuntimeModule() const {
    return codeBlock_->getRuntimeModule();
  }

  /// Add a source location for a function in a heap snapshot.
  /// \param snap The snapshot to add a location to.
  /// \param id The object id to annotate with the location.
  void addLocationToSnapshot(HeapSnapshot &snap, HeapSnapshot::NodeID id) const;

 protected:
  /// Call the JavaScript function with arguments already on the stack.
  static CallResult<HermesValue> _callImpl(
      Handle<Callable> selfHandle,
      Runtime *runtime);

  static std::string _snapshotNameImpl(GCCell *cell, GC *gc);
  static void
  _snapshotAddLocationsImpl(GCCell *cell, GC *gc, HeapSnapshot &snap);
};

/// A function which interprets code and returns a Generator when called.
/// Needs a separate class because it must be a different CellKind from
/// JSFunction.
class JSGeneratorFunction final : public JSFunction {
  using Super = JSFunction;

  static constexpr auto kHasFinalizer = HasFinalizer::No;

 public:
  static CallableVTable vt;

  /// Create a GeneratorFunction.
  static PseudoHandle<JSGeneratorFunction> create(
      Runtime *runtime,
      Handle<Domain> domain,
      Handle<JSObject> parentHandle,
      Handle<Environment> envHandle,
      CodeBlock *codeBlock);

  /// Create a GeneratorFunction with no environment and a CodeBlock simply
  /// returning undefined, with the prototype property auto-initialized to new
  /// Object().
  static PseudoHandle<JSGeneratorFunction> create(
      Runtime *runtime,
      Handle<JSObject> parentHandle) {
    return create(
        runtime,
        runtime->makeHandle(Domain::create(runtime)),
        parentHandle,
        runtime->makeNullHandle<Environment>(),
        runtime->getEmptyCodeBlock());
  }

  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::GeneratorFunctionKind;
  }

 protected:
#ifdef HERMESVM_SERIALIZE
  explicit JSGeneratorFunction(Deserializer &d);

  friend void GeneratorFunctionDeserialize(Deserializer &d, CellKind kind);
#endif

  JSGeneratorFunction(
      Runtime *runtime,
      const VTable *vtp,
      Domain *domain,
      JSObject *parent,
      HiddenClass *clazz,
      Handle<Environment> environment,
      CodeBlock *codeBlock)
      : Super(runtime, vtp, domain, parent, clazz, environment, codeBlock) {
    assert(
        !vt.base.base.finalize_ == (kHasFinalizer != HasFinalizer::Yes) &&
        "kHasFinalizer invalid value");
  }

  JSGeneratorFunction(
      Runtime *runtime,
      Domain *domain,
      JSObject *parent,
      HiddenClass *clazz,
      Handle<Environment> environment,
      CodeBlock *codeBlock)
      : JSFunction(
            runtime,
            &vt.base.base,
            domain,
            parent,
            clazz,
            environment,
            codeBlock) {}
};

/// A function which can save its state and yield execution to the caller.
/// Wrapped by Generators which are returned from JSGeneratorFunctions.
/// This class stores:
/// - `state_`: Current state of generator execution (to ensure we don't
///   try to reenter a generator while it's executing, and to ensure we
///   handle completion conditions properly).
/// - `nextIPOffset_`: The offset into the CodeBlock for the IP at which to
///   resume execution, which `StartGenerator` will jump to.
/// - `action_`: The next requested user action, to be read by
///   `ResumeGenerator`.
/// - `result`: The result passed to, e.g. `next()`, by the user.
///   Placed in the result register of `ResumeGenerator`.
/// - `savedContext`: Keeps the arguments passed initially to the
///   `GeneratorFunction` as well as the saved frame variables.
///   The arguments are restored during the _callImpl, and the saved frame
///   is restored when restoreStack is called.
/// - `argCount`: The number of arguments provided to the outer function when
///   creating the generator, used when restoring the context.
class GeneratorInnerFunction final : public JSFunction {
  using Super = JSFunction;
  friend void GeneratorInnerFunctionBuildMeta(
      const GCCell *cell,
      Metadata::Builder &mb);

  static constexpr auto kHasFinalizer = HasFinalizer::No;

 public:
  static CallableVTable vt;

  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::GeneratorInnerFunctionKind;
  }

  /// Represent the  internal slot.
  /// ES6.0 25.3.2.
  enum class State {
    SuspendedStart,
    SuspendedYield,
    Executing,
    Completed,
  };

  /// Represent the action requested on resume: next, throw, or return.
  enum class Action {
    Next,
    Throw,
    Return,
  };

  static CallResult<Handle<GeneratorInnerFunction>> create(
      Runtime *runtime,
      Handle<Domain> domain,
      Handle<JSObject> parentHandle,
      Handle<Environment> envHandle,
      CodeBlock *codeBlock,
      NativeArgs args);

  /// Call the GeneratorInnerFunction with arguments already on the stack.
  /// This is the only means by which the inner function code should be invoked.
  /// \param arg the user-provided argument, assigned to the result_ field.
  /// \param action the user-requested action, assigned to the action_ field.
  /// Restores the stored arguments before actually calling the wrapped
  /// function.
  static CallResult<HermesValue> callInnerFunction(
      Handle<GeneratorInnerFunction> selfHandle,
      Runtime *runtime,
      Handle<> arg,
      Action action);

  /// GeneratorInnerFunction must not be called through _callImpl, but rather
  /// directly by VM code only.
  static CallResult<HermesValue> _callImpl(
      Handle<Callable> selfHandle,
      Runtime *runtime) {
    return runtime->raiseTypeError(
        "Generator inner functions may not be called directly by user code");
  }

  void setState(State state) {
    assert(
        state_ != State::Completed && "Cannot leave completed generator state");
    state_ = state;
  }

  State getState() const {
    return state_;
  }

  void setAction(Action action) {
    action_ = action;
  }

  Action getAction() const {
    return action_;
  }

  /// Clear the stored result_ field to prevent memory leaks.
  /// Should be called after getResult() by the ResumeGenerator instruction.
  void clearResult() {
    result_.setNonPtr(HermesValue::encodeEmptyValue());
  }

  HermesValue getResult() const {
    return result_;
  }

  bool isDelegated() const {
    return isDelegated_;
  }

  void setIsDelegated(bool isDelegated) {
    isDelegated_ = isDelegated;
  }

  /// Restores the stack variables needed to resume execution from a
  /// SuspendedYield state.
  void restoreStack(Runtime *runtime);

  /// Saves the stack variables needed to resume execution from a SuspendedYield
  /// state, and places them in an internal property.
  void saveStack(Runtime *runtime);

  void setNextIP(const Inst *ip) {
    nextIPOffset_ = getCodeBlock()->getOffsetOf(ip);
  }

  const Inst *getNextIP() const {
    return getCodeBlock()->getOffsetPtr(nextIPOffset_);
  }

 protected:
#ifdef HERMESVM_SERIALIZE
  explicit GeneratorInnerFunction(Deserializer &d);

  friend void GeneratorInnerFunctionSerialize(
      Serializer &s,
      const GCCell *cell);
  friend void GeneratorInnerFunctionDeserialize(Deserializer &d, CellKind kind);
#endif

  GeneratorInnerFunction(
      Runtime *runtime,
      Domain *domain,
      JSObject *parent,
      HiddenClass *clazz,
      Handle<Environment> environment,
      CodeBlock *codeBlock,
      uint32_t argCount)
      : JSFunction(
            runtime,
            &vt.base.base,
            domain,
            parent,
            clazz,
            environment,
            codeBlock),
        argCount_(argCount) {
    assert(
        !vt.base.base.finalize_ == (kHasFinalizer != HasFinalizer::Yes) &&
        "kHasFinalizer invalid value");
  }

 private:
  /// The current state of the generator.
  State state_{State::SuspendedStart};

  /// The number of arguments provided to the outer function when creating the
  /// generator.
  /// Note: This does NOT include the `this` argument.
  uint32_t argCount_;

  /// Keeps the arguments passed initially to the `GeneratorFunction` as well as
  /// the saved frame variables. The arguments are restored during the
  /// _callImpl, and the saved frame is restored when restoreStack is called.
  GCPointer<ArrayStorage> savedContext_{nullptr};

  /// The result passed to `next()`, `throw()`, or `return()` by the user.
  /// Placed in the result register of `ResumeGenerator`.
  GCHermesValue result_{};

  /// The next instruction to jump to upon resuming from SuspendedYield,
  /// invalid if the generator is in any other state.
  /// Saved as uint32_t instead of Inst * in order to save memory.
  uint32_t nextIPOffset_{0};

  /// The action requested by the user by the way the generator is invoked.
  Action action_;

  /// If true, currently running a yield* expression, and results of yielding
  /// should not be rewrapped using createIterResultObject.
  bool isDelegated_{false};

 private:
  /// \return the offset of the frame registers in the stored context.
  uint32_t getFrameOffsetInContext() const {
    // Account for `this` argument.
    return 1 + argCount_;
  }

  /// \return the number of frame registers in the stored context.
  uint32_t getFrameSizeInContext(Runtime *runtime) const {
    uint32_t frameOffset = getFrameOffsetInContext();
    return savedContext_.get(runtime)->size() - frameOffset;
  }
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_CALLABLE_H
