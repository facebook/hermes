/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifdef HERMES_ENABLE_DEBUGGER

#include "hermes/VM/Debugger/Debugger.h"

#include "hermes/Support/UTF8.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/CodeBlock.h"
#include "hermes/VM/JSError.h"
#include "hermes/VM/JSLib.h"
#include "hermes/VM/Operations.h"
#include "hermes/VM/Profiler/SamplingProfiler.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/RuntimeModule.h"
#include "hermes/VM/StackFrame-inline.h"
#include "hermes/VM/StringView.h"
#pragma GCC diagnostic push

#ifdef HERMES_COMPILER_SUPPORTS_WSHORTEN_64_TO_32
#pragma GCC diagnostic ignored "-Wshorten-64-to-32"
#endif
#ifdef HERMES_ENABLE_DEBUGGER

namespace hermes {
namespace vm {

using namespace hermes::inst;
namespace fhd = ::facebook::hermes::debugger;

// These instructions won't recursively invoke the interpreter,
// and we also can't easily determine where they will jump to.
static inline bool shouldSingleStep(OpCode opCode) {
  return opCode == OpCode::Throw || opCode == OpCode::SwitchImm;
}

static StringView getFunctionName(
    Runtime &runtime,
    const CodeBlock *codeBlock) {
  auto functionName = codeBlock->getNameMayAllocate();
  if (functionName == Predefined::getSymbolID(Predefined::emptyString)) {
    functionName = Predefined::getSymbolID(Predefined::anonymous);
  }
  return runtime.getIdentifierTable().getStringView(runtime, functionName);
}

static std::string getFileNameAsUTF8(
    Runtime &runtime,
    RuntimeModule *runtimeModule,
    uint32_t filenameId) {
  const auto *debugInfo = runtimeModule->getBytecode()->getDebugInfo();
  return debugInfo->getFilenameByID(filenameId);
}

/// \return a scope chain containing the block and all its lexical parents,
/// including the global scope.
/// \return none if the scope chain is unavailable.
static llvh::Optional<ScopeChain> scopeChainForBlock(
    Runtime &runtime,
    const CodeBlock *cb) {
  OptValue<uint32_t> lexicalDataOffset = cb->getDebugLexicalDataOffset();
  if (!lexicalDataOffset)
    return llvh::None;

  ScopeChain scopeChain;
  RuntimeModule *runtimeModule = cb->getRuntimeModule();
  const hbc::BCProvider *bytecode = runtimeModule->getBytecode();
  const hbc::DebugInfo *debugInfo = bytecode->getDebugInfo();
  while (lexicalDataOffset) {
    GCScopeMarkerRAII marker{runtime};
    scopeChain.functions.emplace_back();
    auto &scopeItem = scopeChain.functions.back();
    // Append a new list to the chain.
    auto names = debugInfo->getVariableNames(*lexicalDataOffset);
    scopeItem.variables.insert(
        scopeItem.variables.end(), names.begin(), names.end());

    // Get the parent item.
    // Stop at the global block.
    auto parentId = debugInfo->getParentFunctionId(*lexicalDataOffset);
    if (!parentId)
      break;

    lexicalDataOffset = runtimeModule->getCodeBlockMayAllocate(*parentId)
                            ->getDebugLexicalDataOffset();

    if (!lexicalDataOffset) {
      // The function has a parent, but the parent doesn't have debug info.
      // This could happen when the parent is global.
      // "global" doesn't have a lexical parent.
      // "global" may have 0 variables, and may have no lexical info
      // (which is the case for synthesized parent scopes in lazy compilation).
      // In such case, BytecodeFunctionGenerator::hasDebugInfo returns false,
      // resulting in no debug offset for global in the bytecode.
      // Note that assert "*parentId == bytecode->getGlobalFunctionIndex()"
      // will fail because the getGlobalFunctionIndex() function returns
      // the entry point instead of the global function. The entry point
      // is not the same as the global function in the context of
      // lazy compilation.
      scopeChain.functions.emplace_back();
    }
  }
  return {std::move(scopeChain)};
}

void Debugger::triggerAsyncPause(AsyncPauseKind kind) {
  runtime_.triggerDebuggerAsyncBreak(kind);
}

llvh::Optional<uint32_t> Debugger::findJumpTarget(
    CodeBlock *block,
    uint32_t offset) {
  const Inst *ip = block->getOffsetPtr(offset);

#define DEFINE_JUMP_LONG_VARIANT(name, nameLong) \
  case OpCode::name: {                           \
    return offset + ip->i##name.op1;             \
  }                                              \
  case OpCode::nameLong: {                       \
    return offset + ip->i##nameLong.op1;         \
  }

  switch (ip->opCode) {
#include "hermes/BCGen/HBC/BytecodeList.def"
    default:
      return llvh::None;
  }
#undef DEFINE_JUMP_LONG_VARIANT
}

void Debugger::breakAtPossibleNextInstructions(InterpreterState &state) {
  auto nextOffset = state.codeBlock->getNextOffset(state.offset);
  // Set a breakpoint at the next instruction in the code block if this is not
  // the last instruction.
  if (nextOffset < state.codeBlock->getOpcodeArray().size()) {
    setStepBreakpoint(
        state.codeBlock, nextOffset, runtime_.getCurrentFrameOffset());
  }
  // If the instruction is a jump, set a break point at the possible
  // jump target; otherwise, only break at the next instruction.
  // This instruction could jump to itself, so this step should be after the
  // previous step (otherwise the Jmp will have been overwritten by a Debugger
  // inst, and we won't be able to find the target).
  //
  // Since we've already set a breakpoint on the next instruction, we can
  // skip the case where that is also the jump target.
  auto jumpTarget = findJumpTarget(state.codeBlock, state.offset);
  if (jumpTarget.hasValue() && jumpTarget.getValue() != nextOffset) {
    setStepBreakpoint(
        state.codeBlock,
        jumpTarget.getValue(),
        runtime_.getCurrentFrameOffset());
  }
}

ExecutionStatus Debugger::runDebugger(
    Debugger::RunReason runReason,
    InterpreterState &state) {
  assert(!isDebugging_ && "can't run debugger while debugging is in progress");
  isDebugging_ = true;

  // We're going to derive a PauseReason to pass to the event observer. OptValue
  // is used to check our logic which is rather complicated.
  OptValue<PauseReason> pauseReason;

  // If the pause reason warrants it, this is set to be a valid breakpoint ID.
  BreakpointID breakpoint = fhd::kInvalidBreakpoint;

  if (runReason == RunReason::Exception) {
    // We hit an exception, report that we broke because of this.
    if (isUnwindingException_) {
      // We're currently unwinding an exception, so don't stop here
      // because we must have already reported the exception.
      isDebugging_ = false;
      return ExecutionStatus::EXCEPTION;
    }
    isUnwindingException_ = true;
    clearTempBreakpoints();
    pauseReason = PauseReason::Exception;
  } else if (runReason == RunReason::AsyncBreakImplicit) {
    if (curStepMode_.hasValue()) {
      // Avoid draining the queue or corrupting step state.
      isDebugging_ = false;
      return ExecutionStatus::RETURNED;
    }
    pauseReason = PauseReason::AsyncTrigger;
  } else if (runReason == RunReason::AsyncBreakExplicit) {
    // The user requested an async break, so we can clear stepping state
    // with the knowledge that the inspector isn't sending an immediate
    // continue.
    if (curStepMode_) {
      clearTempBreakpoints();
      curStepMode_ = llvh::None;
    }
    pauseReason = PauseReason::AsyncTrigger;
  } else {
    assert(runReason == RunReason::Opcode && "Unknown run reason");
    // First, check if we have to finish a step that's in progress.
    auto breakpointOpt = getBreakpointLocation(state.codeBlock, state.offset);
    if (breakpointOpt.hasValue() &&
        (breakpointOpt->hasStepBreakpoint() || breakpointOpt->onLoad)) {
      // We've hit a Step, which must mean we were stepping, or
      // pause-on-load if it's the first instruction of the global function.
      if (breakpointOpt->onLoad) {
        pauseReason = PauseReason::ScriptLoaded;
        clearTempBreakpoints();
      } else if (
          breakpointOpt->callStackDepths.count(0) ||
          breakpointOpt->callStackDepths.count(
              runtime_.getCurrentFrameOffset())) {
        // This is in fact a temp breakpoint we want to stop on right now.
        assert(curStepMode_ && "no step to finish");
        clearTempBreakpoints();
        auto locationOpt = getLocationForState(state);

        if (*curStepMode_ == StepMode::Into ||
            *curStepMode_ == StepMode::Over) {
          // If we're not stepping out, then we need to finish the step
          // in progress.
          // Otherwise, we just need to stop at the breakpoint site.
          while (!locationOpt.hasValue() || locationOpt->statement == 0 ||
                 sameStatementDifferentInstruction(state, preStepState_)) {
            // Move to the next source location.
            OpCode curCode = state.codeBlock->getOpCode(state.offset);

            if (curCode == OpCode::Ret) {
              // We're stepping out now.
              breakpointCaller();
              pauseOnAllCodeBlocks_ = true;
              curStepMode_ = StepMode::Out;
              isDebugging_ = false;
              return ExecutionStatus::RETURNED;
            }

            // These instructions won't recursively invoke the interpreter,
            // and we also can't easily determine where they will jump to,
            // so use single-step mode.
            if (shouldSingleStep(curCode)) {
              ExecutionStatus status = stepInstruction(state);
              if (status == ExecutionStatus::EXCEPTION) {
                isDebugging_ = false;
                return status;
              }
              locationOpt = getLocationForState(state);
              continue;
            }

            // Set a breakpoint at the next instruction and continue.
            breakAtPossibleNextInstructions(state);
            if (*curStepMode_ == StepMode::Into) {
              pauseOnAllCodeBlocks_ = true;
            }
            isDebugging_ = false;
            return ExecutionStatus::RETURNED;
          }
        }

        // Done stepping.
        curStepMode_ = llvh::None;
        pauseReason = PauseReason::StepFinish;
      } else {
        // We don't want to stop on this Step breakpoint.
        isDebugging_ = false;
        return ExecutionStatus::RETURNED;
      }
    } else {
      auto checkBreakpointCondition =
          [&](const std::string &condition) -> bool {
        if (condition.empty()) {
          // The empty condition is considered unset,
          // and we always pause on such breakpoints.
          return true;
        }
        EvalResultMetadata metadata;
        EvalArgs args;
        args.frameIdx = 0;
        // No handle here - we will only pass the value to toBoolean,
        // and no allocations should occur until then.
        HermesValue conditionResult =
            evalInFrame(args, condition, state, &metadata);
        NoAllocScope noAlloc(runtime_);
        if (metadata.isException) {
          // Ignore exceptions.
          // Cleanup is done by evalInFrame.
          return false;
        }
        noAlloc.release();
        return toBoolean(conditionResult);
      };

      // We've stopped on either a user breakpoint or a debugger statement.
      // Note: if we've stopped on both (breakpoint set on a debugger statement)
      // then we only report the breakpoint and move past it,
      // ignoring the debugger statement.
      if (breakpointOpt.hasValue()) {
        assert(
            breakpointOpt->user.hasValue() &&
            "must be stopped on a user breakpoint");
        const auto &condition =
            userBreakpoints_[*breakpointOpt->user].condition;
        if (checkBreakpointCondition(condition)) {
          pauseReason = PauseReason::Breakpoint;
          breakpoint = *(breakpointOpt->user);
        } else {
          isDebugging_ = false;
          return ExecutionStatus::RETURNED;
        }
      } else {
        pauseReason = PauseReason::DebuggerStatement;
      }

      // Stop stepping immediately.
      if (curStepMode_) {
        // If we're in a step, then the client still thinks we're debugging,
        // so just clear the status and clear the temp breakpoints.
        curStepMode_ = llvh::None;
        clearTempBreakpoints();
      }
    }
  }

  assert(pauseReason.hasValue() && "runDebugger failed to set PauseReason");
  return debuggerLoop(state, *pauseReason, breakpoint);
}

ExecutionStatus Debugger::debuggerLoop(
    InterpreterState &state,
    PauseReason pauseReason,
    BreakpointID breakpoint) {
  const InterpreterState startState = state;
  const bool startException = pauseReason == PauseReason::Exception;
  EvalResultMetadata evalResultMetadata;
  CallResult<InterpreterState> result{ExecutionStatus::EXCEPTION};
  GCScope gcScope{runtime_};
  MutableHandle<> evalResult{runtime_};
  // Keep the evalResult alive, even if all other handles are flushed.
  static constexpr unsigned KEEP_HANDLES = 1;
  SuspendSamplingProfilerRAII ssp{runtime_, "debugger"};
  while (true) {
    GCScopeMarkerRAII marker{runtime_};
    auto command = getNextCommand(
        state, pauseReason, *evalResult, evalResultMetadata, breakpoint);
    evalResult.clear();
    switch (command.type) {
      case DebugCommandType::NONE:
        break;
      case DebugCommandType::CONTINUE:
        isDebugging_ = false;
        curStepMode_ = llvh::None;
        return ExecutionStatus::RETURNED;
      case DebugCommandType::EVAL:
        evalResult = evalInFrame(
            command.evalArgs, command.text, startState, &evalResultMetadata);
        pauseReason = PauseReason::EvalComplete;
        break;
      case DebugCommandType::STEP: {
        // If we pause again in this function, it will be due to a step.
        pauseReason = PauseReason::StepFinish;
        const StepMode stepMode = command.stepArgs.mode;
        // We should only be able to step from instructions with recorded
        // locations.
        const auto startLocationOpt = getLocationForState(state);
        (void)startLocationOpt;
        assert(
            startLocationOpt.hasValue() &&
            "starting step from a location without debug info");
        preStepState_ = state;
        if (stepMode == StepMode::Into || stepMode == StepMode::Over) {
          if (startException) {
            // Paused because of a throw or we're about to throw.
            // Breakpoint the handler if it's there, and continue.
            breakpointExceptionHandler(state);
            isDebugging_ = false;
            curStepMode_ = stepMode;
            return ExecutionStatus::RETURNED;
          }
          while (true) {
            // NOTE: this loop doesn't actually allocate any handles presently,
            // but it could, and clearing all handles is really cheap.
            gcScope.flushToSmallCount(KEEP_HANDLES);
            OpCode curCode = state.codeBlock->getOpCode(state.offset);

            if (curCode == OpCode::Ret) {
              breakpointCaller();
              pauseOnAllCodeBlocks_ = true;
              isDebugging_ = false;
              // Equivalent to a step out.
              curStepMode_ = StepMode::Out;
              return ExecutionStatus::RETURNED;
            }

            // These instructions won't recursively invoke the interpreter,
            // and we also can't easily determine where they will jump to,
            // so use single-step mode.
            if (shouldSingleStep(curCode)) {
              ExecutionStatus status = stepInstruction(state);
              if (status == ExecutionStatus::EXCEPTION) {
                breakpointExceptionHandler(state);
                isDebugging_ = false;
                curStepMode_ = stepMode;
                return status;
              }
              auto locationOpt = getLocationForState(state);
              if (locationOpt.hasValue() && locationOpt->statement != 0 &&
                  !sameStatementDifferentInstruction(state, preStepState_)) {
                // We've moved on from the statement that was executing.
                break;
              }
              continue;
            }

            // Set a breakpoint at the next instruction and continue.
            // If there is a user installed breakpoint, we need to temporarily
            // uninstall the breakpoint so that we can get the correct
            // offset for the next instruction.
            auto breakpointOpt =
                getBreakpointLocation(state.codeBlock, state.offset);
            if (breakpointOpt) {
              state.codeBlock->uninstallBreakpointAtOffset(
                  state.offset, breakpointOpt->opCode);
            }
            breakAtPossibleNextInstructions(state);
            if (breakpointOpt) {
              state.codeBlock->installBreakpointAtOffset(state.offset);
            }
            if (stepMode == StepMode::Into) {
              // Stepping in could enter another code block,
              // so handle that by breakpointing all code blocks.
              pauseOnAllCodeBlocks_ = true;
            }
            isDebugging_ = false;
            curStepMode_ = stepMode;
            return ExecutionStatus::RETURNED;
          }
        } else {
          ExecutionStatus status;
          if (startException) {
            breakpointExceptionHandler(state);
            status = ExecutionStatus::EXCEPTION;
          } else {
            breakpointCaller();
            status = ExecutionStatus::RETURNED;
          }
          // Stepping out of here is the same as continuing.
          isDebugging_ = false;
          curStepMode_ = StepMode::Out;
          return status;
        }
        break;
      }
    }
  }
}

void Debugger::willExecuteModule(RuntimeModule *module, CodeBlock *codeBlock) {
  // This function should only be called on the main RuntimeModule and not on
  // any "child" RuntimeModules it may create through lazy compilation.
  assert(
      module == module->getLazyRootModule() &&
      "Expected to only run on lazy root module");

  if (!getShouldPauseOnScriptLoad())
    return;
  // We want to pause on the first instruction of this module.
  // Add a breakpoint on the first opcode of its global function.
  auto globalFunctionIndex = module->getBytecode()->getGlobalFunctionIndex();
  auto globalCode = module->getCodeBlockMayAllocate(globalFunctionIndex);
  setOnLoadBreakpoint(globalCode, 0);
}

void Debugger::willUnloadModule(RuntimeModule *module) {
  if (tempBreakpoints_.size() == 0 && userBreakpoints_.size() == 0) {
    return;
  }

  llvh::DenseSet<CodeBlock *> unloadingBlocks;
  for (auto *block : module->getFunctionMap()) {
    if (block) {
      unloadingBlocks.insert(block);
    }
  }

  for (auto &bp : userBreakpoints_) {
    if (unloadingBlocks.count(bp.second.codeBlock)) {
      unresolveBreakpointLocation(bp.second);
    }
  }

  auto cleanTempBreakpoint = [&](Breakpoint &bp) {
    if (!unloadingBlocks.count(bp.codeBlock))
      return false;

    auto *ptr = bp.codeBlock->getOffsetPtr(bp.offset);
    auto it = breakpointLocations_.find(ptr);
    if (it != breakpointLocations_.end()) {
      auto &location = it->second;
      assert(!location.user.hasValue() && "Unexpected user breakpoint");
      bp.codeBlock->uninstallBreakpointAtOffset(bp.offset, location.opCode);
      breakpointLocations_.erase(it);
    }
    return true;
  };

  tempBreakpoints_.erase(
      std::remove_if(
          tempBreakpoints_.begin(),
          tempBreakpoints_.end(),
          cleanTempBreakpoint),
      tempBreakpoints_.end());
}

void Debugger::resolveBreakpoints(CodeBlock *codeBlock) {
  for (auto &it : userBreakpoints_) {
    auto &breakpoint = it.second;
    if (!breakpoint.isResolved()) {
      resolveBreakpointLocation(breakpoint);
      if (breakpoint.isResolved() && breakpoint.enabled) {
        setUserBreakpoint(breakpoint.codeBlock, breakpoint.offset, it.first);
        if (breakpointResolvedCallback_) {
          breakpointResolvedCallback_(it.first);
        }
      }
    }
  }
}

auto Debugger::getCallFrameInfo(const CodeBlock *codeBlock, uint32_t ipOffset)
    const -> CallFrameInfo {
  GCScopeMarkerRAII marker{runtime_};
  CallFrameInfo frameInfo;
  if (!codeBlock) {
    frameInfo.functionName = "(native)";
  } else {
    // The caller doesn't expect that this function is allocating new handles,
    // so make sure we aren't.
    GCScopeMarkerRAII gcMarker{runtime_};

    llvh::SmallVector<char16_t, 64> storage;
    UTF16Ref functionName =
        getFunctionName(runtime_, codeBlock).getUTF16Ref(storage);
    convertUTF16ToUTF8WithReplacements(frameInfo.functionName, functionName);
    auto locationOpt = codeBlock->getSourceLocation(ipOffset);
    if (locationOpt) {
      frameInfo.location.line = locationOpt->line;
      frameInfo.location.column = locationOpt->column;
      frameInfo.location.fileId = resolveScriptId(
          codeBlock->getRuntimeModule(), locationOpt->filenameId);
      frameInfo.location.fileName = getFileNameAsUTF8(
          runtime_, codeBlock->getRuntimeModule(), locationOpt->filenameId);
    }
  }
  return frameInfo;
}

auto Debugger::getStackTrace(InterpreterState state) const -> StackTrace {
  using fhd::CallFrameInfo;
  GCScopeMarkerRAII marker{runtime_};
  MutableHandle<> displayName{runtime_};
  MutableHandle<JSObject> propObj{runtime_};
  std::vector<CallFrameInfo> frames;
  // Note that we are iterating backwards from the top.
  // Also note that each frame saves its caller's code block and IP. The initial
  // one comes from the paused state.
  const CodeBlock *codeBlock = state.codeBlock;
  uint32_t ipOffset = state.offset;
  GCScopeMarkerRAII marker2{runtime_};
  for (auto cf : runtime_.getStackFrames()) {
    marker2.flush();
    CallFrameInfo frameInfo = getCallFrameInfo(codeBlock, ipOffset);
    if (auto callableHandle = Handle<Callable>::dyn_vmcast(
            Handle<>(&cf.getCalleeClosureOrCBRef()))) {
      NamedPropertyDescriptor desc;
      propObj = JSObject::getNamedDescriptorPredefined(
          callableHandle, runtime_, Predefined::displayName, desc);
      if (propObj) {
        auto displayNameRes = JSObject::getNamedSlotValue(
            createPseudoHandle(*propObj), runtime_, desc);
        if (LLVM_UNLIKELY(displayNameRes == ExecutionStatus::EXCEPTION)) {
          displayName = HermesValue::encodeUndefinedValue();
        } else {
          displayName = std::move(*displayNameRes);
          if (displayName->isString()) {
            llvh::SmallVector<char16_t, 64> storage;
            displayName->getString()->appendUTF16String(storage);
            convertUTF16ToUTF8WithReplacements(frameInfo.functionName, storage);
          }
        }
      }
    }
    frames.push_back(frameInfo);

    codeBlock = cf.getSavedCodeBlock();
    const Inst *const savedIP = cf.getSavedIP();
    if (!codeBlock && savedIP) {
      // If we have a saved IP but no saved code block, this was a bound call.
      // Go up one frame and get the callee code block but use the current
      // frame's saved IP.
      StackFramePtr prev = cf->getPreviousFrame();
      assert(prev && "bound function calls must have a caller");
      if (CodeBlock *parentCB = prev->getCalleeCodeBlock(runtime_)) {
        codeBlock = parentCB;
      }
    }

    ipOffset = (codeBlock && savedIP) ? codeBlock->getOffsetOf(savedIP) : 0;
  }
  return StackTrace(std::move(frames));
}

auto Debugger::createBreakpoint(const SourceLocation &loc) -> BreakpointID {
  using fhd::kInvalidBreakpoint;

  OptValue<hbc::DebugSearchResult> locationOpt{llvh::None};

  Breakpoint breakpoint{};
  breakpoint.requestedLocation = loc;
  // Breakpoints are enabled by default.
  breakpoint.enabled = true;
  bool resolved = resolveBreakpointLocation(breakpoint);

  BreakpointID breakpointId;
  if (resolved) {
    auto breakpointLoc =
        getBreakpointLocation(breakpoint.codeBlock, breakpoint.offset);
    if (breakpointLoc.hasValue() && breakpointLoc->user) {
      // Don't set duplicate user breakpoint.
      return kInvalidBreakpoint;
    }

    breakpointId = nextBreakpointId_++;
    setUserBreakpoint(breakpoint.codeBlock, breakpoint.offset, breakpointId);
  } else {
    breakpointId = nextBreakpointId_++;
  }

  userBreakpoints_[breakpointId] = std::move(breakpoint);

  return breakpointId;
}

void Debugger::setBreakpointCondition(BreakpointID id, std::string condition) {
  auto it = userBreakpoints_.find(id);

  if (it == userBreakpoints_.end()) {
    return;
  }

  auto &breakpoint = it->second;
  breakpoint.condition = std::move(condition);
}

void Debugger::deleteBreakpoint(BreakpointID id) {
  auto it = userBreakpoints_.find(id);

  if (it == userBreakpoints_.end()) {
    return;
  }

  auto &breakpoint = it->second;
  if (breakpoint.enabled && breakpoint.isResolved()) {
    unsetUserBreakpoint(breakpoint);
  }
  userBreakpoints_.erase(it);
}

void Debugger::deleteAllBreakpoints() {
  for (auto &it : userBreakpoints_) {
    auto &breakpoint = it.second;
    if (breakpoint.enabled && breakpoint.isResolved()) {
      unsetUserBreakpoint(breakpoint);
    }
  }
  userBreakpoints_.clear();
}

void Debugger::setBreakpointEnabled(BreakpointID id, bool enable) {
  auto it = userBreakpoints_.find(id);

  if (it == userBreakpoints_.end()) {
    return;
  }

  auto &breakpoint = it->second;
  if (enable && !breakpoint.enabled) {
    breakpoint.enabled = true;
    if (breakpoint.isResolved()) {
      setUserBreakpoint(breakpoint.codeBlock, breakpoint.offset, id);
    }
  } else if (!enable && breakpoint.enabled) {
    breakpoint.enabled = false;
    if (breakpoint.isResolved()) {
      unsetUserBreakpoint(breakpoint);
    }
  }
}

llvh::Optional<const Debugger::BreakpointLocation>
Debugger::getBreakpointLocation(CodeBlock *codeBlock, uint32_t offset) const {
  return getBreakpointLocation(codeBlock->getOffsetPtr(offset));
}

auto Debugger::installBreakpoint(CodeBlock *codeBlock, uint32_t offset)
    -> BreakpointLocation & {
  auto opcodes = codeBlock->getOpcodeArray();
  assert(offset < opcodes.size() && "invalid offset to set breakpoint");
  auto &location =
      breakpointLocations_
          .try_emplace(codeBlock->getOffsetPtr(offset), opcodes[offset])
          .first->second;
  if (location.count() == 0) {
    // count used to be 0, so patch this in now that the count > 0.
    codeBlock->installBreakpointAtOffset(offset);
  }
  return location;
}

void Debugger::setUserBreakpoint(
    CodeBlock *codeBlock,
    uint32_t offset,
    BreakpointID id) {
  BreakpointLocation &location = installBreakpoint(codeBlock, offset);
  location.user = id;
}

void Debugger::setStepBreakpoint(
    CodeBlock *codeBlock,
    uint32_t offset,
    uint32_t callStackDepth) {
  BreakpointLocation &location = installBreakpoint(codeBlock, offset);
  // Leave the resolved location empty for now,
  // let the caller fill it in lazily.
  Breakpoint breakpoint{};
  breakpoint.codeBlock = codeBlock;
  breakpoint.offset = offset;
  breakpoint.enabled = true;
  assert(
      location.callStackDepths.count(callStackDepth) == 0 &&
      "can't set duplicate Step breakpoint");
  location.callStackDepths.insert(callStackDepth);
  tempBreakpoints_.push_back(breakpoint);
}

void Debugger::setOnLoadBreakpoint(CodeBlock *codeBlock, uint32_t offset) {
  BreakpointLocation &location = installBreakpoint(codeBlock, offset);
  // Leave the resolved location empty for now,
  // let the caller fill it in lazily.
  Breakpoint breakpoint{};
  breakpoint.codeBlock = codeBlock;
  breakpoint.offset = offset;
  breakpoint.enabled = true;
  assert(!location.onLoad && "can't set duplicate on-load breakpoint");
  location.onLoad = true;
  tempBreakpoints_.push_back(breakpoint);
  assert(location.count() && "invalid count following set breakpoint");
}

void Debugger::unsetUserBreakpoint(const Breakpoint &breakpoint) {
  CodeBlock *codeBlock = breakpoint.codeBlock;
  uint32_t offset = breakpoint.offset;

  auto opcodes = codeBlock->getOpcodeArray();
  (void)opcodes;
  assert(offset < opcodes.size() && "invalid offset to set breakpoint");

  const Inst *offsetPtr = codeBlock->getOffsetPtr(offset);

  auto locIt = breakpointLocations_.find(offsetPtr);
  assert(
      locIt != breakpointLocations_.end() &&
      "can't unset a non-existent breakpoint");

  auto &location = locIt->second;

  assert(location.user && "no user breakpoints to unset");
  location.user = llvh::None;
  if (location.count() == 0) {
    // No more reason to keep this location around.
    // Unpatch it from the opcode stream and delete it from the map.
    codeBlock->uninstallBreakpointAtOffset(offset, location.opCode);
    breakpointLocations_.erase(offsetPtr);
  }
}

void Debugger::setEntryBreakpointForCodeBlock(CodeBlock *codeBlock) {
  assert(!codeBlock->isLazy() && "can't set breakpoint on a lazy codeblock");
  assert(
      pauseOnAllCodeBlocks_ && "can't set temp breakpoint while not stepping");
  setStepBreakpoint(codeBlock, 0, 0);
}

void Debugger::breakpointCaller() {
  auto callFrames = runtime_.getStackFrames();

  assert(callFrames.begin() != callFrames.end() && "empty call stack");

  // Go through the callStack backwards to find the first place we can break.
  auto frameIt = callFrames.begin();
  const Inst *ip = nullptr;
  for (; frameIt != callFrames.end(); ++frameIt) {
    ip = frameIt->getSavedIP();
    if (ip) {
      break;
    }
  }
  if (!ip) {
    return;
  }
  // If the ip was saved in the stack frame, the caller is the function
  // that we want to return to. The code block might not be saved in this
  // frame, so we need to find that in the frame below.
  do {
    frameIt++;
    assert(
        frameIt != callFrames.end() &&
        "The frame that has saved ip cannot be the bottom frame");
  } while (!frameIt->getCalleeCodeBlock(runtime_));
  // In the frame below, the 'calleeClosureORCB' register contains
  // the code block we need.
  CodeBlock *codeBlock = frameIt->getCalleeCodeBlock(runtime_);
  assert(codeBlock && "The code block must exist since we have ip");
  // Track the call stack depth that the breakpoint would be set on.
  uint32_t offset = codeBlock->getNextOffset(codeBlock->getOffsetOf(ip));
  setStepBreakpoint(codeBlock, offset, runtime_.calcFrameOffset(frameIt));
}

void Debugger::breakpointExceptionHandler(const InterpreterState &state) {
  auto target = findCatchTarget(state);
  if (!target) {
    return;
  }
  auto *codeBlock = target->first.codeBlock;
  auto offset = target->first.offset;
  setStepBreakpoint(codeBlock, offset, target->second);
}

void Debugger::clearTempBreakpoints() {
  llvh::SmallVector<const Inst *, 4> toErase{};
  for (const auto &breakpoint : tempBreakpoints_) {
    auto *codeBlock = breakpoint.codeBlock;
    auto offset = breakpoint.offset;
    const Inst *inst = codeBlock->getOffsetPtr(offset);
    auto it = breakpointLocations_.find(inst);
    if (it == breakpointLocations_.end()) {
      continue;
    }
    auto &location = it->second;
    if (location.count()) {
      location.callStackDepths.clear();
      location.onLoad = false;
      if (location.count() == 0) {
        codeBlock->uninstallBreakpointAtOffset(offset, location.opCode);
        toErase.push_back(inst);
      }
    }
  }
  for (const Inst *inst : toErase) {
    breakpointLocations_.erase(inst);
  }
  tempBreakpoints_.clear();
  pauseOnAllCodeBlocks_ = false;
}

ExecutionStatus Debugger::stepInstruction(InterpreterState &state) {
  auto *codeBlock = state.codeBlock;
  uint32_t offset = state.offset;
  assert(
      codeBlock->getOpCode(offset) != OpCode::Ret &&
      "can't stepInstruction in Ret, use step-out semantics instead");
  assert(
      shouldSingleStep(codeBlock->getOpCode(offset)) &&
      "can't stepInstruction through Call, use step-in semantics instead");
  auto locationOpt = getBreakpointLocation(codeBlock, offset);
  ExecutionStatus status;
  InterpreterState newState{state};
  if (locationOpt.hasValue()) {
    // Temporarily uninstall the breakpoint so we can run the real instruction.
    codeBlock->uninstallBreakpointAtOffset(offset, locationOpt->opCode);
    status = runtime_.stepFunction(newState);
    codeBlock->installBreakpointAtOffset(offset);
  } else {
    status = runtime_.stepFunction(newState);
  }

  if (status != ExecutionStatus::EXCEPTION)
    state = newState;
  return status;
}

auto Debugger::getLexicalInfoInFrame(uint32_t frame) const -> LexicalInfo {
  auto frameInfo = runtime_.stackFrameInfoByIndex(frame);
  assert(frameInfo && "Invalid frame");

  LexicalInfo result;
  if (frameInfo->isGlobal) {
    // Globals not yet supported.
    // TODO: support them. For now we have an empty entry for the global scope.
    result.variableCountsByScope_.push_back(0);
    return result;
  }
  const CodeBlock *cb = frameInfo->frame->getCalleeCodeBlock(runtime_);
  if (!cb) {
    // Native functions have no saved code block.
    result.variableCountsByScope_.push_back(0);
    return result;
  }

  auto scopeChain = scopeChainForBlock(runtime_, cb);
  if (!scopeChain) {
    // Binary was compiled without variable debug info.
    result.variableCountsByScope_.push_back(0);
    return result;
  }

  for (const auto &func : scopeChain->functions) {
    result.variableCountsByScope_.push_back(func.variables.size());
  }
  return result;
}

HermesValue Debugger::getVariableInFrame(
    uint32_t frame,
    uint32_t scopeDepth,
    uint32_t variableIndex,
    std::string *outName) const {
  GCScope gcScope{runtime_};
  auto frameInfo = runtime_.stackFrameInfoByIndex(frame);
  assert(frameInfo && "Invalid frame");

  const HermesValue undefined = HermesValue::encodeUndefinedValue();

  // Clear the outgoing info so we don't leave stale data there.
  if (outName)
    outName->clear();

  if (frameInfo->isGlobal) {
    // Globals not yet supported.
    // TODO: support them.
    return undefined;
  }
  const CodeBlock *cb = frameInfo->frame->getCalleeCodeBlock(runtime_);
  assert(cb && "Unexpectedly null code block");
  auto scopeChain = scopeChainForBlock(runtime_, cb);
  if (!scopeChain) {
    // Binary was compiled without variable debug info.
    return undefined;
  }

  const ScopeChainItem &item = scopeChain->functions.at(scopeDepth);
  if (outName)
    *outName = item.variables.at(variableIndex);

  // Descend the environment chain to the desired depth, or stop at null.
  // We may get a null environment if it has not been created.
  MutableHandle<Environment> env(
      runtime_, frameInfo->frame->getDebugEnvironment());
  for (uint32_t i = 0; env && i < scopeDepth; i++)
    env = env->getParentEnvironment(runtime_);

  // Now we can get the variable, or undefined if we have no environment.
  return env ? env->slot(variableIndex) : undefined;
}

HermesValue Debugger::getThisValue(uint32_t frame) const {
  const auto frameInfo = runtime_.stackFrameInfoByIndex(frame);
  assert(frameInfo && "Invalid frame");

  if (frameInfo->isGlobal) {
    // "this" value in the global frame is the global object.
    return runtime_.getGlobal().getHermesValue();
  }

  return frameInfo->frame.getThisArgRef();
}

HermesValue Debugger::getExceptionAsEvalResult(
    EvalResultMetadata *outMetadata) {
  outMetadata->isException = true;

  Handle<> thrownValue = runtime_.makeHandle(runtime_.getThrownValue());
  assert(!thrownValue->isEmpty() && "Runtime did not throw");
  runtime_.clearThrownValue();

  // Set the exceptionDetails.text to toString_RJS() of the thrown value.
  // TODO: rationalize what should happen if toString_RJS() itself throws.
  auto res = toString_RJS(runtime_, thrownValue);
  if (res != ExecutionStatus::EXCEPTION) {
    llvh::SmallVector<char16_t, 64> errorText;
    res->get()->appendUTF16String(errorText);
    convertUTF16ToUTF8WithReplacements(
        outMetadata->exceptionDetails.text, errorText);
  }

  // Try to fetch the stack trace. It may not exist; for example, if the
  // exception was a parse error in eval(), then the exception will be set
  // directly and the stack trace will not be collected.
  if (auto errorHandle = Handle<JSError>::dyn_vmcast(thrownValue)) {
    if (auto stackTracePtr = errorHandle->getStackTrace()) {
      // Copy the stack trace to ensure it's not moved out from under us.
      const auto stackTraceCopy = *stackTracePtr;
      std::vector<CallFrameInfo> frames;
      frames.reserve(stackTraceCopy.size());
      for (const StackTraceInfo &sti : stackTraceCopy)
        frames.push_back(getCallFrameInfo(sti.codeBlock, sti.bytecodeOffset));
      outMetadata->exceptionDetails.stackTrace_ = StackTrace{std::move(frames)};
    }
  }
  return *thrownValue;
}

HermesValue Debugger::evalInFrame(
    const EvalArgs &args,
    const std::string &src,
    const InterpreterState &state,
    EvalResultMetadata *outMetadata) {
  GCScope gcScope{runtime_};
  *outMetadata = EvalResultMetadata{};
  uint32_t frame = args.frameIdx;
  auto frameInfo = runtime_.stackFrameInfoByIndex(frame);
  if (!frameInfo) {
    return HermesValue::encodeUndefinedValue();
  }

  MutableHandle<> resultHandle(runtime_);
  bool singleFunction = false;

  // Environment may be undefined if it has not been created yet.
  Handle<Environment> env = frameInfo->frame->getDebugEnvironmentHandle();
  if (!env) {
    // TODO: this comes about when we break in a function before its environment
    // has been created. What we would like to do here is synthesize an
    // environment with undefined for all locals, since no variables can have
    // been defined yet, and link it to the parent scope. For now we just bail
    // out.
    return HermesValue::encodeUndefinedValue();
  }

  const CodeBlock *cb = frameInfo->frame->getCalleeCodeBlock(runtime_);
  auto scopeChain = scopeChainForBlock(runtime_, cb);
  if (!scopeChain) {
    // Binary was compiled without variable debug info.
    return HermesValue::encodeUndefinedValue();
  }

  // Interpreting code requires that the `thrownValue_` is empty.
  // Save it temporarily so we can restore it after the evalInEnvironment.
  Handle<> savedThrownValue = runtime_.makeHandle(runtime_.getThrownValue());
  runtime_.clearThrownValue();

  CallResult<HermesValue> result = evalInEnvironment(
      runtime_,
      src,
      env,
      *scopeChain,
      Handle<>(&frameInfo->frame->getThisArgRef()),
      singleFunction);

  // Check if an exception was thrown.
  if (result.getStatus() == ExecutionStatus::EXCEPTION) {
    resultHandle = getExceptionAsEvalResult(outMetadata);
  } else {
    assert(
        !result->isEmpty() &&
        "eval result should not be empty unless exception was thrown");
    resultHandle = *result;
  }

  runtime_.setThrownValue(savedThrownValue.getHermesValue());
  return *resultHandle;
}

llvh::Optional<std::pair<InterpreterState, uint32_t>> Debugger::findCatchTarget(
    const InterpreterState &state) const {
  auto *codeBlock = state.codeBlock;
  auto offset = state.offset;
  auto frames = runtime_.getStackFrames();
  for (auto it = frames.begin(), e = frames.end(); it != e; ++it) {
    if (codeBlock) {
      auto handlerOffset = codeBlock->findCatchTargetOffset(offset);
      if (handlerOffset != -1) {
        return std::make_pair(
            InterpreterState(codeBlock, handlerOffset),
            runtime_.calcFrameOffset(it));
      }
    }
    codeBlock = it->getSavedCodeBlock();
    if (codeBlock) {
      offset = codeBlock->getOffsetOf(it->getSavedIP());
    }
  }
  return llvh::None;
}

bool Debugger::resolveBreakpointLocation(Breakpoint &breakpoint) const {
  using fhd::kInvalidLocation;
  assert(!breakpoint.isResolved() && "breakpoint already resolved");

  OptValue<hbc::DebugSearchResult> locationOpt{};

#ifndef HERMESVM_LEAN
  // If we could have lazy code blocks, compile them before we try to resolve.
  // Eagerly compile code blocks that may contain the location.
  // This is done using a search in which we enumerate all CodeBlocks in the
  // runtime module, and we visit any code blocks which are lazy and check
  // their ASTs to see if the breakpoint location is in them.
  // Note that this works because we have the start and end locations
  // exactly when a CodeBlock is lazy, because that's only when the AST exists.
  // If it is, we compile the CodeBlock and start over,
  // skipping any CodeBlocks we've seen before.
  GCScope gcScope{runtime_};
  for (auto &runtimeModule : runtime_.getRuntimeModules()) {
    llvh::DenseSet<CodeBlock *> visited{};
    std::vector<CodeBlock *> toVisit{};
    for (uint32_t i = 0, e = runtimeModule.getNumCodeBlocks(); i < e; ++i) {
      GCScopeMarkerRAII marker{gcScope};
      // Use getCodeBlock to ensure they get initialized (but not compiled).
      toVisit.push_back(runtimeModule.getCodeBlockMayAllocate(i));
    }

    while (!toVisit.empty()) {
      GCScopeMarkerRAII marker{gcScope};
      CodeBlock *codeBlock = toVisit.back();
      toVisit.pop_back();

      if (!codeBlock || !codeBlock->isLazy()) {
        // When looking for a lazy code block to expand,
        // there's no point looking at the non-lazy ones.
        continue;
      }

      if (visited.count(codeBlock) > 0) {
        // We've already been here.
        continue;
      }

      visited.insert(codeBlock);
      auto start = codeBlock->getLazyFunctionStartLoc();
      auto end = codeBlock->getLazyFunctionEndLoc();

      const auto &request = breakpoint.requestedLocation;
      if ((start.line < request.line && request.line < end.line) ||
          ((start.line == request.line || request.line == end.line) &&
           (start.col <= request.column && request.column <= end.col))) {
        // The code block probably contains the breakpoint we want to set.
        // First, we compile it.
        if (LLVM_UNLIKELY(
                codeBlock->lazyCompile(runtime_) ==
                ExecutionStatus::EXCEPTION)) {
          // TODO: how to better handle this?
          runtime_.clearThrownValue();
        }

        // We've found the codeBlock at this level and expanded it,
        // so there's no point continuing the search.
        // Abandon the current toVisit queue and repopulate it.
        toVisit.clear();

        // Compiling the function will add more functions to the runtimeModule.
        // Re-add them all so we can continue the search.
        for (uint32_t i = 0, e = runtimeModule.getNumCodeBlocks(); i < e; ++i) {
          GCScopeMarkerRAII marker2{gcScope};
          // Use getCodeBlock to ensure they get initialized (but not compiled).
          toVisit.push_back(runtimeModule.getCodeBlockMayAllocate(i));
        }
      }
    }
  }
#endif

  // Iterate backwards through runtime modules, under the assumption that
  // modules at the end of the list were added more recently, and are more
  // likely to match the user's intention.
  // Specifically, this will check any user source before runtime modules loaded
  // by the VM.
  for (auto it = runtime_.getRuntimeModules().rbegin();
       it != runtime_.getRuntimeModules().rend();
       ++it) {
    auto &runtimeModule = *it;
    GCScope gcScope{runtime_};

    if (!runtimeModule.isInitialized()) {
      // Uninitialized module.
      continue;
    }
    if (!runtimeModule.getBytecode()->getDebugInfo()) {
      // No debug info in this module, keep going.
      continue;
    }

    const auto *debugInfo = runtimeModule.getBytecode()->getDebugInfo();
    const auto &fileRegions = debugInfo->viewFiles();
    if (fileRegions.empty()) {
      continue;
    }

    uint32_t resolvedFileId = kInvalidLocation;
    std::string resolvedFileName{};

    if (!breakpoint.requestedLocation.fileName.empty()) {
      for (const auto &region : fileRegions) {
        std::string storage =
            getFileNameAsUTF8(runtime_, &runtimeModule, region.filenameId);
        llvh::StringRef storageRef{storage};
        if (storageRef.consume_back(breakpoint.requestedLocation.fileName)) {
          resolvedFileId = region.filenameId;
          resolvedFileName = std::move(storage);
          break;
        }
      }
    } else if (breakpoint.requestedLocation.fileId != kInvalidLocation) {
      for (const auto &region : fileRegions) {
        // We don't yet have a convincing story for debugging CommonJS, so for
        // now just assert that we're still living in the one-file-per-RM world.
        // TODO(T84976604): Properly handle setting breakpoints when there are
        // multiple JS files per HBC file.
        assert(
            region.filenameId == 0 && "Unexpected multiple filenames per RM");
        if (resolveScriptId(&runtimeModule, region.filenameId) ==
            breakpoint.requestedLocation.fileId) {
          resolvedFileId = region.filenameId;
          resolvedFileName =
              getFileNameAsUTF8(runtime_, &runtimeModule, resolvedFileId);
          break;
        }
      }
    } else {
      // No requested file, just pick the first one.
      resolvedFileId = fileRegions.front().filenameId;
      resolvedFileName =
          getFileNameAsUTF8(runtime_, &runtimeModule, resolvedFileId);
    }

    if (resolvedFileId == kInvalidLocation) {
      // Unable to find the file here.
      continue;
    }

    locationOpt = debugInfo->getAddressForLocation(
        resolvedFileId,
        breakpoint.requestedLocation.line,
        breakpoint.requestedLocation.column == kInvalidLocation
            ? llvh::None
            : OptValue<uint32_t>{breakpoint.requestedLocation.column});

    if (locationOpt.hasValue()) {
      breakpoint.codeBlock =
          runtimeModule.getCodeBlockMayAllocate(locationOpt->functionIndex);
      breakpoint.offset = locationOpt->bytecodeOffset;

      SourceLocation resolvedLocation;
      resolvedLocation.line = locationOpt->line;
      resolvedLocation.column = locationOpt->column;
      resolvedLocation.fileId = resolveScriptId(&runtimeModule, resolvedFileId);
      resolvedLocation.fileName = std::move(resolvedFileName);
      breakpoint.resolvedLocation = resolvedLocation;
      return true;
    }
  }

  return false;
}

void Debugger::unresolveBreakpointLocation(Breakpoint &breakpoint) {
  assert(breakpoint.isResolved() && "Breakpoint already unresolved");
  if (breakpoint.enabled) {
    unsetUserBreakpoint(breakpoint);
  }
  breakpoint.resolvedLocation.reset();
  breakpoint.codeBlock = nullptr;
  breakpoint.offset = -1;
}

auto Debugger::getSourceMappingUrl(ScriptID scriptId) const -> String {
  for (auto &runtimeModule : runtime_.getRuntimeModules()) {
    if (!runtimeModule.isInitialized()) {
      // Uninitialized module.
      continue;
    }

    auto *debugInfo = runtimeModule.getBytecode()->getDebugInfo();
    if (!debugInfo) {
      // No debug info in this module, keep going.
      continue;
    }

    for (const auto &file : debugInfo->viewFiles()) {
      if (resolveScriptId(&runtimeModule, file.filenameId) == scriptId) {
        if (file.sourceMappingUrlId == fhd::kInvalidBreakpoint) {
          return "";
        }
        return getFileNameAsUTF8(
            runtime_, &runtimeModule, file.sourceMappingUrlId);
      }
    }
  }

  return "";
}

auto Debugger::resolveScriptId(
    RuntimeModule *runtimeModule,
    uint32_t filenameId) const -> ScriptID {
  return runtimeModule->getScriptID();
}

} // namespace vm
} // namespace hermes

#endif

#endif // HERMES_ENABLE_DEBUGGER
