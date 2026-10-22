/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <hermes/hermes.h>
#include <hermes_sandbox/HermesSandboxRuntime.h>
#include <jsi/test/testlib.h>
#include <jsi/threadsafe.h>

using namespace facebook::hermes;

namespace facebook {
namespace jsi {

std::vector<RuntimeFactory> runtimeGenerators() {
  return {
      [] {
        return makeHermesRuntime(
            ::hermes::vm::RuntimeConfig::Builder()
                .withEnableHermesInternalTestMethods(true)
                .build());
      },
      [] {
        return makeThreadSafeHermesRuntime(
            ::hermes::vm::RuntimeConfig::Builder()
                .withEnableHermesInternalTestMethods(true)
                .build());
      }};
}

/// The factories in runtimeGenerators(), plus HermesSandboxRuntime.
std::vector<RuntimeFactory> runtimeGeneratorsWithSandBox() {
  auto factories = runtimeGenerators();
  factories.push_back([] { return makeHermesSandboxRuntime(); });
  return factories;
}

} // namespace jsi
} // namespace facebook
