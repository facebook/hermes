/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <deque>
#include <memory>
#include <string>
#include <unordered_map>

#include <hermes/Public/DebuggerTypes.h>

namespace facebook {
namespace jsi {
class StringBuffer;
} // namespace jsi
namespace hermes {
namespace cdp {

/// Sources of the scripts created by evaluating debug client expressions (e.g.
/// Runtime.evaluate). Hermes discards those sources once compiled, so the CDP
/// layer retains them to answer Debugger.getScriptSource. Shared by all agents
/// of a runtime via CDPDebugAPI. Everything in this class must be used
/// exclusively from the runtime thread.
class EvaluatedScriptSources {
 public:
  /// Parks the source of the evaluation in flight. The script ID is only known
  /// once the script loads, part way through the evaluation, so the source is
  /// parked here until then. Destruction restores the previously parked
  /// source: Runtime.callFunctionOn reaches the runtime as a debugger
  /// interrupt, so it can begin part way through another evaluation.
  class PendingSource {
   public:
    ~PendingSource();

    PendingSource(const PendingSource &) = delete;
    PendingSource &operator=(const PendingSource &) = delete;
    PendingSource(PendingSource &&) = delete;
    PendingSource &operator=(PendingSource &&) = delete;

   private:
    friend class EvaluatedScriptSources;
    PendingSource(
        EvaluatedScriptSources &sources,
        std::shared_ptr<const jsi::StringBuffer> source);

    EvaluatedScriptSources &sources_;

    /// Source of the evaluation this one interrupted, restored on destruction.
    std::shared_ptr<const jsi::StringBuffer> previous_;
  };

  /// Parks \p source as the source of the evaluation in flight and returns a
  /// guard restoring the previously parked source when it goes out of scope.
  /// The retained source shares ownership with the caller: one buffer serves
  /// both compilation and later retrieval.
  [[nodiscard]] PendingSource push(
      std::shared_ptr<const jsi::StringBuffer> source);

  /// Records the parked source, if any, as the source of \p scriptID and stops
  /// tracking it. An evaluation compiles a single script; anything it loads
  /// afterwards (an `eval()` call it makes, say) has a source of its own.
  void popAndAssignSource(debugger::ScriptID scriptID);

  /// Returns the source recorded for \p scriptId, or nullptr if there is none.
  /// \p scriptId comes straight from the debug client, so it may be any string.
  std::shared_ptr<const jsi::StringBuffer> find(
      const std::string &scriptId) const;

 private:
  /// Retained sources by script ID. A debug client can evaluate without limit
  /// and nothing tells us when a script stops being interesting, so retention
  /// is capped and the oldest entries are dropped to stay under it. V8 bounds
  /// its own script source cache the same way.
  std::unordered_map<std::string, std::shared_ptr<const jsi::StringBuffer>>
      sources_;

  /// Script IDs in insertion order, oldest first, for eviction.
  std::deque<std::string> insertionOrder_;

  /// Total size of \c sources_, counting script IDs and sources.
  size_t sourcesSize_ = 0;

  /// Source of the evaluation currently in flight, if any.
  std::shared_ptr<const jsi::StringBuffer> pending_;
};

} // namespace cdp
} // namespace hermes
} // namespace facebook
