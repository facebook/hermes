/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_DEBUGGER_DEBUGGER_H
#define HERMES_VM_DEBUGGER_DEBUGGER_H

#ifdef HERMES_ENABLE_DEBUGGER

#include "hermes/BCGen/HBC/BytecodeInstructionGenerator.h"
#include "hermes/BCGen/HBC/DebugInfo.h"
#include "hermes/Inst/Inst.h"
#include "hermes/Public/DebuggerTypes.h"
#include "hermes/Support/OptValue.h"
#include "hermes/VM/Debugger/DebugCommand.h"
#include "hermes/VM/HermesValue.h"
#include "hermes/VM/InterpreterState.h"
#include "hermes/VM/RuntimeModule.h"
#include "llvh/ADT/DenseSet.h"
#include "llvh/ADT/MapVector.h"

#include <cstdint>
#include <string>

namespace hermes {
namespace vm {
class HermesValue;
class CodeBlock;
class Runtime;
} // namespace vm
} // namespace hermes

namespace hermes {
namespace vm {

/// Main debugger object that receives commands from an external source,
/// and passes them back to the interpreter loop, which handles them.
class Debugger {
 public:
  struct EvalResultMetadata;

  using DidPauseCallback = std::function<DebugCommand(
      InterpreterState state,
      ::facebook::hermes::debugger::PauseReason pauseReason,
      HermesValue evalResult,
      const EvalResultMetadata &evalResultMetadata,
      ::facebook::hermes::debugger::BreakpointID)>;

  using BreakpointResolvedCallback =
      std::function<void(facebook::hermes::debugger::BreakpointID)>;

 private:
  friend DebugCommand;

  using PauseOnThrowMode = ::facebook::hermes::debugger::PauseOnThrowMode;
  using BreakpointID = ::facebook::hermes::debugger::BreakpointID;
  using BreakpointInfo = ::facebook::hermes::debugger::BreakpointInfo;
  using CallFrameInfo = ::facebook::hermes::debugger::CallFrameInfo;
  using ExceptionDetails = ::facebook::hermes::debugger::ExceptionDetails;
  using PauseReason = ::facebook::hermes::debugger::PauseReason;
  using SourceLocation = ::facebook::hermes::debugger::SourceLocation;
  using StackTrace = ::facebook::hermes::debugger::StackTrace;
  using StepMode = ::facebook::hermes::debugger::StepMode;
  using String = ::facebook::hermes::debugger::String;
  using LexicalInfo = ::facebook::hermes::debugger::LexicalInfo;
  using ScriptID = ::facebook::hermes::debugger::ScriptID;
  using AsyncPauseKind = ::facebook::hermes::debugger::AsyncPauseKind;

  Runtime &runtime_;

  /// Function handling pauses.
  DidPauseCallback didPauseCallback_;

  /// Function handling breakpoint resolution.
  BreakpointResolvedCallback breakpointResolvedCallback_;

  /// Logical breakpoint.
  struct Breakpoint {
    CodeBlock *codeBlock;
    uint32_t offset;

    /// Whether the breakpoint is "enabled" from the user's perspective.
    /// Note that this is set independently of resolved.
    bool enabled;

    /// Condition on which the breakpoint should trigger.
    /// If empty, the breakpoint will always trigger at the location it's set.
    std::string condition{};

    /// Requested location of the breakpoint.
    SourceLocation requestedLocation;
    /// Resolved location of the breakpoint.
    /// Optionally filled in for resolved user breakpoints.
    llvh::Optional<SourceLocation> resolvedLocation{llvh::None};

    bool isResolved() const {
      return resolvedLocation.hasValue();
    }
  };

  /// Breakpoints that were set by the user.
  /// Every breakpoint has a unique ID that is never reused.
  /// Use a MapVector to iterate in insertion order.
  llvh::MapVector<BreakpointID, Breakpoint> userBreakpoints_{};
  BreakpointID nextBreakpointId_{1};

  /// One-shot breakpoints that are used for stepping commands.
  /// These are typically cleared immediately after breaking.
  std::vector<Breakpoint> tempBreakpoints_{};

  /// Physical breakpoint location.
  struct BreakpointLocation {
    /// Opcode that was replaced.
    hbc::opcode_atom_t opCode;

    /// If this location has a user breakpoint set,
    /// then this is set to the ID of the user breakpoint.
    /// Else, it's set to None.
    OptValue<BreakpointID> user{llvh::None};

    /// Whether this location has an on-load breakpoint set.
    bool onLoad{false};

    /// This is the set of callStackDepths at which we are supposed to stop
    /// on this breakpoint for Step breakpoints.
    /// The size of this set is the number of Step breakpoints.
    /// We enforce that there is only one Step breakpoint
    /// per physical location, because Step breakpoints
    /// should be set in one location and cleared immediately when hit.
    /// If a depth != 0, then only break when the runtime call stack
    /// has exactly depth elements in it, and if a depth == 0,
    /// then break on any runtime call stack size.
    llvh::DenseSet<uint32_t> callStackDepths{};

    BreakpointLocation(hbc::opcode_atom_t opCode) : opCode(opCode) {}

    /// Total number of logical breakpoints set at this location.
    uint32_t count() const {
      return callStackDepths.size() + (user ? 1 : 0) + (onLoad ? 1 : 0);
    }

    bool hasStepBreakpoint() const {
      return !callStackDepths.empty();
    }
  };

  llvh::DenseMap<const inst::Inst *, BreakpointLocation> breakpointLocations_{};

  /// The debugger is currently executing instructions.
  bool isDebugging_{false};

  /// If not None, the debugger was issued a STEP instruction of some sort,
  /// and it hasn't completed yet.
  /// The value indicates the type of step we're trying to take.
  OptValue<StepMode> curStepMode_{llvh::None};

  /// If true, all code blocks are breakpointed,
  /// and the debugger should stop on entering any code blocks.
  bool pauseOnAllCodeBlocks_{false};

  /// What conditions the debugger needs to stop on exceptions.
  PauseOnThrowMode pauseOnThrowMode_{PauseOnThrowMode::None};

  // When true, debugger is moving on after reporting an exception.
  // This is used to ensure we don't report an exception to the user twice
  // when pauseOnThrowMode_ is not None. The Interpreter can't distinguish
  // between exceptions thrown in native functions vs. thrown by users
  // when there's a native stack frame in the middle, so we need to account for
  // that by setting this flag to not stop between a caught exception
  // and its exception handler. This works because there should be no ability
  // to throw a new exception between reporting it and stepping to its handler.
  bool isUnwindingException_{false};

  /// Whether the Debugger should pause after a script has loaded, before it
  /// begins executing.
  bool pauseOnScriptLoad_{false};

  /// The last location we were at before we resumed execution in mid-step.
  /// This is only valid if curStepMode_ is not None.
  InterpreterState preStepState_{};

  // Whether an user has attached to any Inspector.
  // It is exposed to JS via a property %DebuggerInternal.isDebuggerAttached
  bool isDebuggerAttached_{false};

 public:
  explicit Debugger(Runtime &runtime) : runtime_(runtime) {}

  /// Reasons why the interpreter may invoke the debugger. Note this is a more
  /// limited set than PauseReason, because the Interpreter cannot distinguish
  /// between debugger opcodes as part of the Debugger command versus those
  /// inserted for breakpoints.
  enum class RunReason {
    /// The Interpreter hit a Debugger opcode.
    Opcode,

    /// The Interpreter hit an exception.
    Exception,

    /// The Interpreter is reacting to an async break request from the user.
    /// Any current stepping state should be cleared.
    AsyncBreakExplicit,

    /// The Interpreter is reacting to an async break request from the
    /// inspector.
    /// Any current stepping state should be *preserved*.
    AsyncBreakImplicit,
  };

  /// An EvalResultMetadata is a subset of EvalResult in DebuggerAPI.h, lacking
  /// a jsi::Value field.
  struct EvalResultMetadata {
    bool isException = false;
    ExceptionDetails exceptionDetails;
  };

  /// Runs debugger commands until execution continues.
  /// \param runReason the reason that the interpreter invoked the debugger.
  /// \param[in,out] state the debugging state.
  /// \return status after running the debugger.
  /// Ensures that if we stopped on an exception, the state doesn't change and
  /// the status is RETURNED.
  /// If the status is EXCEPTION, then the state is the state at the instruction
  /// that threw the exception - this allows the interpreter to handle the
  /// exception directly from the instruction that threw.
  ExecutionStatus runDebugger(RunReason runReason, InterpreterState &state);

  bool isDebugging() const {
    return isDebugging_;
  }

  // \return the stack trace for the state given by \p state.
  StackTrace getStackTrace(InterpreterState state) const;

  llvh::Optional<const BreakpointLocation> getBreakpointLocation(
      const inst::Inst *ip) const {
    auto it = breakpointLocations_.find(ip);
    if (it == breakpointLocations_.end()) {
      return llvh::None;
    }
    return {it->second};
  }
  llvh::Optional<const BreakpointLocation> getBreakpointLocation(
      CodeBlock *codeBlock,
      uint32_t offset) const;

  void setDidPauseCallback(DidPauseCallback callback) {
    didPauseCallback_ = std::move(callback);
  }

  void setBreakpointResolvedCallback(BreakpointResolvedCallback callback) {
    breakpointResolvedCallback_ = std::move(callback);
  }

  void setShouldPauseOnScriptLoad(bool flag) {
    pauseOnScriptLoad_ = flag;
  }

  bool getShouldPauseOnScriptLoad() const {
    return pauseOnScriptLoad_;
  }

  void setPauseOnThrowMode(PauseOnThrowMode mode) {
    pauseOnThrowMode_ = mode;
  }

  PauseOnThrowMode getPauseOnThrowMode() const {
    return pauseOnThrowMode_;
  }

  /// Sets the property %isDebuggerAttached in the %DebuggerInternal object.
  void setIsDebuggerAttached(bool isAttached) {
    isDebuggerAttached_ = isAttached;
  }

  /// Gets the property %isDebuggerAttached in the %DebuggerInternal object.
  bool getIsDebuggerAttached() const {
    return isDebuggerAttached_;
  }

  /// Signal to the debugger that we are done unwinding an exception.
  /// This means that we can begin reporting exceptions to the user again
  /// if the user has requested them.
  void finishedUnwindingException() {
    isUnwindingException_ = false;
  }

  /// Request an async pause. This may be called from any thread, or a signal
  /// handler.
  void triggerAsyncPause(AsyncPauseKind kind);

  /// Creates a user breakpoint given filename, line, and column.
  /// \param loc the location to set the breakpoint.
  /// \return the id of the new breakpoint, kInvalidBreakpoint if none was
  /// created.
  BreakpointID createBreakpoint(const SourceLocation &loc);

  /// Sets the condition on a breakpoint.
  /// \param id the breakpoint to change the condition on.
  /// \param condition if None, unset the condition, else set the condition.
  void setBreakpointCondition(BreakpointID id, std::string condition);

  /// Deletes the breakpoint given.
  void deleteBreakpoint(BreakpointID id);

  /// Deletes all breakpoints.
  void deleteAllBreakpoints();

  /// Enables/disables the breakpoint given.
  /// \param id the id of the breakpoint to edit.
  /// \param enable if true, enable breakpoint \p id.
  void setBreakpointEnabled(BreakpointID id, bool enable);

  /// \return the breakpoint information for breakpoint \p id.
  /// If the supplied id is not valid, then the result has id ==
  /// kInvalidBreakpoint.
  BreakpointInfo getBreakpointInfo(BreakpointID id) const {
    auto it = userBreakpoints_.find(id);
    if (it == userBreakpoints_.end()) {
      // Invalid input.
      BreakpointInfo result{};
      result.id = ::facebook::hermes::debugger::kInvalidBreakpoint;
      return result;
    }

    const auto &breakpoint = it->second;
    BreakpointInfo result{};
    result.id = id;
    result.enabled = breakpoint.enabled;
    result.resolved = breakpoint.isResolved();
    result.requestedLocation = breakpoint.requestedLocation;
    if (breakpoint.isResolved()) {
      result.resolvedLocation = *breakpoint.resolvedLocation;
    }
    return result;
  }

  /// \return a list of valid user breakpoint IDs.
  std::vector<BreakpointID> getBreakpoints() const {
    std::vector<BreakpointID> result{};
    result.reserve(userBreakpoints_.size());
    for (const auto &it : userBreakpoints_) {
      result.push_back(it.first);
    }
    return result;
  }

  /// \return the number of variables in the given frame \p frame.
  LexicalInfo getLexicalInfoInFrame(uint32_t frame) const;

  /// \return the variable at \p variableIndex at scope depth \p scopeDepth in
  /// frame \p frame. 0 is the topmost frame, corresponding to \p topBlock. If
  /// \p outName is not null, populate it with the variable's name.
  HermesValue getVariableInFrame(
      uint32_t frame,
      uint32_t scopeDepth,
      uint32_t variableIndex,
      std::string *outName = nullptr) const;

  /// \param frame the index of the frame, with 0 being the topmost.
  /// \return the 'this' value at \p frame.
  HermesValue getThisValue(uint32_t frame) const;

  /// Report to the debugger that the runtime will execute a module given by \p
  /// module. The debugger may propagate a pause to the client.
  void willExecuteModule(RuntimeModule *module, CodeBlock *codeBlock);

  /// Report to the debugger that the runtime will unload a RuntimeModule.
  /// The debugger should unresolve breakpoints in that module.
  void willUnloadModule(RuntimeModule *module);

  /// Report to the debugger that the runtime will execute a codeBlock given by
  /// \p codeBlock.
  /// For example, this is used to set up the codeBlock while stepping.
  void willEnterCodeBlock(CodeBlock *codeBlock) {
    if (LLVM_UNLIKELY(pauseOnAllCodeBlocks_)) {
      setEntryBreakpointForCodeBlock(codeBlock);
    }
  }

  /// Resolve any unresolved breakpoints.
  /// Intended for use following compilation or loading of a new function.
  /// \param codeBlock the code block that was compiled before this call.
  void resolveBreakpoints(CodeBlock *codeBlock);

  /// \return the source map URL for \p scriptId, empty string if non exists.
  String getSourceMappingUrl(ScriptID scriptId) const;

  /// Find the handler for an exception thrown at \p state.
  /// \return llvh::None if no handler is found, else return the state of the
  /// handler and the offset of its frame.
  llvh::Optional<std::pair<InterpreterState, uint32_t>> findCatchTarget(
      const InterpreterState &state) const;

  /// Attempt to resolve the \p filenameId to a script ID based on the table.
  /// \return the ScriptID of the given filenameId.
  ScriptID resolveScriptId(RuntimeModule *runtimeModule, uint32_t filenameId)
      const;

 private:
  /// The primary debugger command loop.
  ExecutionStatus debuggerLoop(
      InterpreterState &state,
      PauseReason pauseReason,
      BreakpointID breakpoint);

  /// Gets a BreakpointLocation from breakpointLocations_ if it exists
  /// for the given codeBlock and offset, else creates one.
  /// Used by other functions which should be called to set breakpoints.
  /// Installs a breakpoint at that location, doesn't modify it.
  /// \return the location at which the breakpoint was installed.
  BreakpointLocation &installBreakpoint(CodeBlock *codeBlock, uint32_t offset);

  /// Set a user breakpoint at \p offset in \p codeBlock.
  /// Increments the count at a breakpoint if it already exists.
  /// If the physical breakpoint isn't enabled yet, patches the debugger
  /// instruction in.
  /// Sets the breakpoint ID to \p id.
  void
  setUserBreakpoint(CodeBlock *codeBlock, uint32_t offset, BreakpointID id);

  /// Set a stepping breakpoint at \p offset in \p codeBlock.
  /// Increments the count at a breakpoint if it already exists.
  /// If the physical breakpoint isn't enabled yet, patches the debugger
  /// instruction in.
  /// The breakpoint is set to be a one-shot.
  /// If \p callStackDepth != 0, then the Interpreter should only
  /// break if the callStack is exactly \p callStackDepth frames long.
  /// If \p callStackDepth == 0, then the Step breakpoint should break
  /// regardless of the actual depth of the call stack.
  void setStepBreakpoint(
      CodeBlock *codeBlock,
      uint32_t offset,
      uint32_t callStackDepth);

  /// Set an on-load breakpoint at \p offset in \p codeBlock.
  /// Increments the count at a breakpoint if it already exists.
  /// If the physical breakpoint isn't enabled yet, patches the debugger
  /// instruction in.
  /// The breakpoint is set to be a one-shot.
  void setOnLoadBreakpoint(CodeBlock *codeBlock, uint32_t offset);

  /// Unset user breakpoint \p breakpoint.
  /// Removes the location from breakpointLocations_ if no longer needed.
  void unsetUserBreakpoint(const Breakpoint &breakpoint);

  /// Creates a step breakpoint at offset 0 of \p codeBlock,
  /// to be used while in the middle of a step and entering a
  /// codeBlock for the first time.
  /// Requires that getShouldPauseOnAllCodeBlocks is true.
  /// The breakpoint will clear on the next call to clearTempBreakpoints.
  void setEntryBreakpointForCodeBlock(CodeBlock *codeBlock);

  /// Add a Step breakpoint to the last interpreted function on the
  /// call stack, at the next offset after the saved return address.
  /// This allows stepping out of the currently executing function.
  void breakpointCaller();

  /// Add a Step breakpoint at the exception handler for the current state.
  /// Does nothing if the exception is uncaught.
  void breakpointExceptionHandler(const InterpreterState &state);

  /// Clear all the Step and OnLoad breakpoints that have been set.
  /// Uninstall the debugger instruction from any locations
  /// that no longer have any logical breakpoints.
  void clearTempBreakpoints();

  /// Steps a single instruction.
  /// Requires that the current instruction steps to somewhere else in the
  /// current function.
  /// If the current instruction is a call or a return, don't use this function.
  ExecutionStatus stepInstruction(InterpreterState &state);

  /// Evaluate \p src rooted in the stack frame specified in \p args. 0 is the
  /// topmost frame, corresponding to \p state. Populate \p outMetadata
  /// with metadata for the result.
  /// \return the resulting HermesValue.
  HermesValue evalInFrame(
      const EvalArgs &args,
      const std::string &src,
      const InterpreterState &state,
      EvalResultMetadata *outMetadata);

  /// Given that the runtime threw an exception, clear the thrown value, and
  /// populate the \p outMetadata. \return the thrown value.
  HermesValue getExceptionAsEvalResult(EvalResultMetadata *outMetadata);

  /// Requires that we're currently debugging.
  /// If the queue is empty, invoke the didPauseCallback_ (or command line,
  /// provided that stdin is interactive), with the given interpeter state \p
  /// state and pause reason \p pause.
  /// If the pause is EvalComplete, the evalResult is given in \p evalResult;
  /// othewrise the evalResult should be ignored.
  /// \return the next command to run in a paused interpreter loop.
  DebugCommand getNextCommand(
      InterpreterState state,
      PauseReason pauseReason,
      HermesValue evalResult,
      const EvalResultMetadata &evalMetadata,
      BreakpointID breakpoint) {
    // If we have a didPauseCallback, invoke it. If we don't have a callback,
    // fall back to simply continuing.
    if (didPauseCallback_)
      return didPauseCallback_(
          state, pauseReason, evalResult, evalMetadata, breakpoint);

    return DebugCommand::makeContinue();
  }

  /// \return true if the states are on different instructions within the same
  /// statement.
  /// Ensure different instructions to allow for stepping to the same
  /// statement within a loop.
  inline bool sameStatementDifferentInstruction(
      const InterpreterState &a,
      const InterpreterState &b) const {
    auto aLoc = getLocationForState(a);
    auto bLoc = getLocationForState(b);

    // Same statement in the same codeBlock, but different offsets.
    return a.codeBlock == b.codeBlock && aLoc->statement == bLoc->statement &&
        a.offset != b.offset;
  }

  OptValue<hbc::DebugSourceLocation> getLocationForState(
      const InterpreterState &state) const {
    return state.codeBlock->getSourceLocation(state.offset);
  }

  /// Attempt to resolve the requestedLocation in \p breakpoint.
  /// If successful, sets breakpoint.resolvedLocation to the resolved location.
  /// Requires that \p breakpoint is not yet resolved.
  /// \return true if the breakpoint is successfully resolved.
  bool resolveBreakpointLocation(Breakpoint &breakpoint) const;

  /// Unresolves the breakpoint.
  void unresolveBreakpointLocation(Breakpoint &breakpoint);

  /// \return a CallFrameInfo for a given code block \p codeBlock and IP offset
  /// into it \p ipOffset.
  CallFrameInfo getCallFrameInfo(const CodeBlock *codeBlock, uint32_t offset)
      const;

  /// Get the jump target for an instruction (if it is a jump).
  llvh::Optional<uint32_t> findJumpTarget(CodeBlock *block, uint32_t offset);

  /// Set breakpoints at all possible next instructions after the current one.
  void breakAtPossibleNextInstructions(InterpreterState &state);
};

} // namespace vm
} // namespace hermes

#endif // HERMES_ENABLE_DEBUGGER
#endif
