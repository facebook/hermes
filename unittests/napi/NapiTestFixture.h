/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_UNITTESTS_NAPI_NAPITESTFIXTURE_H
#define HERMES_UNITTESTS_NAPI_NAPITESTFIXTURE_H

#include "hermes_napi_impl.h"

#include "hermes/Public/RuntimeConfig.h"
#include "hermes/VM/Runtime.h"

#include "gtest/gtest.h"

#include <memory>

namespace hermes {
namespace napi {

/// A GTest fixture for NAPI unit tests.
///
/// Creates a Hermes Runtime and a napi_env backed by it. The env is
/// created in SetUp and destroyed in TearDown so that the Runtime
/// outlives the env (the custom root callback captures the env pointer).
class NapiTestFixture : public ::testing::Test {
 protected:
  std::shared_ptr<vm::Runtime> rt_;
  napi_env env_ = nullptr;

  void SetUp() override {
    auto config = vm::RuntimeConfig::Builder()
                      .withGCConfig(
                          vm::GCConfig::Builder()
                              .withInitHeapSize(1 << 16)
                              .withMaxHeapSize(1 << 19)
                              .build())
                      .build();
    rt_ = vm::Runtime::create(config);
    env_ = hermes_napi_create_env(&*rt_);
  }

  /// Trigger a full GC and drain any pending deferred finalizers.
  /// Use this instead of rt_->collect() when testing finalizer behavior.
  void collectAndDrain(const char *name = "test") {
    rt_->collect(name);
    env_->drainPendingFinalizers();
  }

  void TearDown() override {
    // The env is owned by the Runtime and torn down as part of
    // ~Runtime. Drop the borrowed pointer and reset the runtime.
    env_ = nullptr;
    rt_.reset();
  }
};

} // namespace napi
} // namespace hermes

#endif // HERMES_UNITTESTS_NAPI_NAPITESTFIXTURE_H
