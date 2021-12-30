/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifdef HERMES_ENABLE_DEBUGGER

#include "DebuggerAPI.h"

#include "hermes.h"
#include "hermes/VM/Debugger/DebugCommand.h"
#include "hermes/VM/Debugger/Debugger.h"
#include "hermes/VM/HermesValue.h"

#include <jsi/jsi.h>

#include <memory>

using namespace ::facebook::hermes::debugger;
using ::hermes::vm::DebugCommand;
using ::hermes::vm::HermesValue;
using ::hermes::vm::InterpreterState;
EventObserver::~EventObserver() {}

::hermes::vm::Debugger *ProgramState::impl() const {
  return dbg_->impl_;
}

LexicalInfo ProgramState::getLexicalInfo(uint32_t frameIndex) const {
  return impl()->getLexicalInfoInFrame(frameIndex);
}

VariableInfo ProgramState::getVariableInfo(
    uint32_t frameIndex,
    uint32_t scopeDepth,
    uint32_t variableIndexInScope) const {
  VariableInfo info{};
  HermesValue hermesValue = impl()->getVariableInFrame(
      frameIndex, scopeDepth, variableIndexInScope, &info.name);
  info.value = dbg_->jsiValueFromHermesValue(hermesValue);
  return info;
}

VariableInfo ProgramState::getVariableInfoForThis(uint32_t frameIndex) const {
  VariableInfo info{};
  info.name = "this";
  HermesValue hermesValue = impl()->getThisValue(frameIndex);
  info.value = dbg_->jsiValueFromHermesValue(hermesValue);
  return info;
}

EvalResult ProgramState::getEvalResult() const {
  assert(
      pauseReason_ == PauseReason::EvalComplete &&
      "No EvalResult because pause is not due to EvalComplete");
  ::facebook::jsi::Value copiedValue(*dbg_->runtime_, evalResult_.value);
  return debugger::EvalResult{
      std::move(copiedValue),
      evalResult_.isException,
      evalResult_.exceptionDetails};
}

Command::~Command() = default;
Command::Command(Command &&) = default;
Command &Command::operator=(Command &&) = default;

Debugger::Debugger(
    ::facebook::hermes::HermesRuntime *runtime,
    ::hermes::vm::Debugger *impl)
    : runtime_(runtime), impl_(impl), state_(this) {
  using EvalResultMetadata = ::hermes::vm::Debugger::EvalResultMetadata;
  impl->setDidPauseCallback(
      [this](
          InterpreterState state,
          PauseReason reason,
          HermesValue evalResult,
          const EvalResultMetadata &evalResultMd,
          BreakpointID breakpoint) -> DebugCommand {
        if (!eventObserver_)
          return DebugCommand::makeContinue();
        state_.pauseReason_ = reason;
        state_.stackTrace_ = impl_->getStackTrace(state);
        state_.evalResult_.value = jsiValueFromHermesValue(evalResult);
        state_.evalResult_.isException = evalResultMd.isException;
        state_.evalResult_.exceptionDetails = evalResultMd.exceptionDetails;
        state_.breakpoint_ = breakpoint;
        Command command = eventObserver_->didPause(*this);
        return std::move(*command.debugCommand_);
      });
  impl->setBreakpointResolvedCallback([this](BreakpointID breakpoint) -> void {
    if (!eventObserver_) {
      return;
    }
    eventObserver_->breakpointResolved(*this, breakpoint);
  });
}

String Debugger::getSourceMappingUrl(uint32_t fileId) const {
  return impl_->getSourceMappingUrl(fileId);
}

uint64_t Debugger::setBreakpoint(SourceLocation loc) {
  return impl_->createBreakpoint(loc);
}

void Debugger::setBreakpointCondition(
    BreakpointID breakpoint,
    const String &condition) {
  return impl_->setBreakpointCondition(breakpoint, condition);
}

void Debugger::deleteBreakpoint(BreakpointID id) {
  impl_->deleteBreakpoint(id);
}

void Debugger::deleteAllBreakpoints() {
  impl_->deleteAllBreakpoints();
}

void Debugger::setBreakpointEnabled(BreakpointID id, bool enable) {
  impl_->setBreakpointEnabled(id, enable);
}

BreakpointInfo Debugger::getBreakpointInfo(BreakpointID breakpoint) {
  return impl_->getBreakpointInfo(breakpoint);
}

std::vector<BreakpointID> Debugger::getBreakpoints() {
  return impl_->getBreakpoints();
}

void Debugger::setShouldPauseOnScriptLoad(bool flag) {
  return impl_->setShouldPauseOnScriptLoad(flag);
}

bool Debugger::getShouldPauseOnScriptLoad() const {
  return impl_->getShouldPauseOnScriptLoad();
}

void Debugger::setPauseOnThrowMode(PauseOnThrowMode mode) {
  impl_->setPauseOnThrowMode(mode);
}

PauseOnThrowMode Debugger::getPauseOnThrowMode() const {
  return impl_->getPauseOnThrowMode();
}

void Debugger::triggerAsyncPause(AsyncPauseKind kind) {
  return impl_->triggerAsyncPause(kind);
}

void Debugger::setIsDebuggerAttached(bool isAttached) {
  return impl_->setIsDebuggerAttached(isAttached);
}

void Debugger::setEventObserver(EventObserver *observer) {
  eventObserver_ = observer;
}

Command::Command(::hermes::vm::DebugCommand &&debugCommand)
    : debugCommand_(new DebugCommand(std::move(debugCommand))) {}

Command Command::step(StepMode mode) {
  return Command(DebugCommand::makeStep(mode));
}

Command Command::continueExecution() {
  return Command(DebugCommand::makeContinue());
}

Command Command::eval(const String &src, uint32_t frameIndex) {
  return Command(DebugCommand::makeEval(frameIndex, src));
}

#endif // HERMES_ENABLE_DEBUGGER
