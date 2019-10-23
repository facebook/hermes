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

#include <vector>
#include <fbjni/fbjni.h>
#include <fbjni/ByteBuffer.h>
#include <fbjni/ReadableByteChannel.h>

#include "expect.h"

using namespace facebook::jni;

jboolean testSmallRead(alias_ref<JClass> cls, alias_ref<JReadableByteChannel> channel, alias_ref<JArrayByte> data) {
  std::vector<uint8_t> vec(data->size() * 2);
  auto npp = JByteBuffer::wrapBytes(vec.data(), vec.size());

  unsigned n = channel->read(npp);
  EXPECT(n == data->size());

  auto pinned = data->pin();
  for (size_t i = 0; i < data->size(); i++) {
    EXPECT(vec[i] == pinned[i]);
  }

  return JNI_TRUE;
}

jboolean testReadToBufferCapacity(
  alias_ref<JClass> cls, alias_ref<JReadableByteChannel> channel, alias_ref<JArrayByte> data) {
  std::vector<uint8_t> vec(data->size() / 2);
  auto npp = JByteBuffer::wrapBytes(vec.data(), vec.size());

  unsigned n = channel->read(npp);
  EXPECT(n == vec.size());

  n = channel->read(npp);
  EXPECT(n == 0);

  auto pinned = data->pin();
  for (size_t i = 0; i < vec.size(); i++) {
    EXPECT(vec[i] == pinned[i]);
  }

  return JNI_TRUE;
}

jboolean testConsumeChannel(
  alias_ref<JClass> cls, alias_ref<JReadableByteChannel> channel, alias_ref<JArrayByte> data) {
  std::vector<uint8_t> vec(data->size() + 16);
  auto npp = JByteBuffer::wrapBytes(vec.data(), vec.size());

  int n = channel->read(npp);
  EXPECT((unsigned) n == data->size());

  n = channel->read(npp);
  EXPECT(n == -1);

  auto pinned = data->pin();
  for (size_t i = 0; i < data->size(); i++) {
    EXPECT(vec[i] == pinned[i]);
  }

  return JNI_TRUE;
}

jboolean testConsumeChannelIteratively(
  alias_ref<JClass> cls, alias_ref<JReadableByteChannel> channel, alias_ref<JArrayByte> data) {
  std::vector<uint8_t> vec(data->size() / 4);
  auto npp = JByteBuffer::wrapBytes(vec.data(), vec.size());
  auto pinned = data->pin();

  for (size_t i = 0; i < 4; i++) {
    unsigned n = channel->read(npp);
    EXPECT(n == data->size() / 4);
    npp->rewind();
  }

  int n = channel->read(npp);
  EXPECT(n == -1);

  return JNI_TRUE;
}

void RegisterReadableByteChannelTests() {
  registerNatives("com/facebook/jni/ReadableByteChannelTests", {
    makeNativeMethod("nativeTestSmallRead", testSmallRead),
    makeNativeMethod("nativeTestReadToBufferCapacity", testReadToBufferCapacity),
    makeNativeMethod("nativeTestConsumeChannel", testConsumeChannel),
    makeNativeMethod("nativeTestConsumeChannelIteratively", testConsumeChannelIteratively),
  });
}
