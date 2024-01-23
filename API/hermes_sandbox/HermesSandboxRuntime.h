/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SANDBOX_HERMES_SANDBOX_RUNTIME_H
#define HERMES_SANDBOX_HERMES_SANDBOX_RUNTIME_H

#include <hermes/Public/HermesExport.h>
#include <jsi/jsi.h>

namespace facebook {
namespace hermes {

/// A JSI Runtime that is implemented on top of a sandboxed build of Hermes.
/// This imposes safety checks that prevent bugs in Hermes from being exploited,
/// although they may still result in an abort.
class HERMES_EXPORT HermesSandboxRuntime : public jsi::Runtime {
 public:
  /// Check if the given buffer contains Hermes bytecode.
  [[nodiscard]] static bool isHermesBytecode(const uint8_t *data, size_t len);

  /// Evaluate the given bytecode buffer and return the result.
  virtual jsi::Value evaluateHermesBytecode(
      const std::shared_ptr<const jsi::Buffer> &buffer,
      const std::string &sourceURL) = 0;

  /// Asynchronously terminates the current execution. This can be called on
  /// any thread.
  virtual void asyncTriggerTimeout() = 0;
};

/// Create a sandboxed Hermes runtime.
HERMES_EXPORT std::unique_ptr<HermesSandboxRuntime> makeHermesSandboxRuntime();

} // namespace hermes
} // namespace facebook

#endif // HERMES_SANDBOX_HERMES_SANDBOX_RUNTIME_H
