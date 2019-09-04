// Copyright 2004-present Facebook. All Rights Reserved.

#include <vector>

#include <fbjni/fbjni.h>
#include <fbjni/ByteBuffer.h>

#include "expect.h"

using namespace facebook::jni;

namespace {

size_t ByteBufferCapacity(alias_ref<JByteBuffer> buffer) {
  static auto meth = JByteBuffer::javaClassStatic()->getMethod<int()>("capacity");
  return meth(buffer);
}

jboolean testDirectByteBuffer(JNIEnv*, jclass) {
  std::vector<uint8_t> vec{5, 4, 3, 2, 1, 0};
  auto nbb = JByteBuffer::wrapBytes(vec.data(), vec.size());

  EXPECT(ByteBufferCapacity(nbb) == vec.size());
  EXPECT(nbb->isDirect());
  EXPECT(nbb->getDirectSize() == vec.size());

  auto bytes = nbb->getDirectBytes();

  for (size_t i = 0; i < vec.size(); i++) {
    EXPECT(bytes[i] == vec[i]);
  }
  return JNI_TRUE;
}

jboolean testEmptyDirectByteBuffer(JNIEnv*, jclass) {
  uint8_t data;
  auto nbb = JByteBuffer::wrapBytes(&data, 0);

  EXPECT(ByteBufferCapacity(nbb) == 0);
  EXPECT(nbb->isDirect());
  EXPECT(nbb->getDirectSize() == 0);

  return JNI_TRUE;
}

jboolean testRewindBuffer(alias_ref<jobject> self) {
  std::vector<uint8_t> vec{0, 0, 0, 0, 0, 0, 0, 0};
  auto nbb = JByteBuffer::wrapBytes(vec.data(), vec.size());

  auto cls = self->getClass();
  auto writeBytes =
    cls->getStaticMethod<void(JByteBuffer, jbyte, jbyte, jbyte, jbyte)>("writeBytes");

  writeBytes(cls, *nbb, 0, 1, 2, 3);
  nbb->rewind();
  writeBytes(cls, *nbb, 4, 5, 6, 7);

  EXPECT(vec[0] == 4);
  EXPECT(vec[1] == 5);
  EXPECT(vec[2] == 6);
  EXPECT(vec[3] == 7);
  EXPECT(vec[4] == 0);
  EXPECT(vec[5] == 0);
  EXPECT(vec[6] == 0);
  EXPECT(vec[7] == 0);

  return JNI_TRUE;
}

local_ref<JByteBuffer> nativeAllocateDirect(alias_ref<jobject> self, int size) {
  return JByteBuffer::allocateDirect(size);
}

jboolean testFloatBuffer(alias_ref<jobject> self, alias_ref<facebook::jni::JBuffer> buffer) {
  EXPECT(buffer->isDirect());
  EXPECT(buffer->getDirectCapacity() == 5);
  float* raw = (float*)buffer->getDirectAddress();
  EXPECT(raw);

  EXPECT(raw[0] == 1);
  EXPECT(raw[1] == 2);
  EXPECT(raw[2] == 2.5);
  EXPECT(raw[3] == 2.75);
  EXPECT(raw[4] == 3);

  return JNI_TRUE;
}


}

void RegisterByteBufferTests() {
  registerNatives("com/facebook/jni/ByteBufferTests", {
    makeNativeMethod("nativeTestDirectByteBuffer", testDirectByteBuffer),
    makeNativeMethod("nativeTestEmptyDirectByteBuffer", testEmptyDirectByteBuffer),
    makeNativeMethod("nativeTestRewindBuffer", testRewindBuffer),
    makeNativeMethod("nativeAllocateDirect", nativeAllocateDirect),
    makeNativeMethod("nativeTestFloatBuffer", testFloatBuffer),
  });
}
