/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_STACKFRAME_H
#define HERMES_VM_STACKFRAME_H

#include "hermes/BCGen/HBC/StackFrameLayout.h"
#include "hermes/Support/Compiler.h"
#include "hermes/VM/Handle.h"
#include "hermes/VM/NativeArgs.h"

#include <iterator>
#include <type_traits>

namespace llvh {
class raw_ostream;
}

namespace hermes {
namespace inst {
struct Inst;
}
namespace vm {

class CodeBlock;

using hbc::StackFrameLayout;
using inst::Inst;

/// This class provides access to the standard fields associated with a stack
/// frame in the stack. It is from the point of view of a callee even though
/// many of the fields reside in the caller's stack frame. (See the
/// documentation of \c hbc::StackFrameLayout.)
template <bool isConst>
class StackFramePtrT {
  using QualifiedHV = typename std::
      conditional<isConst, const PinnedHermesValue, PinnedHermesValue>::type;
  using QualifiedCB =
      typename std::conditional<isConst, const CodeBlock, CodeBlock>::type;

  QualifiedHV *frame_;

 public:
  /// Default construct a null stack frame object.
  LLVM_ATTRIBUTE_ALWAYS_INLINE
  explicit StackFramePtrT() : frame_(nullptr) {}
  /// Construct from a pointer to the last register in the caller's frame.
  LLVM_ATTRIBUTE_ALWAYS_INLINE
  explicit StackFramePtrT(QualifiedHV *frame) : frame_(frame) {}

  /// Constructor from non-const to const.
  template <
      bool isOtherConst,
      typename = typename std::enable_if<!isOtherConst && isConst>::type>
  StackFramePtrT(const StackFramePtrT<isOtherConst> &other)
      : frame_(other.ptr()){};

  /// \return true if the frame pointer is non-null.
  LLVM_ATTRIBUTE_ALWAYS_INLINE
  explicit operator bool() const {
    return frame_;
  }

  LLVM_ATTRIBUTE_ALWAYS_INLINE
  bool operator==(StackFramePtrT o) const {
    return frame_ == o.frame_;
  }
  LLVM_ATTRIBUTE_ALWAYS_INLINE
  bool operator!=(StackFramePtrT o) const {
    return frame_ != o.frame_;
  }

  /// This method enables this class to be used as a return value from iterator
  /// arrow operator.
  LLVM_ATTRIBUTE_ALWAYS_INLINE
  StackFramePtrT *operator->() {
    return this;
  }
  LLVM_ATTRIBUTE_ALWAYS_INLINE
  const StackFramePtrT *operator->() const {
    return this;
  }

  /// \return a pointer to the register at the start of this frame. Technically
  /// it points to the last register of the previous frame.
  LLVM_ATTRIBUTE_ALWAYS_INLINE
  QualifiedHV *ptr() const {
    return frame_;
  }

#define _HERMESVM_DEFINE_STACKFRAME_REF(name) \
  LLVM_ATTRIBUTE_ALWAYS_INLINE                \
  QualifiedHV &get##name##Ref() const {       \
    return frame_[StackFrameLayout::name];    \
  }

  // Declare convenience accessors to the underlying HermesValue slots.
  _HERMESVM_DEFINE_STACKFRAME_REF(FirstLocal)
  _HERMESVM_DEFINE_STACKFRAME_REF(Scratch)
  _HERMESVM_DEFINE_STACKFRAME_REF(DebugEnvironment)
  _HERMESVM_DEFINE_STACKFRAME_REF(PreviousFrame)
  _HERMESVM_DEFINE_STACKFRAME_REF(SavedIP)
  _HERMESVM_DEFINE_STACKFRAME_REF(SavedCodeBlock)
  _HERMESVM_DEFINE_STACKFRAME_REF(ArgCount)
  _HERMESVM_DEFINE_STACKFRAME_REF(NewTarget)
  _HERMESVM_DEFINE_STACKFRAME_REF(CalleeClosureOrCB)
  _HERMESVM_DEFINE_STACKFRAME_REF(ThisArg)
  _HERMESVM_DEFINE_STACKFRAME_REF(FirstArg)

#undef _HERMESVM_DEFINE_STACKFRAME_REF

  /// \return a pointer to the register at the start of the previous stack
  /// frame.
  QualifiedHV *getPreviousFramePointer() const {
    return getPreviousFrameRef().template getNativePointer<PinnedHermesValue>();
  }

  /// \return the previous stack frame.
  StackFramePtrT<isConst> getPreviousFrame() const {
    return StackFramePtrT<isConst>{getPreviousFramePointer()};
  }

  /// \return the saved IP of the caller. Execution will continue there as soon
  /// as the execution of the current frame completes.
  const Inst *getSavedIP() const {
    return getSavedIPRef().template getNativePointer<const Inst>();
  }

  /// \return the saved CodeBlock of the caller. Execution will continue there
  /// as soon as the execution of the current frame completes.
  QualifiedCB *getSavedCodeBlock() const {
    return getSavedCodeBlockRef().template getNativePointer<CodeBlock>();
  }

  /// \return a handle holding the callee debug environment.
  /// The environment associated with the callee's stack frame, that is, the
  /// Environment created by the last CreateEnvironment instruction to execute
  /// in the callee's stack frame. It is null if debugging support is not
  /// present, or if no CreateEnvironment instruction has executed, which is
  /// possible if we are early in the code block, or with optimized code. This
  /// is stored in the call frame so that the debugger can gain access to the
  /// Environment at arbitrary frames. Note this is managed by the GC.
  inline Handle<Environment> getDebugEnvironmentHandle() const;

  /// \return the callee debug environment.
  /// The environment associated with the callee's stack frame, that is, the
  /// Environment created by the last CreateEnvironment instruction to execute
  /// in the callee's stack frame. It is null if debugging support is not
  /// present, or if no CreateEnvironment instruction has executed, which is
  /// possible if we are early in the code block, or with optimized code. This
  /// is stored in the call frame so that the debugger can gain access to the
  /// Environment at arbitrary frames. Note this is managed by the GC.
  inline Environment *getDebugEnvironment() const;

  /// \return the number of JavaScript arguments passed to the callee excluding
  /// \c "this".
  uint32_t getArgCount() const {
    return getArgCountRef().getNativeUInt32();
  }

  /// In very rare cases (namely in bound function calls) we need to be able to
  /// update the arg count in-place. This method does that, but it shouldn't
  /// normally be used.
  void setArgCount(uint32_t argCount) const {
    getArgCountRef() = HermesValue::encodeNativeUInt32(argCount);
  }

  /// \return a raw pointer to the JavaScript Function object representing the
  /// callee. This assumes that we know that it is a closure and not a
  /// CodeBlock *.
  inline Callable *getCalleeClosureUnsafe() const;

  /// \return a handle to the JavaScript Function object representing the
  /// callee. This assumes that we know that it is a closure and not a
  /// CodeBlock *.
  inline Handle<Callable> getCalleeClosureHandleUnsafe() const;

  /// \return the callee's CodeBlock, i.e. the CodeBlock that is executing in
  ///   this frame. It could be nullptr if calleeClosure is a Callable but not
  ///   a JSFunction.
  QualifiedCB *getCalleeCodeBlock(Runtime &runtime) const;

  /// \return true if this is a constructor being invoked by \c new.
  bool isConstructorCall() const {
    return !getNewTargetRef().isUndefined();
  }

  /// \return an iterator pointing to the first explicit argument.
  ArgIteratorT<isConst> argsBegin() const {
    return ArgIteratorT<isConst>(&getThisArgRef());
  }

  /// \return a reference to the register containing the N-th argument to the
  /// callee. -1 is this, 0 is the first explicit argument. It is an error to
  /// use a number greater or equal to \c getArgCount().
  QualifiedHV &getArgRef(int32_t n) const {
    assert(n >= -1 && n < (int64_t)getArgCount() && "invalid argument index");
    return argsBegin()[n];
  }

  /// Same as \c getArgRef() but allows to obtain a reference to one past the
  /// last argument.
  QualifiedHV &getArgRefUnsafe(int32_t n) const {
    return argsBegin()[n];
  }

  /// Initialize a new frame with the supplied values.
  /// \param calleeClosureOrCB a HermesValue which may not necessarily be of
  ///   the correct type. We use this occasionally when we want to initialize a
  ///   frame but delay the error checking. We never execute a frame with
  ///   the wrong type of callee though.
  /// \param newTarget `undefined` or the callable of the constructor being
  ///   invoked directly by `new`.
  LLVM_ATTRIBUTE_ALWAYS_INLINE
  static StackFramePtrT<false> initFrame(
      PinnedHermesValue *stackPointer,
      StackFramePtrT previousFrame,
      const Inst *savedIP,
      const CodeBlock *savedCodeBlock,
      uint32_t argCount,
      HermesValue calleeClosureOrCB,
      HermesValue newTarget) {
    stackPointer[StackFrameLayout::PreviousFrame] =
        HermesValue::encodeNativePointer(previousFrame.ptr());
    stackPointer[StackFrameLayout::SavedIP] =
        HermesValue::encodeNativePointer(savedIP);
    stackPointer[StackFrameLayout::SavedCodeBlock] =
        HermesValue::encodeNativePointer(savedCodeBlock);
    stackPointer[StackFrameLayout::ArgCount] =
        HermesValue::encodeNativeUInt32(argCount);
    stackPointer[StackFrameLayout::NewTarget] = newTarget;
    stackPointer[StackFrameLayout::CalleeClosureOrCB] = calleeClosureOrCB;

    return StackFramePtrT<false>{stackPointer};
  }

  /// Initialize a new frame with the supplied values.
  LLVM_ATTRIBUTE_ALWAYS_INLINE
  static StackFramePtrT<false> initFrame(
      PinnedHermesValue *stackPointer,
      StackFramePtrT previousFrame,
      const Inst *savedIP,
      const CodeBlock *savedCodeBlock,
      uint32_t argCount,
      Callable *calleeClosure,
      bool construct) {
    return initFrame(
        stackPointer,
        previousFrame,
        savedIP,
        savedCodeBlock,
        argCount,
        HermesValue::encodeObjectValue(calleeClosure),
        construct ? HermesValue::encodeObjectValue(calleeClosure)
                  : HermesValue::encodeUndefinedValue());
  }

  /// Create an instance of NativeArgs pointing to the arguments in this
  /// frame.
  NativeArgs getNativeArgs() const {
    return NativeArgs{argsBegin(), getArgCount(), &getNewTargetRef()};
  }
};

using StackFramePtr = StackFramePtrT<false>;
using ConstStackFramePtr = StackFramePtrT<true>;

/// Dump information about this frame to the supplied stream.
/// \param next the starting address of the next frame, so the size of the
///   frame can be calculated. if nullptr, size is not calculated.
void dumpStackFrame(
    ConstStackFramePtr frame,
    llvh::raw_ostream &OS,
    const PinnedHermesValue *next = nullptr);

/// Dump information about this frame to llvh::errs().
void dumpStackFrame(ConstStackFramePtr frame);
/// Dump information about this frame to llvh::errs(). This overload is only
/// needed for calls directly from teh debugger.
void dumpStackFrame(StackFramePtr frame);

static_assert(
    std::is_trivially_copyable<StackFramePtr>::value,
    "StackFramePtr must be trivially copyable");
static_assert(
    std::is_trivially_copyable<ConstStackFramePtr>::value,
    "ConstStackFramePtr must be trivially copyable");

/// Unidirectional iterator over stack frames, starting from the top-most
/// frame.
template <bool isConst>
class StackFrameIteratorT {
  using QualifiedHV = typename std::
      conditional<isConst, const PinnedHermesValue, PinnedHermesValue>::type;

  StackFramePtrT<isConst> frame_;

 public:
  using iterator_category = std::forward_iterator_tag;
  using value_type = StackFramePtrT<isConst>;
  using difference_type = int32_t;
  using pointer = StackFramePtrT<isConst>;
  using reference = StackFramePtrT<isConst>;

  /// To satisfy the requirements of a forward iterator, a default constructor
  /// initializing the iterator with a null stack frame.
  StackFrameIteratorT() : frame_() {}

  StackFrameIteratorT(QualifiedHV *frame) : frame_(frame) {}

  StackFrameIteratorT(StackFramePtrT<isConst> frame) : frame_(frame) {}

  /// Constructor from non-const to const.
  template <
      bool isOtherConst,
      typename = typename std::enable_if<!isOtherConst && isConst>::type>
  StackFrameIteratorT(const StackFrameIteratorT<isOtherConst> &other)
      : frame_(other.operator->()){};

  /// \return true if the iterator doesn't refer to the null frame.
  explicit operator bool() const {
    return frame_;
  }

  bool operator==(StackFrameIteratorT o) const {
    return frame_ == o.frame_;
  }
  bool operator!=(StackFrameIteratorT o) const {
    return frame_ != o.frame_;
  }

  StackFrameIteratorT &operator++() {
    frame_ = frame_.getPreviousFrame();
    return *this;
  }
  StackFrameIteratorT operator++(int) {
    auto res = *this;
    frame_ = frame_.getPreviousFrame();
    return res;
  }

  StackFramePtrT<isConst> operator->() const {
    return frame_;
  }
  StackFramePtrT<isConst> operator*() const {
    return frame_;
  }
};

using StackFrameIterator = StackFrameIteratorT<false>;
using ConstStackFrameIterator = StackFrameIteratorT<true>;

static_assert(
    std::is_trivially_copyable<StackFrameIterator>::value,
    "StackFrameIterator must be trivially copyable");
static_assert(
    std::is_trivially_copyable<ConstStackFrameIterator>::value,
    "ConstStackFrameIterator must be trivially copyable");

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_STACKFRAME_H
