/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <hermes/hermes.h>
#include <jsi/test/testlib.h>
#include <jsi/threadsafe.h>

using namespace facebook::hermes;

namespace facebook {
namespace jsi {

std::vector<RuntimeFactory> runtimeGenerators() {
  return {
      [] { return makeHermesRuntime(); },
      [] { return makeThreadSafeHermesRuntime(); },
  };
}

} // namespace jsi
} // namespace facebook
