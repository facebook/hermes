/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <jni.h>

extern "C" JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
  return JNI_VERSION_1_6;
}
