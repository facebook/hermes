/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <cassert>
#include <utility>

#include <hermes/cdp/EvaluatedScriptSources.h>

#include <jsi/jsi.h>

namespace facebook {
namespace hermes {
namespace cdp {

/// Most retained sources are a line or two of console input, but
/// Runtime.callFunctionOn compiles one per call and debug clients issue many.
static constexpr size_t kMaxRetainedBytes = 1 << 20;

EvaluatedScriptSources::PendingSource::PendingSource(
    EvaluatedScriptSources &sources,
    std::shared_ptr<const jsi::StringBuffer> source)
    : sources_(sources),
      previous_(std::exchange(sources.pending_, std::move(source))) {}

EvaluatedScriptSources::PendingSource::~PendingSource() {
  sources_.pending_ = std::move(previous_);
}

EvaluatedScriptSources::PendingSource EvaluatedScriptSources::push(
    std::shared_ptr<const jsi::StringBuffer> source) {
  return PendingSource(*this, std::move(source));
}

void EvaluatedScriptSources::popAndAssignSource(debugger::ScriptID scriptID) {
  if (!pending_) {
    return;
  }
  std::shared_ptr<const jsi::StringBuffer> source = std::move(pending_);
  pending_.reset();

  // A script compiled without debug info is never announced to the client, so
  // there is no source to serve. Drop it rather than leaving it parked for a
  // later script of the same evaluation to claim.
  if (scriptID == debugger::kInvalidLocation) {
    return;
  }

  // An evaluation records at most one script: recording consumes the parked
  // source, so a key is never recorded twice while retained.
  std::string key = std::to_string(scriptID);
  sourcesSize_ += key.size() + source->size();
  insertionOrder_.push_back(key);
  bool inserted = sources_.emplace(std::move(key), std::move(source)).second;
  assert(inserted && "duplicate script ID recorded");
  // `assert` compiles out in release builds, leaving `inserted` unread.
  (void)inserted;

  // Keep the entry just added even when it alone exceeds the budget: dropping
  // it would leave the client unable to read the script it just created.
  while (sources_.size() > 1 && sourcesSize_ > kMaxRetainedBytes) {
    auto it = sources_.find(insertionOrder_.front());
    assert(it != sources_.end() && "eviction ID missing from sources");
    sourcesSize_ -= it->first.size() + it->second->size();
    sources_.erase(it);
    insertionOrder_.pop_front();
  }
}

std::shared_ptr<const jsi::StringBuffer> EvaluatedScriptSources::find(
    const std::string &scriptId) const {
  auto it = sources_.find(scriptId);
  if (it == sources_.end()) {
    return nullptr;
  }
  return it->second;
}

} // namespace cdp
} // namespace hermes
} // namespace facebook
