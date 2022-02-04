/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMESEPILOGUE_H_
#define HERMESEPILOGUE_H_

#include <fbjni/fbjni.h>
#include <jsi/jsi.h>

namespace facebook {
namespace jsi {
namespace jni {

namespace jni = ::facebook::jni;

class HermesEpilogue : public jni::HybridClass<HermesEpilogue> {
 public:
  constexpr static auto kJavaDescriptor =
      "Lcom/facebook/hermes/test/HermesEpilogue;";
  static jni::local_ref<jbyteArray> getHermesBytecodeMetadata(
      jni::alias_ref<jclass>,
      jni::alias_ref<jbyteArray> bytes);
  static jni::local_ref<jbyteArray> getHermesBCFileMetadata(
      jni::alias_ref<jclass>,
      std::string filename);

  static void registerNatives();

 private:
  friend HybridBase;
  HermesEpilogue(std::unique_ptr<jsi::Runtime> rt);

  std::unique_ptr<jsi::Runtime> runtime_;
};

} // namespace jni
} // namespace jsi
} // namespace facebook

#endif /* HERMESEPILOGUE_H_ */
