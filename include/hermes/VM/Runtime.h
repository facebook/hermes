/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_RUNTIME_H
#define HERMES_VM_RUNTIME_H

#include "hermes/Public/DebuggerTypes.h"
#include "hermes/Public/RuntimeConfig.h"
#include "hermes/Support/Compiler.h"
#include "hermes/Support/ErrorHandling.h"
#include "hermes/VM/AllocOptions.h"
#include "hermes/VM/AllocResult.h"
#include "hermes/VM/BasicBlockExecutionInfo.h"
#include "hermes/VM/CallResult.h"
#include "hermes/VM/Casting.h"
#include "hermes/VM/Debugger/Debugger.h"
#include "hermes/VM/Deserializer.h"
#include "hermes/VM/GC.h"
#include "hermes/VM/Handle-inline.h"
#include "hermes/VM/HandleRootOwner-inline.h"
#include "hermes/VM/IdentifierTable.h"
#include "hermes/VM/InternalProperty.h"
#include "hermes/VM/InterpreterState.h"
#include "hermes/VM/JIT/JIT.h"
#include "hermes/VM/PointerBase.h"
#include "hermes/VM/Predefined.h"
#include "hermes/VM/Profiler.h"
#include "hermes/VM/PropertyCache.h"
#include "hermes/VM/PropertyDescriptor.h"
#include "hermes/VM/RegExpMatch.h"
#include "hermes/VM/RuntimeModule.h"
#include "hermes/VM/RuntimeStats.h"
#include "hermes/VM/Serializer.h"
#include "hermes/VM/StackFrame.h"
#include "hermes/VM/StackTracesTree.h"
#include "hermes/VM/SymbolRegistry.h"
#include "hermes/VM/TwineChar16.h"

#ifdef HERMESVM_PROFILER_BB
#include "hermes/VM/Profiler/InlineCacheProfiler.h"
#endif

#include "llvh/ADT/DenseMap.h"
#include "llvh/ADT/SmallVector.h"

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <type_traits>
#include <vector>

namespace hermes {
// Forward declaration.
class JSONEmitter;

namespace inst {
struct Inst;
}

namespace hbc {
class BytecodeModule;
struct CompileFlags;
} // namespace hbc

namespace vm {

// External forward declarations.
class CodeBlock;
class ArrayStorage;
class Environment;
class Interpreter;
class JSObject;
class PropertyAccessor;
struct RuntimeCommonStorage;
struct RuntimeOffsets;
class ScopedNativeDepthReducer;
class ScopedNativeDepthTracker;
class ScopedNativeCallFrame;
class SamplingProfiler;
class CodeCoverageProfiler;
struct MockedEnvironment;

#ifdef HERMESVM_PROFILER_BB
class JSArray;
#endif

/// Number of stack words after the top of frame that we always ensure are
/// available. This is necessary so we can perform native calls with small
/// number of arguments without checking.
static const unsigned STACK_RESERVE = 32;

/// List of active experiments, corresponding to getVMExperimentFlags().
namespace experiments {
enum {
  Default = 0,
  MAdviseSequential = 1 << 2,
  MAdviseRandom = 1 << 3,
  MAdviseStringsSequential = 1 << 4,
  MAdviseStringsRandom = 1 << 5,
  MAdviseStringsWillNeed = 1 << 6,
  VerifyBytecodeChecksum = 1 << 7,
};
/// Set of flags for active VM experiments.
using VMExperimentFlags = uint32_t;
} // namespace experiments

/// Type used to assign object unique integer identifiers.
using ObjectID = uint32_t;

using DestructionCallback = std::function<void(Runtime *)>;

#define PROP_CACHE_IDS(V) V(RegExpLastIndex, Predefined::lastIndex)

/// Fixed set of ids used by the property cache in Runtime.
enum class PropCacheID {
#define V(id, predef) id,
  PROP_CACHE_IDS(V)
#undef V
      _COUNT
};

/// The Runtime encapsulates the entire context of a VM. Multiple instances can
/// exist and are completely independent from each other.
class Runtime : public HandleRootOwner,
                public PointerBase,
                private GCBase::GCCallbacks {
 public:
  static std::shared_ptr<Runtime> create(const RuntimeConfig &runtimeConfig);

  ~Runtime();

  /// Add a custom function that will be executed at the start of every garbage
  /// collection to mark additional GC roots that may not be known to the
  /// Runtime.
  void addCustomRootsFunction(
      std::function<void(GC *, SlotAcceptor &)> markRootsFn);

  /// Add a custom function that will be executed sometime during garbage
  /// collection to mark additional weak GC roots that may not be known to the
  /// Runtime.
  void addCustomWeakRootsFunction(
      std::function<void(GC *, WeakRefAcceptor &)> markRootsFn);

  /// Make the runtime read from \p env to replay its environment-dependent
  /// behavior.
  void setMockedEnvironment(const MockedEnvironment &env);

  /// Runs the given UTF-8 \p code in a new RuntimeModule as top-level code.
  /// Note that if compileFlags.lazy is set, the code string will be copied.
  /// \param sourceURL the location of the source that's being run.
  /// \param compileFlags Flags controlling compilation.
  /// \return the status of the execution.
  CallResult<HermesValue> run(
      llvh::StringRef code,
      llvh::StringRef sourceURL,
      hbc::CompileFlags compileFlags);

  /// Runs the given UTF-8 \p code in a new RuntimeModule as top-level code.
  /// \param sourceURL the location of the source that's being run.
  /// \param compileFlags Flags controlling compilation.
  /// \return the status of the execution.
  CallResult<HermesValue> run(
      std::unique_ptr<Buffer> code,
      llvh::StringRef sourceURL,
      hbc::CompileFlags compileFlags);

  /// Runs the given \p bytecode with the given \p runtimeModuleFlags. The \p
  /// sourceURL, if not empty, is reported as the file name in backtraces. If \p
  /// environment is not null, set it as the environment associated with the
  /// initial JSFunction, which enables local eval. \p thisArg the "this"
  /// argument to use initially. \p isPersistent indicates whether the created
  /// runtime module should persist in memory.
  CallResult<HermesValue> runBytecode(
      std::shared_ptr<hbc::BCProvider> &&bytecode,
      RuntimeModuleFlags runtimeModuleFlags,
      llvh::StringRef sourceURL,
      Handle<Environment> environment,
      Handle<> thisArg);

  /// Runs the given \p bytecode. If \p environment is not null, set it as the
  /// environment associated with the initial JSFunction, which enables local
  /// eval.
  /// Uses global_ as the "this" value initially.
  /// \p isPersistent indicates whether the created runtime module should
  /// persist in memory.
  CallResult<HermesValue> runBytecode(
      std::shared_ptr<hbc::BCProvider> &&bytecode,
      RuntimeModuleFlags runtimeModuleFlags,
      llvh::StringRef sourceURL,
      Handle<Environment> environment) {
    heap_.runtimeWillExecute();
    return runBytecode(
        std::move(bytecode),
        runtimeModuleFlags,
        sourceURL,
        environment,
        Handle<>(&global_));
  }

  ExecutionStatus loadSegment(
      std::shared_ptr<hbc::BCProvider> &&bytecode,
      Handle<RequireContext> requireContext,
      RuntimeModuleFlags flags = {});

  /// Runs the internal bytecode. This is called once during initialization.
  void runInternalBytecode();

  /// A convenience function to print an exception to a stream.
  void printException(llvh::raw_ostream &os, Handle<> valueHandle);

  /// @name Heap management
  /// @{

  /// Allocate a new cell of the specified size \p size.
  /// If necessary perform a GC cycle, which may potentially move allocated
  /// objects.
  /// The \p fixedSize template argument indicates whether the allocation is for
  /// a fixed-size cell, which can assumed to be small if true.  The
  /// \p hasFinalizer template argument indicates whether the object
  /// being allocated will have a finalizer.
  template <bool fixedSize = true, HasFinalizer hasFinalizer = HasFinalizer::No>
  void *alloc(uint32_t size);

  /// Create a fixed size object of type T.
  /// If necessary perform a GC cycle, which may potentially move
  /// allocated objects.
  /// \return a pointer to the newly created object in the GC heap.
  template <
      typename T,
      HasFinalizer hasFinalizer = HasFinalizer::No,
      LongLived longLived = LongLived::No,
      class... Args>
  T *makeAFixed(Args &&... args);

  /// Create a variable size object of type T and size \p size.
  /// If necessary perform a GC cycle, which may potentially move
  /// allocated objects.
  /// \return a pointer to the newly created object in the GC heap.
  template <
      typename T,
      HasFinalizer hasFinalizer = HasFinalizer::No,
      LongLived longLived = LongLived::No,
      class... Args>
  T *makeAVariable(uint32_t size, Args &&... args);

  /// Used as a placeholder for places where we should be checking for OOM
  /// but aren't yet.
  /// TODO: do something when there is an uncaught exception, e.g. print
  /// stack traces.
  template <typename T>
  T ignoreAllocationFailure(CallResult<T> res);

  /// Used as a placeholder for places where we should be checking for OOM
  /// but aren't yet.
  void ignoreAllocationFailure(ExecutionStatus status);

  // Inform the VM that TTI has been reached.  (In case, for example, the
  // runtime should change its behavior at that point.)
  void ttiReached();

  /// Force a garbage collection cycle.
  void collect(std::string cause) {
    heap_.collect(std::move(cause));
  }

  /// Potentially move the heap if handle sanitization is on.
  void potentiallyMoveHeap();

  using HandleRootOwner::makeHandle;
  using HandleRootOwner::makeMutableHandle;

  /// Convenience function to create a Handle from a GCPointer.
  template <class T>
  inline Handle<T> makeHandle(const GCPointer<T> &p);

  /// Convenience function to create a MutableHandle from a GCPointer.
  template <class T>
  MutableHandle<T> makeMutableHandle(const GCPointer<T> &p);

  /// \return the \c StringPrimitive of a predefined string.
  StringPrimitive *getPredefinedString(Predefined::Str predefined);
  StringPrimitive *getPredefinedString(Predefined::Sym predefined);

  /// \return a \c Handle<StringPrimitive> to a predefined string.
  Handle<StringPrimitive> getPredefinedStringHandle(Predefined::Str predefined);
  Handle<StringPrimitive> getPredefinedStringHandle(Predefined::Sym predefined);

  /// \return the \c StringPrimitive given a symbol ID \p id.
  inline StringPrimitive *getStringPrimFromSymbolID(SymbolID id);

  /// \return true if a symbol specified by \p id has the same string content
  /// as a given string primitive \p strPrim.
  bool symbolEqualsToStringPrim(SymbolID id, StringPrimitive *strPrim);

  /// A wrapper to facilitate printing the name of a SymbolID to a stream.
  struct FormatSymbolID {
    Runtime *const runtime;
    SymbolID const symbolID;

    FormatSymbolID(Runtime *runtime, SymbolID symbolID)
        : runtime(runtime), symbolID(symbolID) {}
  };

  /// Create an object that when serialized to a stream will print the contents
  /// of a SymbolID.
  FormatSymbolID formatSymbolID(SymbolID id);

  GC &getHeap() {
    return heap_;
  }

  /// @}

  /// Return a pointer to a builtin native function builtin identified by id.
  /// Unfortunately we can't use the enum here, since we don't want to include
  /// the builtins header header.
  inline NativeFunction *getBuiltinNativeFunction(unsigned builtinMethodID);

  IdentifierTable &getIdentifierTable() {
    return identifierTable_;
  }

  SymbolRegistry &getSymbolRegistry() {
    return symbolRegistry_;
  }

  /// Return a StringPrimitive representation of a single character. The first
  /// 256 characters are pre-allocated. The rest are allocated every time.
  Handle<StringPrimitive> getCharacterString(char16_t ch);

  CodeBlock *getEmptyCodeBlock() const {
    assert(emptyCodeBlock_ && "Invalid empty code block");
    return emptyCodeBlock_;
  }

  CodeBlock *getReturnThisCodeBlock() const {
    assert(returnThisCodeBlock_ && "Invalid return this code block");
    return returnThisCodeBlock_;
  }

  /// \return the next unique object ID.
  ObjectID generateNextObjectID() {
    return ++nextObjectID_;
  }

  /// Compute a hash value of a given HermesValue that is guaranteed to
  /// be stable with a moving GC. It however does not guarantee to be
  /// a perfect hash for strings.
  uint64_t gcStableHashHermesValue(Handle<HermesValue> value);

  /// @name Public VM State
  /// @{

  /// \return the current stack pointer.
  PinnedHermesValue *getStackPointer() {
    return stackPointer_;
  }

  /// Pop the register stack down to a previously saved stack pointer.
  inline void popToSavedStackPointer(PinnedHermesValue *stackPointer);

  /// \return the number of elements in the stack.
  inline uint32_t getStackLevel() const;

  /// Return the available stack size (in registers).
  inline uint32_t availableStackSize() const;

  /// \return true if there is space to allocate <tt>count + STACK_RESERVE</tt>
  /// registers.
  inline bool checkAvailableStack(uint32_t count);

  /// Allocate stack space for \p registers, but keep it uninitialized. The
  /// caller should initialize it ASAP.
  /// \return the new stack pointer.
  inline PinnedHermesValue *allocUninitializedStack(uint32_t count);

  /// Allocate stack space for \p registers and initialize them with
  /// \p initValue.
  /// See implementation for why this is not inlined.
  LLVM_ATTRIBUTE_NOINLINE
  void allocStack(uint32_t count, HermesValue initValue);

  /// Check whether <tt>count + STACK_RESERVE</tt> stack registers are available
  /// and allocate \p count registers.
  /// \param count number of registers to allocate.
  /// \param initValue initialize the allocated registers with this value.
  /// \return \c true if allocation was successful.
  inline bool checkAndAllocStack(uint32_t count, HermesValue initValue);

  /// Pop the specified number of elements from the stack.
  inline void popStack(uint32_t count);

  /// \return the current frame pointer.
  StackFramePtr getCurrentFrame() {
    return currentFrame_;
  }

  /// Set the current frame pointer to the current top of the stack and return
  /// it.
  /// \param topFrame a frame constructed at the top of stack. It must equal
  ///   stackPointer_, but it is more efficient to pass it in if it already
  ///   is in a register. It also provides some additional error checking in
  ///   debug builds, ensuring that the stack hasn't changed unexpectedly.
  inline void setCurrentFrameToTopOfStack(StackFramePtr topFrame);

  /// Set the current frame pointer to the current top of the stack and return
  /// it.
  /// \return the new value of the current frame pointer.
  inline StackFramePtr setCurrentFrameToTopOfStack();

  /// Restore the stack pointer to the base of the current frame and then
  /// set the frame pointer to the previous frame.
  /// \param currentFrame the currentFrame. It must match the value of
  ///   this->currentFrame_, but is more efficient to pass it in assuming it
  ///   already is in a register. It also provides some additional error
  ///   checking in debug builds, ensuring that the stack hasn't changed
  ///   unexpectedly.
  /// \return the updated value of the current frame pointer.
  inline StackFramePtr restoreStackAndPreviousFrame(StackFramePtr currentFrame);

  /// Restore the stack pointer to the base of the current frame and then
  /// set the frame pointer to the previous frame.
  /// \return the updated value of the current frame pointer.
  inline StackFramePtr restoreStackAndPreviousFrame();

  /// \return an iterator range that provides access to all stack frames
  /// starting from the top-most one.
  inline llvh::iterator_range<StackFrameIterator> getStackFrames();

  /// \return an iterator range that provides access to all stack frames
  /// starting from the top-most one.
  inline llvh::iterator_range<ConstStackFrameIterator> getStackFrames() const;

  /// Dump information about all stack frames to \p OS.
  void dumpCallFrames(llvh::raw_ostream &OS);

  /// Dump information about all stack frames to llvh::errs(). This is a
  /// helper method intended to be called from a debugger.
  void dumpCallFrames();

  /// \return `thrownValue`.
  HermesValue getThrownValue() const {
    return thrownValue_;
  }

  /// Set `thrownValue` to the specified value \p value, `returnValue` to
  /// empty and \return ExecutionResult::EXCEPTION.
  ExecutionStatus setThrownValue(HermesValue value);

  /// Set `thrownValue` to empty.
  void clearThrownValue();

  /// Return a hidden class corresponding to the specified prototype object
  /// and number of reserved slots. For now we only use the latter.
  /// Takes and returns raw pointers: standard warnings apply!
  inline HiddenClass *getHiddenClassForPrototypeRaw(
      JSObject *proto,
      unsigned reservedSlots) {
    assert(
        reservedSlots <= InternalProperty::NumInternalProperties &&
        "out of bounds");
    auto *clazz = rootClazzRawPtr_[reservedSlots];
    assert(clazz && "must initialize root classes before use");
    return clazz;
  }

  /// Return the global object.
  Handle<JSObject> getGlobal();

  /// Return the JIT context.
  JITContext &getJITContext() {
    return jitContext_;
  }
  /// Returns trailing data for all runtime modules.
  std::vector<llvh::ArrayRef<uint8_t>> getEpilogues();

  /// \return the set of runtime stats.
  instrumentation::RuntimeStats &getRuntimeStats() {
    return runtimeStats_;
  }

  /// Print the heap and other misc. stats to the given stream.
  void printHeapStats(llvh::raw_ostream &os);

  /// Write IO tracking (aka HBC page access) info to the supplied
  /// stream as JSON. There will only be useful data for RuntimeModules
  /// backed by mmap'ed bytecode, and there will only be any data at all if
  /// RuntimeConfig::withTrackIO() has been set, and IO tracking is available on
  /// the current platform.
  void getIOTrackingInfoJSON(llvh::raw_ostream &os);

#ifndef NDEBUG
  /// Iterate over all arrays in the heap and print their sizes and capacities.
  void printArrayCensus(llvh::raw_ostream &os);
#endif

  /// Returns the common storage object.
  RuntimeCommonStorage *getCommonStorage() {
    return commonStorage_.get();
  }

  const GCExecTrace &getGCExecTrace() {
    return getHeap().getGCExecTrace();
  }

#if defined(HERMES_ENABLE_DEBUGGER)
  /// Request the interpreter loop to take an asynchronous break at a convenient
  /// point due to debugger UI request. This may be called from any thread, or a
  /// signal handler.
  void triggerDebuggerAsyncBreak(
      ::facebook::hermes::debugger::AsyncPauseKind kind) {
    triggerAsyncBreak(
        kind == ::facebook::hermes::debugger::AsyncPauseKind::Explicit
            ? AsyncBreakReasonBits::DebuggerExplicit
            : AsyncBreakReasonBits::DebuggerImplicit);
  }
#endif

  /// Request the interpreter loop to take an asynchronous break at a convenient
  /// point due to previous registered timeout. This may be called from any
  /// thread, or a signal handler.
  void triggerTimeoutAsyncBreak() {
    triggerAsyncBreak(AsyncBreakReasonBits::Timeout);
  }

  /// Register \p callback which will be called
  /// during runtime destruction.
  void registerDestructionCallback(DestructionCallback callback) {
    destructionCallbacks_.emplace_back(callback);
  }

#ifdef HERMES_ENABLE_DEBUGGER
  /// Encapsulates useful information about a stack frame, needed by the
  /// debugger. It requres extra context and cannot be extracted from a
  /// single frame pointer.
  struct StackFrameInfo {
    ConstStackFramePtr frame;
    bool isGlobal;
  };

  /// Extract \c StackFrameInfo of the specified stack frame.
  /// \param frameIdx a relative frame index where the top-most frame is 0.
  /// \return a populated StackFrameInfo, or llvh::None if the frame is invalid.
  llvh::Optional<StackFrameInfo> stackFrameInfoByIndex(uint32_t frameIdx) const;

  /// Calculate and \return the offset between the location of the specified
  /// frame and the start of the stack. This value increases with every nested
  /// call.
  uint32_t calcFrameOffset(ConstStackFrameIterator it) const;

  /// \return the offset between the location of the current frame and the
  ///   start of the stack. This value increases with every nested call.
  uint32_t getCurrentFrameOffset() const;
#endif

  /// Flag the interpreter that a type error with the specified message must be
  /// thrown when execution resumes.
  /// If the message is not a string, it is converted using toString().
  LLVM_NODISCARD ExecutionStatus raiseTypeError(Handle<> message);

  /// Flag the interpreter that a type error must be thrown when execution
  /// resumes.
  /// \return ExecutionResult::EXCEPTION
  LLVM_NODISCARD ExecutionStatus raiseTypeError(const TwineChar16 &msg);

  /// Flag the interpreter that a type error must be thrown when execution
  /// resumes. The string thrown concatenates a description of \p value
  /// with \p msg.
  /// \return ExecutionResult::EXCEPTION
  LLVM_NODISCARD ExecutionStatus
  raiseTypeErrorForValue(Handle<> value, llvh::StringRef msg) {
    return raiseTypeErrorForValue("", value, msg);
  }

  /// Flag the interpreter that a type error must be thrown when execution
  /// resumes. The string thrown concatenates \p msg1, a description of \p
  /// value, and \p msg2. \return ExecutionResult::EXCEPTION
  LLVM_NODISCARD ExecutionStatus raiseTypeErrorForValue(
      llvh::StringRef msg1,
      Handle<> value,
      llvh::StringRef msg2);

  /// Flag the interpreter that a syntax error must be thrown.
  /// \return ExecutionStatus::EXCEPTION
  LLVM_NODISCARD ExecutionStatus raiseSyntaxError(const TwineChar16 &msg);

  /// Raise a special SyntaxError when attempting to eval when disallowed.
  LLVM_NODISCARD ExecutionStatus raiseEvalUnsupported(llvh::StringRef code);

  /// Raise a \c RangeError exception.
  /// \return ExecutionStatus::EXCEPTION
  LLVM_NODISCARD ExecutionStatus raiseRangeError(const TwineChar16 &msg);

  /// Raise a \c ReferenceError exception.
  /// \return ExecutionStatus::EXCEPTION
  LLVM_NODISCARD ExecutionStatus raiseReferenceError(const TwineChar16 &msg);

  /// Raise a \c URIError exception.
  /// \return ExecutionStatus::EXCEPTION
  LLVM_NODISCARD ExecutionStatus raiseURIError(const TwineChar16 &msg);

  enum class StackOverflowKind {
    // The JS register stack was exhausted.
    JSRegisterStack,
    // A limit on the number of native stack frames used in
    // evaluation, intended to conservatively prevent native stack
    // overflow, was exceeded.
    NativeStack,
    // RuntimeJSONParser has a maximum number of "nesting levels", and
    // calls raiseStackOverflow if that is exceeded.
    JSONParser,
    // JSONStringifyer has the same limit as JSONParser.
    JSONStringify,
  };

  /// Raise a stack overflow exception. This is special because constructing
  /// the object must not execute any custom or JavaScript code.  The
  /// argument influences the exception's message, to aid debugging.
  /// \return ExecutionStatus::EXCEPTION
  LLVM_NODISCARD ExecutionStatus raiseStackOverflow(StackOverflowKind kind);

  /// Raise an error for the quit function. This error is not catchable.
  LLVM_NODISCARD ExecutionStatus raiseQuitError();

  /// Raise an error for execution timeout. This error is not catchable.
  LLVM_NODISCARD ExecutionStatus raiseTimeoutError();

  /// Utility function to raise a catchable JS error with \p errMessage.
  LLVM_NODISCARD ExecutionStatus
  raiseUncatchableError(Handle<JSObject> prototype, llvh::StringRef errMessage);

  /// Interpret the current function until it returns or throws and return
  /// CallResult<HermesValue> or the thrown object in 'thrownObject'.
  CallResult<HermesValue> interpretFunction(CodeBlock *newCodeBlock);

#ifdef HERMES_ENABLE_DEBUGGER
  /// Single-step the provided function, update the interpreter state.
  ExecutionStatus stepFunction(InterpreterState &state);
#endif

  /// Inserts an object into the string cycle checking stack.
  /// \return true if a cycle was found
  CallResult<bool> insertVisitedObject(Handle<JSObject> obj);

  /// Removes the last element (which must be obj) from the cycle check stack.
  /// \param obj the last element, which will be removed. Used for checking
  /// that invariants aren't violated in debug mode.
  void removeVisitedObject(Handle<JSObject> obj);

  /// Like calling JSObject::getNamed, but uses this runtime's property cache.
  CallResult<PseudoHandle<>> getNamed(Handle<JSObject> obj, PropCacheID id);

  /// Like calling JSObject::putNamed with the ThrowOnError flag, but uses this
  /// runtime's property cache.
  ExecutionStatus
  putNamedThrowOnError(Handle<JSObject> obj, PropCacheID id, HermesValue hv);

  /// @}

#define RUNTIME_HV_FIELD(name) PinnedHermesValue name{};
#define RUNTIME_HV_FIELD_PROTOTYPE(name) RUNTIME_HV_FIELD(name)
#define RUNTIME_HV_FIELD_INSTANCE(name) RUNTIME_HV_FIELD(name)
#define RUNTIME_HV_FIELD_RUNTIMEMODULE(name) RUNTIME_HV_FIELD(name)
#include "hermes/VM/RuntimeHermesValueFields.def"
#undef RUNTIME_HV_FIELD

  /// Raw pointers to prototypes.
  JSObject *objectPrototypeRawPtr{};

  JSObject *functionPrototypeRawPtr{};

  RegExpMatch regExpLastMatch{};

  /// Whether to allow eval and Function ctor.
  const bool enableEval;
  /// Whether to verify the IR being generated by eval and the Function ctor.
  const bool verifyEvalIR;
  /// Whether to optimize the code in the string passed to eval and the Function
  /// ctor.
  const bool optimizedEval;

#ifdef HERMESVM_PROFILER_OPCODE
  /// Track the frequency of each opcode in the interpreter.
  uint32_t opcodeExecuteFrequency[256] = {0};

  /// Track time spent of each opcode in the interpreter, in CPU cycles.
  uint64_t timeSpent[256] = {0};

  /// Dump opcode stats to a stream.
  void dumpOpcodeStats(llvh::raw_ostream &os) const;
#endif

#if defined(HERMESVM_PROFILER_JSFUNCTION) || defined(HERMESVM_PROFILER_EXTERN)
  static std::atomic<ProfilerID> nextProfilerId;

  std::vector<ProfilerFunctionInfo> functionInfo{};

  /// Get block's index in functionInfo (creating a new entry if needed).
  ProfilerID getProfilerID(CodeBlock *block);

  /// Get profiler info associated with given id or nullptr if the id
  /// is not associated with any function.
  const ProfilerFunctionInfo *getProfilerInfo(ProfilerID id);

  /// Track the maximum size of the stack.
  uint32_t maxStackLevel = 0;
#endif

#if defined(HERMESVM_PROFILER_JSFUNCTION)
  std::vector<ProfilerFunctionEvent> functionEvents{};

  /// Total number of opcodes executed by this runtime.
  uint64_t opcodeCount = 0;

  /// Dump function profiling stats to stdout.
  enum class ProfileType{TIME, OPCODES, ALL};
  void dumpJSFunctionStats(ProfileType type = ProfileType::ALL);
#endif

#ifdef HERMESVM_PROFILER_BB
  BasicBlockExecutionInfo &getBasicBlockExecutionInfo();

  /// Dump basic block profile trace to \p OS in json format.
  void dumpBasicBlockProfileTrace(llvh::raw_ostream &OS);
#endif

  CodeCoverageProfiler &getCodeCoverageProfiler() {
    return *codeCoverageProfiler_;
  }

#ifdef HERMESVM_PROFILER_NATIVECALL
  /// Dump statistics about native calls.
  void dumpNativeCallStats(llvh::raw_ostream &OS);
#endif

#ifdef HERMES_ENABLE_DEBUGGER
  Debugger &getDebugger() {
    return debugger_;
  }
#endif

  RuntimeModuleList &getRuntimeModules() {
    return runtimeModuleList_;
  }

  bool hasES6Proxy() const {
    return hasES6Proxy_;
  }

  bool hasES6Symbol() const {
    return hasES6Symbol_;
  }

  bool builtinsAreFrozen() const {
    return builtinsFrozen_;
  }

  bool shouldStabilizeInstructionCount();

  experiments::VMExperimentFlags getVMExperimentFlags() const {
    return vmExperimentFlags_;
  }

  // Return a reference to the runtime's CrashManager.
  inline CrashManager &getCrashManager();

  /// Returns a string representation of the JS stack.  Does no operations
  /// that allocate on the JS heap, so safe to use for an out-of-memory
  /// exception.
  /// \p ip specifies the the IP of the leaf frame.
  std::string getCallStackNoAlloc(const Inst *ip);

  /// \return a string representation of the JS stack without knowing the leaf
  /// frame ip.  Does no operations that allocate on the JS heap, so safe to use
  /// for an out-of-memory exception.
  std::string getCallStackNoAlloc() override {
    return getCallStackNoAlloc(nullptr);
  }

  /// Called when various GC events(e.g. collection start/end) happen.
  void onGCEvent(GCEventKind kind, const std::string &extraInfo) override;

#ifdef HERMESVM_SERIALIZE
  /// Fill the header with current Runtime config
  void populateHeaderRuntimeConfig(SerializeHeader &header);

  /// Check if the SerializeHeader read from the file matches Runtime config of
  /// current run.
  void checkHeaderRuntimeConfig(SerializeHeader &header) const;

  /// Serialize the VM state.
  void serialize(Serializer &s);

  /// Set the closure function to execute after deserialization.
  void setSerializeClosure(Handle<JSFunction> function) {
    serializeClosure = function.getHermesValue();
  }

  HermesValue getSerializeClosure() {
    return serializeClosure;
  }
#endif

#ifdef HERMESVM_PROFILER_BB
  using ClassId = InlineCacheProfiler::ClassId;

  /// Get filename, line number, and column number from
  /// code block and instruction pointer. It returns true if it succeeds.
  llvh::Optional<std::tuple<std::string, uint32_t, uint32_t>>
  getIPSourceLocation(const CodeBlock *codeBlock, const Inst *ip);

  /// Inserts the Hidden class as a root to prevent it from being garbage
  /// collected.
  void preventHCGC(HiddenClass *hc);

  /// Inserts Hidden Classes into InlineCacheProfiler
  void recordHiddenClass(
      CodeBlock *codeBlock,
      const Inst *cacheMissInst,
      SymbolID symbolID,
      HiddenClass *objectHiddenClass,
      HiddenClass *cachedHiddenClass);

  /// Resolve HiddenClass pointers from its hidden class Id.
  HiddenClass *resolveHiddenClassId(ClassId classId);

  /// Dumps inline cache profiler info.
  void getInlineCacheProfilerInfo(llvh::raw_ostream &ostream);
#endif

 protected:
  /// Construct a Runtime on the stack.
  /// NOTE: This should only be used by StackRuntime. All other uses should use
  /// Runtime::create.
  explicit Runtime(
      std::shared_ptr<StorageProvider> provider,
      const RuntimeConfig &runtimeConfig);

/// @}
#if defined(HERMESVM_PROFILER_EXTERN)
 public:
#else
 private:
#endif
  /// Only called internally or by the wrappers used for profiling.
  CallResult<HermesValue> interpretFunctionImpl(CodeBlock *newCodeBlock);

 private:
  /// Called by the GC at the beginning of a collection. This method informs the
  /// GC of all runtime roots.  The \p markLongLived argument
  /// indicates whether root data structures that contain only
  /// references to long-lived objects (allocated directly as long lived)
  /// are required to be scanned.
  void markRoots(RootAcceptor &acceptor, bool markLongLived) override;

  /// Called by the GC during collections that may reset weak references. This
  /// method informs the GC of all runtime weak roots.
  void markWeakRoots(WeakRootAcceptor &weakAcceptor) override;

  /// Visits every entry in the identifier table and calls acceptor with
  /// the entry and its id as arguments. This is intended to be used only for
  /// snapshots, as it is slow. The function passed as acceptor shouldn't
  /// perform any heap operations.
  void visitIdentifiers(
      const std::function<void(UTF16Ref, uint32_t id)> &acceptor) override;

#ifdef HERMESVM_PROFILER_BB
 public:
#endif
  /// Convert the given symbol into its UTF-8 string representation.
  std::string convertSymbolToUTF8(SymbolID id) override;

 private:
  /// Prints any statistics maintained in the Runtime about GC to \p
  /// os.  At present, this means the breakdown of markRoots time by
  /// "phase" within markRoots.
  void printRuntimeGCStats(JSONEmitter &json) const override;

  /// \return one higher than the largest symbol in the identifier table. This
  /// enables the GC to size its internal structures for symbol marking.
  /// Optionally invoked at the beginning of a garbage collection.
  virtual unsigned getSymbolsEnd() const override;

  /// If any symbols are marked by the IdentifierTable, clear that marking.
  /// Optionally invoked at the beginning of some collections.
  virtual void unmarkSymbols() override;

  /// Called by the GC at the end of a collection to free all symbols not set in
  /// markedSymbols.
  virtual void freeSymbols(const std::vector<bool> &markedSymbols) override;

#ifdef HERMES_SLOW_DEBUG
  /// \return true if the given symbol is a live entry in the identifier
  /// table.
  virtual bool isSymbolLive(SymbolID id) override;
#endif

  /// See \c GCCallbacks for details.
  size_t mallocSize() const override;

  /// Generate a bytecode buffer that contains a few special functions:
  /// 0) an empty function that returns undefined.
  /// 1) a function that returns the global object.
  static std::unique_ptr<Buffer> generateSpecialRuntimeBytecode();

  /// Insert the predefined strings into the IdentifierTable.
  /// NOTE: this function does not do any allocations in the GC heap, it is safe
  /// to use at any time in initialization.
  void initPredefinedStrings();

  /// Initialize the \c charStrings_ array with a StringPrimitive for each
  /// character.
  void initCharacterStrings();

  /// \param methodIndex is the index of the method in the table that lists
  ///   all the builtin methods, which is what we are iterating over.
  /// \param objectName is the id for the name of the object in the list of the
  ///   predefined strings.
  /// \param object is the object where the builtin method is defined as a
  ///   property.
  /// \param methodID is the SymbolID for the name of the method.
  using ForEachBuiltinCallback = ExecutionStatus(
      unsigned methodIndex,
      Predefined::Str objectName,
      Handle<JSObject> &object,
      SymbolID methodID);

  /// Enumerate the builtin methods, and invoke the callback on each method.
  ExecutionStatus forEachBuiltin(
      const std::function<ForEachBuiltinCallback> &callback);

  /// Populate the builtins table by extracting the values from the global
  /// object.
  void initBuiltinTable();

  /// Walk all the builtin methods, assert that they are not overridden. If they
  /// are, throw an exception. This will be called at most once, before freezing
  /// the builtins.
  ExecutionStatus assertBuiltinsUnmodified();

  /// Called after asserting all builtin methods are not overridden, to freeze
  /// those builtins. This will be called at most once.
  void freezeBuiltins();

  /// The slow path for \c getCharacterString(). This function allocates a new
  /// string for the passed character \p ch.
  Handle<StringPrimitive> allocateCharacterString(char16_t ch);

  /// Add a \c RuntimeModule \p rm to the runtime module list.
  void addRuntimeModule(RuntimeModule *rm) {
    runtimeModuleList_.push_back(*rm);
  }

  /// Remove a \c RuntimeModule \p rm from the runtime module list.
  void removeRuntimeModule(RuntimeModule *rm);

  /// Called by CrashManager on the event of a crash to produce a stream of data
  /// to crash log. Output should be a JSON object. This is the central point
  /// for adding calls to further functions which dump specific elements of
  /// of crash dump data for a Hermes Runtime instance.
  void crashCallback(int fd);

  /// Write a JS stack trace as part of a \c crashCallback() run.
  void crashWriteCallStack(JSONEmitter &json);

#ifdef HERMESVM_SERIALIZE
  void serializeIdentifierTable(Serializer &s);

  /// Serialize Runtime fields.
  void serializeRuntimeFields(Serializer &s);

  /// Deserialize Runtime fields.
  void deserializeRuntimeFields(Deserializer &d);

  /// Deserialize the VM state.
  /// \param inputFile MemoryBuffer to read from.
  /// \param currentlyInYoung Whether we are allocating from the young gen
  /// before deserialization starts and should we go back to allocating from the
  /// young gen after deserialziation.
  void deserializeImpl(Deserializer &d, bool currentlyInYoung);
#endif

 private:
  GC heap_;
  std::vector<std::function<void(GC *, SlotAcceptor &)>> customMarkRootFuncs_;
  std::vector<std::function<void(GC *, WeakRefAcceptor &)>>
      customMarkWeakRootFuncs_;

  /// All state related to JIT compilation.
  JITContext jitContext_;

  /// Set to true if we should enable ES6 Proxy.
  const bool hasES6Proxy_;

  /// Set to true if we should enable ES6 Symbol.
  const bool hasES6Symbol_;

  /// Set to true if we should randomize stack placement etc.
  const bool shouldRandomizeMemoryLayout_;

  // Percentage in [0,100] of bytecode we should eagerly read into page cache.
  const uint8_t bytecodeWarmupPercent_;

  // Signal-based I/O tracking. Slows down execution.
  const bool trackIO_;

  /// This value can be passed to the runtime as flags to test experimental
  /// features. Each experimental feature decides how to interpret these
  /// values. Generally each experiment is associated with one or more bits of
  /// this value. Interpretation of these bits is up to each experiment.
  /// To add an experiment, populate the VMExperimentFlags enum with additional
  /// bit values, typically 1 as test and 0 as control.
  experiments::VMExperimentFlags vmExperimentFlags_{experiments::Default};

  friend class GCScope;
  friend class HandleBase;
  friend class Interpreter;
  friend class RuntimeModule;
  friend class MarkRootsPhaseTimer;
  friend struct RuntimeOffsets;
  friend class JITContext;
  friend class ScopedNativeDepthReducer;
  friend class ScopedNativeDepthTracker;
  friend class ScopedNativeCallFrame;

  class MarkRootsPhaseTimer;

  /// Whenever we pass through the first phase, we record the current time here,
  /// so we can calculate the total time after we pass through the last phase.
  std::chrono::time_point<std::chrono::steady_clock> startOfMarkRoots_;
  /// The duration of each GC root marking phase is accumulated here.
  double markRootsPhaseTimes_[static_cast<unsigned>(
      RootAcceptor::Section::NumSections)] = {};
  /// The duration of the all root makring is accumulated here.
  double totalMarkRootsTime_ = 0.0;

  /// A global counter that increments and provide unique object IDs.
  ObjectID nextObjectID_{0};

  /// The identifier table.
  IdentifierTable identifierTable_{};

  /// The global symbol registry.
  SymbolRegistry symbolRegistry_{};

  /// Set of runtime statistics.
  instrumentation::RuntimeStats runtimeStats_;

  /// Shared location to place native objects required by JSLib
  std::shared_ptr<RuntimeCommonStorage> commonStorage_;

  /// Empty code block that returns undefined.
  /// Owned by specialCodeBlockRuntimeModule_.
  CodeBlock *emptyCodeBlock_{};

  /// Code block that returns the global object.
  /// Owned by specialCodeBlockRuntimeModule_.
  CodeBlock *returnThisCodeBlock_{};

  /// The runtime module that owns emptyCodeBlock_ and returnThisCodeBlock_.
  /// We use a raw pointer here because it will be added to runtimeModuleList_,
  /// and will be freed when Runtime is freed.
  RuntimeModule *specialCodeBlockRuntimeModule_{};

  /// A list of all active runtime modules. Each \c RuntimeModule adds itself
  /// on construction and removes itself on destruction.
  RuntimeModuleList runtimeModuleList_{};

  /// @name Private VM State
  /// @{

  PinnedHermesValue *registerStack_;
  PinnedHermesValue *registerStackEnd_;
  PinnedHermesValue *stackPointer_;
  /// Bytes of register stack to unmap on destruction.
  /// When set to zero, the register stack is not allocated
  /// by the runtime itself.
  uint32_t registerStackBytesToUnmap_{0};
  /// Manages data to be used in the case of a crash.
  std::shared_ptr<CrashManager> crashMgr_;
  /// Points to the last register in the callers frame. The current frame (the
  /// callee frame) starts in the next register and continues up to and
  /// including \c stackPointer_.
  StackFramePtr currentFrame_{nullptr};

  /// Current depth of native call frames, including recursive interpreter
  /// calls.
  unsigned nativeCallFrameDepth_{0};

 public:
  /// A stack overflow exception is thrown when \c nativeCallFrameDepth_ exceeds
  /// this threshold.
  static constexpr unsigned MAX_NATIVE_CALL_FRAME_DEPTH =
#ifdef HERMES_LIMIT_STACK_DEPTH
      // UBSAN builds will hit a native stack overflow much earlier, so make
      // this limit dramatically lower.
      30
#elif defined(_MSC_VER) && defined(__clang__) && defined(HERMES_SLOW_DEBUG)
      30
#elif defined(_MSC_VER) && defined(HERMES_SLOW_DEBUG)
      // On windows in dbg mode builds, stack frames are bigger, and a depth
      // limit of 384 results in a C++ stack overflow in testing.
      128
#elif defined(_MSC_VER) && !NDEBUG
      192
#else
      /// This depth limit was originally 256, and we
      /// increased when an app violated it.  The new depth is 128
      /// larger.  See T46966147 for measurements/calculations indicating
      /// that this limit should still insulate us from native stack overflow.)
      384
#endif
      ;

  /// rootClazzRawPtr_[i] is a raw pointer to a hidden class with its i first
  /// slots pre-reserved.
  HiddenClass *rootClazzRawPtr_[InternalProperty::NumInternalProperties + 1] = {
      nullptr};

  /// Cache for property lookups in non-JS code.
  PropertyCacheEntry fixedPropCache_[(size_t)PropCacheID::_COUNT];

  /// StringPrimitive representation of the first 256 characters.
  /// These are allocated as "long-lived" objects, so they don't need
  /// to be scanned as roots in young-gen collections.
  std::vector<PinnedHermesValue> charStrings_{};

  /// Pointers to native implementations of builtins.
  std::vector<NativeFunction *> builtins_{};

  /// True if the builtins are all frozen (non-writable, non-configurable).
  bool builtinsFrozen_{false};

#ifdef HERMESVM_PROFILER_BB
  BasicBlockExecutionInfo basicBlockExecInfo_;

  /// Store all inline caching miss information.
  InlineCacheProfiler inlineCacheProfiler_;
#endif

  /// ScriptIDs to use for new RuntimeModules coming in.
  facebook::hermes::debugger::ScriptID nextScriptId_{1};

  /// Store a key for the function that is executed if a crash occurs.
  /// This key will be unregistered in the destructor.
  const CrashManager::CallbackKey crashCallbackKey_;

  /// Keep a strong reference to the SamplingProfiler so that
  /// we are sure it's safe to unregisterRuntime in destructor.
  std::shared_ptr<SamplingProfiler> samplingProfiler_;

  /// Reference to the code coverage profiler.
  std::shared_ptr<CodeCoverageProfiler> codeCoverageProfiler_;

  /// A list of callbacks to call before runtime destruction.
  std::vector<DestructionCallback> destructionCallbacks_;

  /// Bit flags for async break request reasons.
  enum class AsyncBreakReasonBits : uint8_t {
    DebuggerExplicit = 0x1,
    DebuggerImplicit = 0x2,
    Timeout = 0x4,
  };

  /// An atomic flag set when an async pause is requested.
  /// It is a bits flag with each bit reserved for different clients
  /// defined by AsyncBreakReasonBits.
  /// This may be manipulated from multiple threads.
  std::atomic<uint8_t> asyncBreakRequestFlag_{0};

#if defined(HERMES_ENABLE_DEBUGGER)
  /// \return zero if no debugger async pause was requested, the old nonzero
  /// async flags if an async pause was requested. If nonzero is returned, the
  /// flag is reset to 0.
  uint8_t testAndClearDebuggerAsyncBreakRequest() {
    return testAndClearAsyncBreakRequest(
        (uint8_t)AsyncBreakReasonBits::DebuggerExplicit |
        (uint8_t)AsyncBreakReasonBits::DebuggerImplicit);
  }

  Debugger debugger_{this};
#endif

  /// \return whether any async break is requested or not.
  bool hasAsyncBreak() const {
    return asyncBreakRequestFlag_.load(std::memory_order_relaxed) != 0;
  }

  /// \return whether async break was requested or not for \p reasonBits. Clear
  /// \p reasonBit request bit afterward.
  uint8_t testAndClearAsyncBreakRequest(uint8_t reasonBits) {
    /// Note that while the triggerTimeoutAsyncBreak() function may be called
    /// from any thread, this one may only be called from within the Interpreter
    /// loop.
    uint8_t flag = asyncBreakRequestFlag_.load(std::memory_order_relaxed);
    if (LLVM_LIKELY((flag & (uint8_t)reasonBits) == 0)) {
      // Fast path.
      return false;
    }
    // Clear the flag using CAS.
    uint8_t oldFlag = asyncBreakRequestFlag_.fetch_and(
        ~(uint8_t)reasonBits, std::memory_order_relaxed);
    assert(oldFlag != 0 && "Why is oldFlag zero?");
    return oldFlag;
  }

  /// \return whether timeout async break was requsted or not. Clear the
  /// timeout request bit afterward.
  bool testAndClearTimeoutAsyncBreakRequest() {
    return testAndClearAsyncBreakRequest(
        (uint8_t)AsyncBreakReasonBits::Timeout);
  }

  /// Request the interpreter loop to take an asynchronous break
  /// at a convenient point.
  void triggerAsyncBreak(AsyncBreakReasonBits reason) {
    asyncBreakRequestFlag_.fetch_or((uint8_t)reason, std::memory_order_relaxed);
  }

  /// Notify runtime execution has timeout.
  ExecutionStatus notifyTimeout();

  /// Holds references to persistent BC providers for the lifetime of the
  /// Runtime. This is needed because the identifier table may contain pointers
  /// into bytecode, and so memory backing these must be preserved.
  std::vector<std::shared_ptr<hbc::BCProvider>> persistentBCProviders_;

  /// Config-provided callback for GC events.
  std::function<void(GCEventKind, const char *)> gcEventCallback_;

  /// Set from RuntimeConfig.
  bool allowFunctionToStringWithRuntimeSource_;

 private:
#ifdef NDEBUG
  /// See \c ::setCurrentIP() and \c ::getCurrentIP() .
  const inst::Inst *currentIP_{nullptr};
#else
  /// When assertions are enabled we track whether \c currentIP_ is "valid" by
  /// making it optional. If this is accessed when the optional value is cleared
  /// (the invalid state) we assert.
  llvh::Optional<const inst::Inst *> currentIP_{(const inst::Inst *)nullptr};
#endif

 public:
  /// Set the value of the current Instruction Pointer (IP). Generally this is
  /// called by the interpeter before it makes any major calls out of its main
  /// loop. However, the interpeter also reads the value held here back after
  /// major calls return so this can also be called by other code which wants to
  /// return to the interpreter at a different IP. This allows things external
  /// to the interpreter loop to affect the flow of bytecode execution.
  inline void setCurrentIP(const inst::Inst *ip) {
    currentIP_ = ip;
  }

  /// Get the current Instruction Pointer (IP). This can be used to find out
  /// the last bytecode executed if we're currently in the interpeter loop. If
  /// we are not in the interpeter loop (i.e. we've made it into the VM
  /// internals via a native call), this this will return nullptr.
  inline const inst::Inst *getCurrentIP() const {
#ifdef NDEBUG
    return currentIP_;
#else
    assert(
        currentIP_.hasValue() &&
        "Current IP unknown - this probably means a CAPTURE_IP_* is missing in the interpreter.");
    return *currentIP_;
#endif
  }

  /// This is slow compared to \c getCurrentIP() as it's virtual.
  inline const inst::Inst *getCurrentIPSlow() const override {
    return getCurrentIP();
  }

#ifdef NDEBUG
  void invalidateCurrentIP() {}
#else
  void invalidateCurrentIP() {
    currentIP_.reset();
  }
#endif

  /// Save the return address in the caller in the stack frame.
  /// This needs to be called at the beginning of a function call, after the
  /// stack frame is set up.
  void saveCallerIPInStackFrame() {
#ifndef NDEBUG
    assert(
        (!currentFrame_.getSavedIP() ||
         (currentIP_.hasValue() && currentFrame_.getSavedIP() == currentIP_)) &&
        "The ip should either be null or already have the expected value");
#endif
    currentFrame_.getSavedIPRef() =
        HermesValue::encodeNativePointer(getCurrentIP());
  }

  void setAllowFunctionToStringWithRuntimeSource(bool v) {
    allowFunctionToStringWithRuntimeSource_ = v;
  }

  bool getAllowFunctionToStringWithRuntimeSource() const {
    return allowFunctionToStringWithRuntimeSource_;
  }

 private:
  /// Given the current last known IP used in the interpreter loop, returns the
  /// last known CodeBlock and IP combination. IP must not be null as this
  /// suggests we're not in the interpter loop, and there will be no CodeBlock
  /// to find.
  std::pair<const CodeBlock *, const inst::Inst *>
  getCurrentInterpreterLocation(const inst::Inst *initialSearchIP) const;

 public:
  /// Return a StackTraceTreeNode for the last known interpreter bytecode
  /// location. Returns nullptr if we're not in the interpeter loop, or
  /// allocation location tracking is not enabled. For this to function it
  /// needs to be passed the current IP from getCurrentIP(). We do not call
  /// this internally because it should always be called prior even if we
  /// do not really want a stack-traces node. This means we can leverage our
  /// library of tests to assert getCurrentIP() would return the right value
  /// at this point without actually collecting stack-trace data.
  StackTracesTreeNode *getCurrentStackTracesTreeNode(
      const inst::Inst *ip) override;

  /// Return the current StackTracesTree or nullptr if it's not available.
  StackTracesTree *getStackTracesTree() override {
    return stackTracesTree_.get();
  }

  /// To facilitate allocation location tracking this must be called by the
  /// interpeter:
  /// * Just before we enter a new CodeBlock
  /// * At the entry point of a CodeBlock if this is the first entry into the
  ///   interpter loop.
  inline void pushCallStack(const CodeBlock *codeBlock, const inst::Inst *ip) {
    if (stackTracesTree_) {
      pushCallStackImpl(codeBlock, ip);
    }
  }

  /// Must pair up with every call to \c pushCallStack .
  inline void popCallStack() {
    if (stackTracesTree_) {
      popCallStackImpl();
    }
  }

  /// Enable allocation location tracking. Only works with
  /// HERMES_ENABLE_ALLOCATION_LOCATION_TRACES.
  void enableAllocationLocationTracker();

  /// Disable allocation location tracking for new objects. Old objects tagged
  /// with stack traces continue to be tracked until they are freed.
  /// \param clearExistingTree is for use by tests and in general will break
  /// because old objects would end up with dead pointers to stack-trace nodes.
  void disableAllocationLocationTracker(bool clearExistingTree = false);

 private:
  void popCallStackImpl();
  void pushCallStackImpl(const CodeBlock *codeBlock, const inst::Inst *ip);
  std::unique_ptr<StackTracesTree> stackTracesTree_;
};

/// StackRuntime is meant to be used whenever a Runtime should be allocated on
/// the stack. This should only be used by JSI, everything else should use the
/// default creator.
class StackRuntime final : public Runtime {
 public:
  StackRuntime(const RuntimeConfig &config);
  StackRuntime(
      std::shared_ptr<StorageProvider> provider,
      const RuntimeConfig &config);

  // A dummy virtual destructor to avoid problems when StackRuntime is used
  // in compilation units compiled with RTTI.
  virtual ~StackRuntime();
};

/// An RAII class for automatically tracking the native call frame depth.
class ScopedNativeDepthTracker {
  Runtime *const runtime_;

 public:
  explicit ScopedNativeDepthTracker(Runtime *runtime) : runtime_(runtime) {
    ++runtime->nativeCallFrameDepth_;
  }
  ~ScopedNativeDepthTracker() {
    --runtime_->nativeCallFrameDepth_;
  }

  /// \return whether we overflowed the native call frame depth.
  bool overflowed() const {
    return runtime_->nativeCallFrameDepth_ >
        Runtime::MAX_NATIVE_CALL_FRAME_DEPTH;
  }
};

/// An RAII class which creates a little headroom in the native depth
/// tracking.  This is used when calling into JSError to extract the
/// stack, as the error may represent an overflow.  Without this, a
/// cascade of exceptions could occur, overflowing the C++ stack.
class ScopedNativeDepthReducer {
  Runtime *const runtime_;
  bool undo = false;
  // This is empirically good enough.
  static constexpr int kDepthAdjustment = 3;

 public:
  explicit ScopedNativeDepthReducer(Runtime *runtime) : runtime_(runtime) {
    if (runtime->nativeCallFrameDepth_ >= kDepthAdjustment) {
      runtime->nativeCallFrameDepth_ -= kDepthAdjustment;
      undo = true;
    }
  }
  ~ScopedNativeDepthReducer() {
    if (undo) {
      runtime_->nativeCallFrameDepth_ += kDepthAdjustment;
    }
  }
};

/// A ScopedNativeCallFrame is an RAII class that manipulates the Runtime
/// stack and depth counter, and holds a stack frame. The stack frame contents
/// may be accessed (as StackFramePtr) via ->. Note that constructing this may
/// fail due to stack overflow, either via the register stack or the depth
/// counter. It is necessary to check the overflowed() flag before access the
/// stack frame contents.
/// Note that the arguments to the call frame are left uninitialized. The caller
/// must clear these before triggering a GC. The fillArgs() function may be used
/// for this purpose.
class ScopedNativeCallFrame {
  /// The runtime for this call frame.
  Runtime *const runtime_;

  /// The stack pointer that will be restored in the destructor.
  PinnedHermesValue *const savedSP_;

  /// The contents of the new frame.
  StackFramePtr frame_;

  /// Whether this call frame overflowed.
  bool overflowed_;

#ifndef NDEBUG
  /// Whether the user has called overflowed() with a false result.
  mutable bool overflowHasBeenChecked_{false};
#endif

  /// \return whether the runtime can allocate a new frame with the given number
  /// of registers. This may fail if we've overflowed our register stack, or
  /// exceeded the native call frame depth.
  static bool runtimeCanAllocateFrame(
      Runtime *runtime,
      uint32_t registersNeeded) {
    return runtime->checkAvailableStack(registersNeeded) &&
        runtime->nativeCallFrameDepth_ <= Runtime::MAX_NATIVE_CALL_FRAME_DEPTH;
  }

 public:
  /// Construct a native call frame for the given \p runtime in preparation of
  /// calling \p callee with \p argCount arguments and the given \p thisArg.
  /// \p callee is either a native pointer to CodeBlock, or an object pointer to
  /// a Callable (the two cases are distinguished by the type tag).
  /// \p newTarget is either \c undefined or the callable of the constructor
  /// currently being invoked by new.
  /// On overflow, the overflowed() flag is set, in which case the stack frame
  /// must not be used.
  /// The arguments are initially uninitialized. The caller should initialize
  /// them by storing into them, or via fillArguments().
  ScopedNativeCallFrame(
      Runtime *runtime,
      uint32_t argCount,
      HermesValue callee,
      HermesValue newTarget,
      HermesValue thisArg)
      : runtime_(runtime), savedSP_(runtime->getStackPointer()) {
    runtime->nativeCallFrameDepth_++;
    uint32_t registersNeeded =
        StackFrameLayout::callerOutgoingRegisters(argCount);
    overflowed_ = !runtimeCanAllocateFrame(runtime, registersNeeded);
    if (LLVM_UNLIKELY(overflowed_)) {
      return;
    }

    // We have enough space. Increment the call frame depth and construct the
    // frame. The ScopedNativeCallFrame will restore both.
    auto *stack = runtime->allocUninitializedStack(registersNeeded);
    frame_ = StackFramePtr::initFrame(
        stack,
        runtime->currentFrame_,
        nullptr,
        nullptr,
        argCount,
        callee,
        newTarget);
    frame_.getThisArgRef() = thisArg;
#if HERMES_SLOW_DEBUG
    // Poison the initial arguments to ensure the caller sets all of them before
    // a GC.
    assert(!overflowed_ && "Overflow should return early");
    overflowHasBeenChecked_ = true;
    fillArguments(argCount, HermesValue::encodeInvalidValue());
    // We still want the user to check for overflow.
    overflowHasBeenChecked_ = false;
#endif
  }

  /// Construct a native call frame for the given \p runtime in preparation of
  /// calling \p callee with \p argCount arguments and the given \p thisArg. On
  /// overflow, the overflowed() flag is set, in which case the stack frame must
  /// not be used.
  /// The arguments are initially uninitialized. The caller should initialize
  /// them by storing into them, or via fillArguments().
  ScopedNativeCallFrame(
      Runtime *runtime,
      uint32_t argCount,
      Callable *callee,
      bool construct,
      HermesValue thisArg)
      : ScopedNativeCallFrame(
            runtime,
            argCount,
            HermesValue::encodeObjectValue(callee),
            construct ? HermesValue::encodeObjectValue(callee)
                      : HermesValue::encodeUndefinedValue(),
            thisArg) {}

  ~ScopedNativeCallFrame() {
    // Note that we unconditionally increment the native call frame depth and
    // save the SP to avoid branching in the dtor.
    runtime_->nativeCallFrameDepth_--;
    runtime_->popToSavedStackPointer(savedSP_);
#ifndef NDEBUG
    // Clear the frame to detect use-after-free.
    frame_ = StackFramePtr{};
#endif
  }

  /// Fill \p argCount arguments with the given value \p fillValue.
  void fillArguments(uint32_t argCount, HermesValue fillValue) {
    assert(overflowHasBeenChecked_ && "ScopedNativeCallFrame could overflow");
    assert(argCount == frame_.getArgCount() && "Arg count mismatch.");
    std::uninitialized_fill_n(&frame_.getArgRefUnsafe(0), argCount, fillValue);
  }

  /// \return whether the stack frame overflowed.
  bool overflowed() const {
#ifndef NDEBUG
    overflowHasBeenChecked_ = !overflowed_;
#endif
    return overflowed_;
  }

  /// Access the stack frame contents via ->.
  StackFramePtr operator->() {
    assert(overflowHasBeenChecked_ && "ScopedNativeCallFrame could overflow");
    return frame_;
  }
};

/// RAII class to temporarily disallow allocation.
/// Enforced by the GC in slow debug mode only.
class NoAllocScope {
 public:
#ifdef NDEBUG
  explicit NoAllocScope(Runtime *runtime) {}
  explicit NoAllocScope(GC *gc) {}
  void release() {}
#else
  explicit NoAllocScope(Runtime *runtime) : NoAllocScope(&runtime->getHeap()) {}
  explicit NoAllocScope(GC *gc) : noAllocLevel_(&gc->noAllocLevel_) {
    ++*noAllocLevel_;
  }

  ~NoAllocScope() {
    if (noAllocLevel_)
      release();
  }

  /// End this scope early. May only be called once.
  void release() {
    assert(noAllocLevel_ && "already released");
    assert(*noAllocLevel_ > 0 && "unbalanced no alloc");
    --*noAllocLevel_;
    noAllocLevel_ = nullptr;
  }

 private:
  uint32_t *noAllocLevel_;
#endif
};

//===----------------------------------------------------------------------===//
// Runtime inline methods.

inline void Runtime::addCustomRootsFunction(
    std::function<void(GC *, SlotAcceptor &)> markRootsFn) {
  customMarkRootFuncs_.emplace_back(std::move(markRootsFn));
}

inline void Runtime::addCustomWeakRootsFunction(
    std::function<void(GC *, WeakRefAcceptor &)> markRootsFn) {
  customMarkWeakRootFuncs_.emplace_back(std::move(markRootsFn));
}

template <bool fixedSize, HasFinalizer hasFinalizer>
inline void *Runtime::alloc(uint32_t sz) {
#if !defined(HERMES_ENABLE_ALLOCATION_LOCATION_TRACES) && !defined(NDEBUG)
  // If allocation location tracking is enabled we implicitly call
  // getCurrentIP() via newAlloc() below. Even if this isn't enabled, we
  // always call getCurrentIP() in a debug build as this has the effect of
  // asserting the IP is correctly set (not invalidated) at this point. This
  // allows us to leverage our whole test-suite to find missing cases of
  // CAPTURE_IP* macros in the interpreter loop.
  (void)getCurrentIP();
#endif
  void *ptr = heap_.alloc<fixedSize, hasFinalizer>(sz);
#ifdef HERMES_ENABLE_ALLOCATION_LOCATION_TRACES
  heap_.getAllocationLocationTracker().newAlloc(ptr);
#endif
  return ptr;
}

template <
    typename T,
    HasFinalizer hasFinalizer,
    LongLived longLived,
    class... Args>
T *Runtime::makeAFixed(Args &&... args) {
#if !defined(HERMES_ENABLE_ALLOCATION_LOCATION_TRACES) && !defined(NDEBUG)
  // If allocation location tracking is enabled we implicitly call
  // getCurrentIP() via newAlloc() below. Even if this isn't enabled, we
  // always call getCurrentIP() in a debug build as this has the effect of
  // asserting the IP is correctly set (not invalidated) at this point. This
  // allows us to leverage our whole test-suite to find missing cases of
  // CAPTURE_IP* macros in the interpreter loop.
  (void)getCurrentIP();
#endif
  T *ptr = heap_.makeA<T, true /* fixedSize */, hasFinalizer, longLived>(
      cellSize<T>(), std::forward<Args>(args)...);
#ifdef HERMES_ENABLE_ALLOCATION_LOCATION_TRACES
  heap_.getAllocationLocationTracker().newAlloc(ptr);
#endif
  return ptr;
}

template <
    typename T,
    HasFinalizer hasFinalizer,
    LongLived longLived,
    class... Args>
T *Runtime::makeAVariable(uint32_t size, Args &&... args) {
#if !defined(HERMES_ENABLE_ALLOCATION_LOCATION_TRACES) && !defined(NDEBUG)
  // If allocation location tracking is enabled we implicitly call
  // getCurrentIP() via newAlloc() below. Even if this isn't enabled, we
  // always call getCurrentIP() in a debug build as this has the effect of
  // asserting the IP is correctly set (not invalidated) at this point. This
  // allows us to leverage our whole test-suite to find missing cases of
  // CAPTURE_IP* macros in the interpreter loop.
  (void)getCurrentIP();
#endif
  T *ptr = heap_.makeA<T, false /* fixedSize */, hasFinalizer, longLived>(
      size, std::forward<Args>(args)...);
#ifdef HERMES_ENABLE_ALLOCATION_LOCATION_TRACES
  heap_.getAllocationLocationTracker().newAlloc(ptr);
#endif
  return ptr;
}

template <typename T>
inline T Runtime::ignoreAllocationFailure(CallResult<T> res) {
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION))
    hermes_fatal("Unhandled out of memory exception");
  // Use std::move here to account for specializations of CallResult,
  // (in particular, CallResult<PseudoHandle<U>>)
  // which wrap a class with a deleted copy constructor.
  return std::move(res.getValue());
}

inline void Runtime::ignoreAllocationFailure(ExecutionStatus status) {
  if (LLVM_UNLIKELY(status == ExecutionStatus::EXCEPTION))
    hermes_fatal("Unhandled out of memory exception");
}

inline void Runtime::ttiReached() {
  // Currently, only the heap_ behavior can change at TTI.
  heap_.ttiReached();
}

template <class T>
inline Handle<T> Runtime::makeHandle(const GCPointer<T> &p) {
  return Handle<T>(this, p.get(this));
}

template <class T>
inline MutableHandle<T> Runtime::makeMutableHandle(const GCPointer<T> &p) {
  return MutableHandle<T>(this, p.get(this));
}

inline StringPrimitive *Runtime::getPredefinedString(
    Predefined::Str predefined) {
  return getStringPrimFromSymbolID(Predefined::getSymbolID(predefined));
}

inline StringPrimitive *Runtime::getPredefinedString(
    Predefined::Sym predefined) {
  return getStringPrimFromSymbolID(Predefined::getSymbolID(predefined));
}

inline Handle<StringPrimitive> Runtime::getPredefinedStringHandle(
    Predefined::Str predefined) {
  return makeHandle(getPredefinedString(predefined));
}

inline Handle<StringPrimitive> Runtime::getPredefinedStringHandle(
    Predefined::Sym predefined) {
  return makeHandle(getPredefinedString(predefined));
}

inline StringPrimitive *Runtime::getStringPrimFromSymbolID(SymbolID id) {
  return identifierTable_.getStringPrim(this, id);
}

#ifdef HERMESVM_PROFILER_BB
inline BasicBlockExecutionInfo &Runtime::getBasicBlockExecutionInfo() {
  return basicBlockExecInfo_;
}

inline void Runtime::dumpBasicBlockProfileTrace(llvh::raw_ostream &OS) {
  basicBlockExecInfo_.dump(OS);
}
#endif

inline Runtime::FormatSymbolID Runtime::formatSymbolID(SymbolID id) {
  return FormatSymbolID(this, id);
}

inline void Runtime::popToSavedStackPointer(PinnedHermesValue *stackPointer) {
  assert(
      stackPointer >= stackPointer_ &&
      "attempting to pop the stack to a higher level");
  stackPointer_ = stackPointer;
}

inline uint32_t Runtime::getStackLevel() const {
  return (uint32_t)(registerStackEnd_ - stackPointer_);
}

inline uint32_t Runtime::availableStackSize() const {
  return (uint32_t)(stackPointer_ - registerStack_);
}

inline bool Runtime::checkAvailableStack(uint32_t count) {
  // Note: use 64-bit arithmetic to avoid overflow. We could also do it with
  // a couple of comparisons, but that is likely to be slower.
  return availableStackSize() >= (uint64_t)count + STACK_RESERVE;
}

inline PinnedHermesValue *Runtime::allocUninitializedStack(uint32_t count) {
  assert(availableStackSize() >= count && "register stack overflow");
  return stackPointer_ -= count;
}

inline bool Runtime::checkAndAllocStack(uint32_t count, HermesValue initValue) {
  if (!checkAvailableStack(count))
    return false;
  allocStack(count, initValue);
  return true;
}

inline void Runtime::popStack(uint32_t count) {
  assert(getStackLevel() >= count && "register stack underflow");
  stackPointer_ += count;
}

inline void Runtime::setCurrentFrameToTopOfStack(StackFramePtr topFrame) {
  assert(
      topFrame.ptr() == stackPointer_ &&
      "topFrame must equal the top of stack");
  currentFrame_ = topFrame;
}

/// Set the current frame pointer to the current top of the stack and return
/// it.
/// \return the new value of the current frame pointer.
inline StackFramePtr Runtime::setCurrentFrameToTopOfStack() {
  return currentFrame_ = StackFramePtr(stackPointer_);
}

inline StackFramePtr Runtime::restoreStackAndPreviousFrame(
    StackFramePtr currentFrame) {
  assert(
      currentFrame_ == currentFrame &&
      "currentFrame parameter must match currentFrame_");
  stackPointer_ = currentFrame.ptr();
  return currentFrame_ = currentFrame.getPreviousFrame();
}

inline StackFramePtr Runtime::restoreStackAndPreviousFrame() {
  return restoreStackAndPreviousFrame(currentFrame_);
}

inline llvh::iterator_range<StackFrameIterator> Runtime::getStackFrames() {
  return {StackFrameIterator{currentFrame_},
          StackFrameIterator{registerStackEnd_}};
};

inline llvh::iterator_range<ConstStackFrameIterator> Runtime::getStackFrames()
    const {
  return {ConstStackFrameIterator{currentFrame_},
          ConstStackFrameIterator{registerStackEnd_}};
};

inline ExecutionStatus Runtime::setThrownValue(HermesValue value) {
  thrownValue_ = value;
  return ExecutionStatus::EXCEPTION;
}

inline void Runtime::clearThrownValue() {
  thrownValue_ = HermesValue::encodeEmptyValue();
}

inline CrashManager &Runtime::getCrashManager() {
  return *crashMgr_;
}

#ifndef HERMESVM_SANITIZE_HANDLES
inline void Runtime::potentiallyMoveHeap() {}
#endif

//===----------------------------------------------------------------------===//

/// Invoke the T constructor with the given args to construct the
/// object in the given memory mem.
template <typename T, typename... CtorArgs>
inline void constructInHeapObj(void *mem, CtorArgs... args) {
  new (mem) T(args...);
}

/// Serialize a SymbolID.
llvh::raw_ostream &operator<<(
    llvh::raw_ostream &OS,
    Runtime::FormatSymbolID format);

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_RUNTIME_H
