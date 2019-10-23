/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <jni.h>

#include <fbjni/fbjni.h>

using namespace facebook::jni;

void RegisterFbjniTests();
void RegisterTestHybridClass();
void RegisterPrimitiveArrayTests();
void RegisterIteratorTests();
void RegisterByteBufferTests();
void RegisterReadableByteChannelTests();

jint JNI_OnLoad(JavaVM* vm, void*) {
  return facebook::jni::initialize(vm, [] {
      RegisterFbjniTests();
      RegisterTestHybridClass();
      RegisterPrimitiveArrayTests();
      RegisterIteratorTests();
      RegisterByteBufferTests();
      RegisterReadableByteChannelTests();
  });
}
