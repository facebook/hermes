/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef JSRUNTIME_H
#define JSRUNTIME_H

#include <fbjni/fbjni.h>
#include <jsi/jsi.h>

namespace facebook {
namespace jsi {
namespace jni {

namespace jni = ::facebook::jni;

class JSRuntime : public jni::HybridClass<JSRuntime> {
 public:
  constexpr static auto kJavaDescriptor =
      "Lcom/facebook/hermes/test/JSRuntime;";

  static jni::local_ref<jhybridobject> makeHermesRuntime(
      jni::alias_ref<jclass>,
      bool shouldRecordGCStats);
  static jni::local_ref<jhybridobject> makeHermesRuntimeWithHeapSpec(
      jni::alias_ref<jclass>,
      jlong initHeapSize,
      jlong maxHeapSize,
      bool shouldRecordGCStats);

  static jni::local_ref<jbyteArray> compileJavaScript(
      jni::alias_ref<jclass>,
      jni::alias_ref<jbyteArray> bytes);

  static void registerNatives();

  void evaluateJavaScript(jni::alias_ref<jbyteArray> bytes);
  void evaluateJavaScriptFile(std::string fileName);

  jint getGlobalNumberProperty(std::string propName);
  std::string getGlobalStringProperty(std::string propName);
  void setGlobalNumberProperty(std::string propName, jint val);
  void setGlobalStringProperty(std::string propName, std::string val);
  void callFunction(std::string functionName);

  std::string getRecordedGCStats();

 private:
  friend HybridBase;
  JSRuntime(std::unique_ptr<jsi::Runtime> rt);

  std::unique_ptr<jsi::Runtime> runtime_;
};

} // namespace jni
} // namespace jsi
} // namespace facebook

#endif // JSRUNTIME_H
