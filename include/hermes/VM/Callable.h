#ifndef HERMES_VM_CALLABLE_H
#define HERMES_VM_CALLABLE_H

#include "hermes/VM/ArrayStorage.h"
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
  uint32_t const size_;

 public:
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
  Environment *getParentEnvironment() const {
    return parentEnvironment_;
  }

  /// \return the number of HermesValue slots in this environment.
  uint32_t getSize() const {
    return size_;
  }

  GCHermesValue &slot(uint32_t index) {
    assert(index < size_ && "invalid Environment slot index");
    return getSlots()[index];
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
        parentEnvironment_(parentEnvironment.get(), &runtime->getHeap()),
        size_(size) {
    // Initialize all slots to 'undefined'.
    GCHermesValue::fill(
        getSlots(), getSlots() + size, HermesValue::encodeUndefinedValue());
  }

  /// \return a pointer to the array of HermesValue.
  GCHermesValue *getSlots() {
    return getTrailingObjects<GCHermesValue>();
  }
  const GCHermesValue *getSlots() const {
    return getTrailingObjects<GCHermesValue>();
  }

  /// Gets the amount of memory required by this object for a given size.
  static uint32_t allocationSize(uint32_t size) {
    return totalSizeToAlloc<GCHermesValue>(size);
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
      Handle<JSObject> protoHandle);

  /// Call the callable with arguments already on the stack.
  CallResult<HermesValue> (
      *call)(Handle<Callable> selfHandle, Runtime *runtime, bool construct);
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
  Environment *getEnvironment() const {
    return environment_.get();
  }

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
  /// \param systemConstructor if true, the prototype property will be defined
  ///   as read-only
  /// \param strictMode the function is a strict mode function; used to
  ///   populate the .arguments and .caller field correctly.
  static ExecutionStatus defineNameLengthAndPrototype(
      Handle<Callable> selfHandle,
      Runtime *runtime,
      SymbolID name,
      unsigned paramCount,
      Handle<JSObject> prototypeObjectHandle,
      bool systemConstructor,
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

  /// Create a new object instance to be passed as the 'this' argument when
  /// invoking the constructor.
  static CallResult<HermesValue> newObject(
      Handle<Callable> selfHandle,
      Runtime *runtime,
      Handle<JSObject> protoHandle) {
    return selfHandle->getVT()->newObject(selfHandle, runtime, protoHandle);
  }

  /// Call the callable with arguments already on the stack.
  /// \param construct true if this is a constructor call.
  static CallResult<HermesValue>
  call(Handle<Callable> selfHandle, Runtime *runtime, bool construct) {
    return selfHandle->getVT()->call(selfHandle, runtime, construct);
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
  static CallResult<double> extractOwnLengthProperty(
      Handle<Callable> selfHandle,
      Runtime *runtime);

  /// Define the length, name, prototype of this function, used when the
  /// creation has been delayed by lazy objects.
  static void defineLazyProperties(Handle<Callable> fn, Runtime *runtime);

 protected:
  Callable(
      Runtime *runtime,
      const VTable *vt,
      JSObject *proto,
      HiddenClass *clazz,
      Handle<Environment> env)
      : JSObject(runtime, vt, proto, clazz),
        environment_(*env, &runtime->getHeap()) {}
  Callable(
      Runtime *runtime,
      const VTable *vt,
      JSObject *proto,
      HiddenClass *clazz)
      : JSObject(runtime, vt, proto, clazz), environment_() {}

  /// Create a an instance of Object to be passed as the 'this' argument when
  /// invoking the constructor.
  static CallResult<HermesValue> _newObjectImpl(
      Handle<Callable> selfHandle,
      Runtime *runtime,
      Handle<JSObject> protoHandle);

 private:
  /// Create an object by calling newObject on \p selfHandle.
  /// The object can then be used as the "this" argument when calling
  /// \p selfHandle to construct an object.
  /// Retrieves the "prototype" property from \p selfHandle,
  /// and calls newObject() on it if it's an object,
  /// else calls newObject() on the built-in Object prototype object.
  static CallResult<HermesValue> createThisForConstruct(
      Handle<Callable> selfHandle,
      Runtime *runtime);
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

  // We need one more slot for '.length'
  static const PropStorage::size_type NEEDED_PROPERTY_SLOTS =
      Super::NEEDED_PROPERTY_SLOTS + 1;

  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::BoundFunctionKind;
  }

  /// \return the target function.
  Callable *getTarget() const {
    return target_;
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
  static CallResult<HermesValue>
  _boundCall(BoundFunction *self, Runtime *runtime, bool construct);

  /// Intialize the length and name and property of a lazily created bound
  /// function.
  static ExecutionStatus initializeLengthAndName(
      Handle<Callable> selfHandle,
      Runtime *runtime,
      Handle<Callable> target,
      unsigned argCount);

  /// \return the number of arguments, including the 'this' param.
  unsigned getArgCountWithThis() const {
    return argStorage_->size();
  }

 private:
  BoundFunction(
      Runtime *runtime,
      JSObject *proto,
      HiddenClass *clazz,
      Handle<Callable> target,
      Handle<ArrayStorage> argStorage)
      : Callable(runtime, &vt.base.base, proto, clazz),
        target_(*target, &runtime->getHeap()),
        argStorage_(*argStorage, &runtime->getHeap()) {}

  /// Return a pointer to the stored arguments, including \c this. \c this is
  /// at index 0, followed by the rest.
  HermesValue *getArgsWithThis() {
    return argStorage_->begin();
  }

  /// Create an instance of the object using the bound constructor.
  static CallResult<HermesValue> _newObjectImpl(
      Handle<Callable> selfHandle,
      Runtime *runtime,
      Handle<JSObject> protoHandle);

  /// Call the callable with arguments already on the stack.
  static CallResult<HermesValue>
  _callImpl(Handle<Callable> selfHandle, Runtime *runtime, bool construct);
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
  NativeFunctionPtr const functionPtr_;

#ifdef HERMESVM_PROFILER_NATIVECALL
  /// How many times the function was called.
  uint32_t callCount_{0};
  /// Total duration of all calls.
  uint64_t callDuration_{0};
#endif

 public:
  using Super = Callable;
  static CallableVTable vt;

  // We need two more slot for '.length' and '.prototype'
  static const PropStorage::size_type NEEDED_PROPERTY_SLOTS =
      Super::NEEDED_PROPERTY_SLOTS + 2;

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
  /// \param construct true if this is a constructor call.
  static CallResult<HermesValue>
  _nativeCall(NativeFunction *self, Runtime *runtime, bool construct) {
    ScopedNativeDepthTracker depthTracker{runtime};
    if (LLVM_UNLIKELY(depthTracker.overflowed())) {
      return runtime->raiseStackOverflow();
    }

    auto newFrame = runtime->setCurrentFrameToTopOfStack();
    // Allocate the "reserved" registers in the new frame.
    runtime->allocStack(
        StackFrameLayout::CalleeExtraRegistersAtStart,
        HermesValue::encodeUndefinedValue());

#ifdef HERMESVM_PROFILER_NATIVECALL
    auto t1 = HERMESVM_RDTSC();
#endif

    auto res = self->functionPtr_(
        self->context_, runtime, newFrame.getNativeArgs(construct));

#ifdef HERMESVM_PROFILER_NATIVECALL
    self->callDuration_ = HERMESVM_RDTSC() - t1;
    ++self->callCount_;
#endif

    runtime->restoreStackAndPreviousFrame(newFrame);
    return res;
  }

  /// Create an instance of NativeFunction.
  /// \param protoHandle object to use as [[Prototype]].
  /// \param context the context to be passed to the function
  /// \param functionPtr the native function
  /// \param paramCount number of parameters (excluding `this`)
  /// \param prototypeObjectHandle if non-null, set as prototype property.
  static Handle<NativeFunction> create(
      Runtime *runtime,
      Handle<JSObject> protoHandle,
      void *context,
      NativeFunctionPtr functionPtr,
      SymbolID name,
      unsigned paramCount,
      Handle<JSObject> prototypeObjectHandle);

  /// Create an instance of NativeFunction.
  /// \param protoHandle object to use as [[Prototype]].
  /// \param parentEnvHandle the parent environment
  /// \param context the context to be passed to the function
  /// \param functionPtr the native function
  /// \param paramCount number of parameters (excluding `this`)
  /// \param prototypeObjectHandle if non-null, set as prototype property.
  static Handle<NativeFunction> create(
      Runtime *runtime,
      Handle<JSObject> protoHandle,
      Handle<Environment> parentEnvHandle,
      void *context,
      NativeFunctionPtr functionPtr,
      SymbolID name,
      unsigned paramCount,
      Handle<JSObject> prototypeObjectHandle);

  /// Create an instance of NativeFunction.
  /// The prototype property will be null.
  /// \param protoHandle object to use as [[Prototype]].
  /// \param context the context to be passed to the function
  /// \param functionPtr the native function
  /// \param paramCount number of parameters (excluding `this`)
  static Handle<NativeFunction> createWithoutPrototype(
      Runtime *runtime,
      Handle<JSObject> protoHandle,
      void *context,
      NativeFunctionPtr functionPtr,
      SymbolID name,
      unsigned paramCount) {
    return create(
        runtime,
        protoHandle,
        context,
        functionPtr,
        name,
        paramCount,
        Handle<JSObject>(runtime));
  }

  /// Create an instance of NativeFunction
  /// The [[Prototype]] will be Function.prototype.
  /// The prototype property wil be null;
  /// \param context the context to be passed to the function
  /// \param functionPtr the native function
  /// \param paramCount number of parameters (excluding `this`)
  static Handle<NativeFunction> createWithoutPrototype(
      Runtime *runtime,
      void *context,
      NativeFunctionPtr functionPtr,
      SymbolID name,
      unsigned paramCount) {
    return createWithoutPrototype(
        runtime,
        Handle<JSObject>::vmcast(&runtime->functionPrototype),
        context,
        functionPtr,
        name,
        paramCount);
  }

 protected:
  NativeFunction(
      Runtime *runtime,
      const VTable *vtp,
      JSObject *proto,
      HiddenClass *clazz,
      void *context,
      NativeFunctionPtr functionPtr)
      : Callable(runtime, vtp, proto, clazz),
        context_(context),
        functionPtr_(functionPtr) {}
  NativeFunction(
      Runtime *runtime,
      const VTable *vtp,
      JSObject *proto,
      HiddenClass *clazz,
      Handle<Environment> environment,
      void *context,
      NativeFunctionPtr functionPtr)
      : Callable(runtime, vtp, proto, clazz, environment),
        context_(context),
        functionPtr_(functionPtr) {}

  /// Call the native function with arguments already on the stack.
  /// \param construct true if this is a constructor call.
  static CallResult<HermesValue>
  _callImpl(Handle<Callable> selfHandle, Runtime *runtime, bool construct);

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
  using CreatorFunction = CallResult<HermesValue>(Runtime *, Handle<JSObject>);
  static const CallableVTable vt;

  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::NativeConstructorKind;
  }

  /// Create an instance of NativeConstructor.
  /// \param context the context to be passed to the function
  /// \param functionPtr the native function
  /// \param paramCount number of parameters (excluding `this`)
  static PseudoHandle<NativeConstructor> create(
      Runtime *runtime,
      Handle<JSObject> protoHandle,
      void *context,
      NativeFunctionPtr functionPtr,
      unsigned paramCount,
      CreatorFunction *creator,
      CellKind targetKind) {
    void *mem = runtime->alloc(sizeof(NativeConstructor));
    return createPseudoHandle(new (mem) NativeConstructor(
        runtime,
        *protoHandle,
        runtime->getHiddenClassForPrototypeRaw(*protoHandle),
        context,
        functionPtr,
        creator,
        targetKind));
  }

  /// Create an instance of NativeConstructor.
  /// \param protoHandle object to use as [[Prototype]].
  /// \param parentEnvHandle the parent environment
  /// \param context the context to be passed to the function
  /// \param functionPtr the native function
  static PseudoHandle<NativeConstructor> create(
      Runtime *runtime,
      Handle<JSObject> protoHandle,
      Handle<Environment> parentEnvHandle,
      void *context,
      NativeFunctionPtr functionPtr,
      CreatorFunction *creator,
      CellKind targetKind) {
    void *mem = runtime->alloc(sizeof(NativeConstructor));
    return createPseudoHandle(new (mem) NativeConstructor(
        runtime,
        *protoHandle,
        runtime->getHiddenClassForPrototypeRaw(*protoHandle),
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
  CreatorFunction *const creator_;

  NativeConstructor(
      Runtime *runtime,
      JSObject *proto,
      HiddenClass *clazz,
      void *context,
      NativeFunctionPtr functionPtr,
      CreatorFunction *creator,
      CellKind targetKind)
      : NativeFunction(
            runtime,
            &vt.base.base,
            proto,
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
      JSObject *proto,
      HiddenClass *clazz,
      Handle<Environment> parentEnvHandle,
      void *context,
      NativeFunctionPtr functionPtr,
      CreatorFunction *creator,
      CellKind targetKind)
      : NativeFunction(
            runtime,
            &vt.base.base,
            proto,
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
      Handle<JSObject> protoHandle) {
    auto nativeConsHandle = Handle<NativeConstructor>::vmcast(selfHandle);
    return nativeConsHandle->creator_(runtime, protoHandle);
  }

#ifndef NDEBUG
  /// If construct=true, check that the constructor was called with a "this"
  /// of the correct type.
  static CallResult<HermesValue>
  _callImpl(Handle<Callable> selfHandle, Runtime *runtime, bool construct);
#endif
};

/// An interpreted callable function with environment.
class JSFunction final : public Callable {
  using Super = Callable;

  /// CodeBlock to execute when called.
  CodeBlock *codeBlock_;

 protected:
  JSFunction(
      Runtime *runtime,
      JSObject *proto,
      HiddenClass *clazz,
      Handle<Environment> environment,
      CodeBlock *codeBlock)
      : Callable(runtime, &vt.base.base, proto, clazz, environment),
        codeBlock_(codeBlock) {
    codeBlock->getRuntimeModule()->addUser();
  }

  ~JSFunction() {
    codeBlock_->getRuntimeModule()->removeUser();
  }

 public:
  static CallableVTable vt;

  // We need two more slot for '.length' and '.prototype'
  static const PropStorage::size_type NEEDED_PROPERTY_SLOTS =
      Super::NEEDED_PROPERTY_SLOTS + 2;

  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::FunctionKind;
  }

  /// Create a Function with the prototype property set to new Object().
  static CallResult<HermesValue> create(
      Runtime *runtime,
      Handle<JSObject> protoHandle,
      Handle<Environment> envHandle,
      CodeBlock *codeBlock);

  /// Create a Function with no environment and a CodeBlock simply returning
  /// undefined, with the prototype property auto-initialized to new Object().
  static CallResult<HermesValue> create(
      Runtime *runtime,
      Handle<JSObject> protoHandle) {
    return create(
        runtime,
        protoHandle,
        runtime->makeNullHandle<Environment>(),
        runtime->getEmptyCodeBlock());
  }

  /// \return the code block containing the function code.
  CodeBlock *getCodeBlock() const {
    return codeBlock_;
  }

  RuntimeModule *getRuntimeModule() const {
    return codeBlock_->getRuntimeModule();
  }

 protected:
  /// Called during GC when an object becomes unreachable. We need it because
  /// CodeBlockPtr_ needs to be destructed.
  static void _finalizeImpl(GCCell *cell, GC *);

  /// Call the JavaScript function with arguments already on the stack.
  /// \param construct true if this is a constructor call.
  static CallResult<HermesValue>
  _callImpl(Handle<Callable> selfHandle, Runtime *runtime, bool construct);
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_CALLABLE_H
