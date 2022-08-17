/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "JSRuntime.h"

#include <jsi/instrumentation.h>

#include "MappedFileBuffer.h"

namespace facebook {
namespace jsi {
namespace jni {

void JSRuntime::registerNatives() {
  javaClassLocal()->registerNatives({
      makeNativeMethod("makeHermesRuntime", JSRuntime::makeHermesRuntime),
      makeNativeMethod(
          "makeHermesRuntimeWithHeapSpec",
          JSRuntime::makeHermesRuntimeWithHeapSpec),
      makeNativeMethod("compileJavaScript", JSRuntime::compileJavaScript),
      makeNativeMethod("evaluateJavaScript", JSRuntime::evaluateJavaScript),
      makeNativeMethod(
          "evaluateJavaScriptFile", JSRuntime::evaluateJavaScriptFile),
      makeNativeMethod(
          "getGlobalNumberProperty", JSRuntime::getGlobalNumberProperty),
      makeNativeMethod(
          "getGlobalStringProperty", JSRuntime::getGlobalStringProperty),
      makeNativeMethod(
          "setGlobalNumberProperty", JSRuntime::setGlobalNumberProperty),
      makeNativeMethod(
          "setGlobalStringProperty", JSRuntime::setGlobalStringProperty),
      makeNativeMethod("callFunction", JSRuntime::callFunction),
      makeNativeMethod("getRecordedGCStats", JSRuntime::getRecordedGCStats),
  });
}

void JSRuntime::evaluateJavaScript(jni::alias_ref<jbyteArray> bytes) {
  auto bytesPrimArr = bytes->pin();
  std::string str(
      reinterpret_cast<char *>(bytesPrimArr.get()), bytesPrimArr.size());
  bytesPrimArr.release();
  runtime_->evaluateJavaScript(std::make_unique<jsi::StringBuffer>(str), "");
}

void JSRuntime::evaluateJavaScriptFile(std::string fileName) {
  runtime_->evaluateJavaScript(
      std::make_unique<MappedFileBuffer>(fileName), fileName);
}

jint JSRuntime::getGlobalNumberProperty(std::string propName) {
  return runtime_->global().getProperty(*runtime_, propName.c_str()).asNumber();
}

std::string JSRuntime::getGlobalStringProperty(std::string propName) {
  return runtime_->global()
      .getProperty(*runtime_, propName.c_str())
      .asString(*runtime_)
      .utf8(*runtime_);
}

void JSRuntime::setGlobalNumberProperty(std::string propName, jint val) {
  return runtime_->global().setProperty(*runtime_, propName.c_str(), val);
}

void JSRuntime::setGlobalStringProperty(std::string propName, std::string val) {
  return runtime_->global().setProperty(
      *runtime_, propName.c_str(), val.c_str());
}

void JSRuntime::callFunction(std::string functionName) {
  jsi::Function function =
      runtime_->global().getPropertyAsFunction(*runtime_, functionName.c_str());

  function.call(*runtime_);
}

std::string JSRuntime::getRecordedGCStats() {
  return runtime_->instrumentation().getRecordedGCStats();
}

JSRuntime::JSRuntime(std::unique_ptr<jsi::Runtime> runtime)
    : runtime_{std::move(runtime)} {}

} // namespace jni
} // namespace jsi
} // namespace facebook
