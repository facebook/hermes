/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <fbjni/fbjni.h>
#include <gtest/gtest.h>
#include <hermes/hermes.h>
#include <jsi/instrumentation.h>
#include <jsi/jsi.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <utility>

namespace facebook::hermes::test {
namespace {

namespace jni = facebook::jni;

/// Signals that a host object below has been destroyed, and records what its
/// destructor observed.
struct ReleaseState {
  std::mutex mutex;
  std::condition_variable condition;
  bool released{false};
  /// Whether the thread that ran the destructor was attached to the JVM.
  bool attachedToJvm{false};
};

/// Host object owning a JNI global reference, so its destructor requires the
/// finalizer thread to be attached to the JVM.
class GlobalRefHostObject final : public jsi::HostObject {
 public:
  GlobalRefHostObject(
      jni::global_ref<jni::JString> globalRef,
      std::shared_ptr<ReleaseState> releaseState)
      : globalRef_(std::move(globalRef)),
        releaseState_(std::move(releaseState)) {}

  ~GlobalRefHostObject() override {
    globalRef_.reset();
    {
      std::lock_guard<std::mutex> lock(releaseState_->mutex);
      releaseState_->released = true;
    }
    releaseState_->condition.notify_one();
  }

 private:
  jni::global_ref<jni::JString> globalRef_;
  std::shared_ptr<ReleaseState> releaseState_;
};

/// Host object that records whether the thread running its destructor is
/// attached to the JVM. It owns no JNI references, so unlike
/// GlobalRefHostObject it is safe to finalize on a detached thread.
class AttachmentProbeHostObject final : public jsi::HostObject {
 public:
  explicit AttachmentProbeHostObject(std::shared_ptr<ReleaseState> releaseState)
      : releaseState_(std::move(releaseState)) {}

  ~AttachmentProbeHostObject() override {
    // currentOrNull() is the only fbjni entry point that reports attachment
    // without attaching the thread as a side effect.
    const bool attached = jni::detail::currentOrNull() != nullptr;
    {
      std::lock_guard<std::mutex> lock(releaseState_->mutex);
      releaseState_->attachedToJvm = attached;
      releaseState_->released = true;
    }
    releaseState_->condition.notify_one();
  }

 private:
  std::shared_ptr<ReleaseState> releaseState_;
};

/// Drop the only reference to \p hostObject and collect. Returns whether its
/// destructor signalled \p releaseState before the timeout.
bool finalizeHostObject(
    jsi::Runtime &runtime,
    std::shared_ptr<jsi::HostObject> hostObject,
    ReleaseState &releaseState) {
  (void)jsi::Object::createFromHostObject(runtime, std::move(hostObject));

  runtime.instrumentation().collectGarbage("test");

  std::unique_lock<std::mutex> lock(releaseState.mutex);
  return releaseState.condition.wait_for(
      lock, std::chrono::seconds(5), [&releaseState] {
        return releaseState.released;
      });
}

/// Finalize a host object owning a global reference. If the finalizer thread
/// is not attached to the JVM, releasing the reference throws and aborts the
/// process instead of returning.
bool finalizeGlobalRefHostObject(jsi::Runtime &runtime) {
  auto releaseState = std::make_shared<ReleaseState>();
  std::shared_ptr<jsi::HostObject> hostObject;
  {
    auto javaObject = jni::make_jstring("host object");
    hostObject = std::make_shared<GlobalRefHostObject>(
        jni::make_global(javaObject), releaseState);
  }

  return finalizeHostObject(runtime, std::move(hostObject), *releaseState);
}

TEST(HostObjectJniTest, HostObjectCanOwnJniGlobalRef) {
  // Everything below needs a JavaVM, so fail here rather than aborting deep
  // inside fbjni.
  ASSERT_TRUE(jni::Environment::isGlobalJvmAvailable());

  // No runner is configured, so hermes.cpp installs the default JNI
  // ThreadScope wrapper.
  auto runtime = facebook::hermes::makeHermesRuntime();
  EXPECT_TRUE(finalizeGlobalRefHostObject(*runtime));
}

TEST(HostObjectJniTest, ConfiguredFinalizerThreadRunnerOverridesDefault) {
  ASSERT_TRUE(jni::Environment::isGlobalJvmAvailable());

  std::atomic<bool> ranUnderConfiguredRunner{false};
  auto runtime = facebook::hermes::makeHermesRuntime(
      ::hermes::vm::RuntimeConfig::Builder()
          .withFinalizerThreadRunner(
              [&ranUnderConfiguredRunner](std::function<void()> run) {
                ranUnderConfiguredRunner = true;
                const jni::ThreadScope scope;
                run();
              })
          .build());

  EXPECT_TRUE(finalizeGlobalRefHostObject(*runtime));
  EXPECT_TRUE(ranUnderConfiguredRunner);
}

TEST(HostObjectJniTest, DefaultFinalizerThreadRunnerAttachesToJvm) {
  ASSERT_TRUE(jni::Environment::isGlobalJvmAvailable());

  auto runtime = facebook::hermes::makeHermesRuntime();

  auto releaseState = std::make_shared<ReleaseState>();
  ASSERT_TRUE(finalizeHostObject(
      *runtime,
      std::make_shared<AttachmentProbeHostObject>(releaseState),
      *releaseState));
  EXPECT_TRUE(releaseState->attachedToJvm);
}

TEST(HostObjectJniTest, EmptyFinalizerThreadRunnerSuppressesTheDefault) {
  ASSERT_TRUE(jni::Environment::isGlobalJvmAvailable());

  // An explicitly empty runner means "leave the thread unwrapped", unlike an
  // unset one, which selects the ThreadScope default exercised above.
  auto runtime = facebook::hermes::makeHermesRuntime(
      ::hermes::vm::RuntimeConfig::Builder()
          .withFinalizerThreadRunner(::hermes::vm::ThreadRunner{})
          .build());

  auto releaseState = std::make_shared<ReleaseState>();
  ASSERT_TRUE(finalizeHostObject(
      *runtime,
      std::make_shared<AttachmentProbeHostObject>(releaseState),
      *releaseState));
  EXPECT_FALSE(releaseState->attachedToJvm);
}

} // namespace
} // namespace facebook::hermes::test
