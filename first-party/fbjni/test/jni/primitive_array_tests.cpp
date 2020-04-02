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

#include <cmath>
#include <vector>

#include <fbjni/fbjni.h>

#include "expect.h"

using namespace facebook::jni;

local_ref<jbooleanArray> testMakeBoolArray(alias_ref<jclass>, jint size) {
  return make_boolean_array(size);
}

local_ref<jbyteArray> testMakeByteArray(alias_ref<jclass>, jint size) {
  return make_byte_array(size);
}

local_ref<jcharArray> testMakeCharArray(alias_ref<jclass>, jint size) {
  return make_char_array(size);
}

local_ref<jshortArray> testMakeShortArray(alias_ref<jclass>, jint size) {
  return make_short_array(size);
}

local_ref<jintArray> testMakeIntArray(alias_ref<jclass>, jint size) {
  return make_int_array(size);
}

local_ref<jlongArray> testMakeLongArray(alias_ref<jclass>, jint size) {
  return make_long_array(size);
}

local_ref<jfloatArray> testMakeFloatArray(alias_ref<jclass>, jint size) {
  return make_float_array(size);
}

local_ref<jdoubleArray> testMakeDoubleArray(alias_ref<jclass>, jint size) {
  return make_double_array(size);
}

jboolean testGetSetBooleanArray(alias_ref<jclass>, alias_ref<jbooleanArray> array) {
  EXPECT(array);

  auto n = array->size();
  EXPECT(n == 2);

  auto vec = std::vector<jboolean>(n);
  array->getRegion(0, n, vec.data());
  auto smartbuf = array->getRegion(0, n);

  EXPECT(!vec[0] && vec[1]);
  EXPECT(!smartbuf[0] && smartbuf[1]);

  for (auto i = 0u; i < n; ++i) {
    smartbuf[i] = !smartbuf[i];
  }

  array->setRegion(0, n, smartbuf.get());

  return JNI_TRUE;
}

jboolean testPinBooleanArray(alias_ref<jclass>, alias_ref<jbooleanArray> array) {
  EXPECT(array);

  auto n = static_cast<jboolean>(array->size());
  auto pinned = array->pin();

  EXPECT(!pinned[0] && pinned[1]);

  for (auto i = 0; i < n; ++i) {
    pinned[i] = !pinned[i];
  }

  return JNI_TRUE;
}

constexpr double kEps = 1e-3;

template <typename JArrayType>
jboolean testGetSetArray(alias_ref<jclass>, alias_ref<JArrayType> array) {
  EXPECT(array);
  int n = array->size();
  auto vec = std::vector<typename jtype_traits<JArrayType>::entry_type>(n);
  array->getRegion(0, n, vec.data());
  auto smartbuf = array->getRegion(0, n);

  for (auto i = 0; i < n; ++i) {
    EXPECT(std::abs(static_cast<double>(vec[i] - i)) < kEps);
    EXPECT(std::abs(static_cast<double>(smartbuf[i] - i)) < kEps);
  }

  for (auto i = 0; i < n; ++i) {
    smartbuf[i] *= 2;
  }

  array->setRegion(0, n, smartbuf.get());

  return JNI_TRUE;
}

template <typename JArrayType>
jboolean testPinArray(alias_ref<jclass>, alias_ref<JArrayType> array) {
  EXPECT(array);

  int n = array->size();
  auto pinned = array->pin();

  for (auto i = 0; i < n; ++i) {
    EXPECT(std::abs(static_cast<double>(pinned[i] - i)) < kEps);
  }

  for (auto i = 0; i < n; ++i) {
    pinned[i] *= 2;
  }

  return JNI_TRUE;
}

template <typename JArrayType>
jboolean testPinArrayRegion(alias_ref<jclass>, alias_ref<JArrayType> array) {
  EXPECT(array);
  EXPECT(array->size() > 5);

  int splits[] = {0, 1, 3, static_cast<int>(array->size())};
  for (int i = 0; i < 3; i++) {
    auto pinned = array->pinRegion(splits[i], splits[i + 1] - splits[i]);
    for (int j = 0; j < static_cast<int>(pinned.size()); j++) {
      EXPECT(std::abs(static_cast<double>(pinned[j] - j - splits[i])) < kEps);
      pinned[j] *= 2;
    }
  }
  return JNI_TRUE;
}

template <typename JArrayType>
jboolean testPinArrayCritical(alias_ref<jclass>, alias_ref<JArrayType> array) {
  EXPECT(array);

  int n = array->size();
  auto pinned = array->pinCritical();

  for (auto i = 0; i < n; ++i) {
    EXPECT(std::abs(static_cast<double>(pinned[i] - i)) < kEps);
  }

  for (auto i = 0; i < n; ++i) {
    pinned[i] *= 2;
  }

  return JNI_TRUE;
}

jboolean testIndexOutOfBoundsInRegions(alias_ref<jclass>) {
  constexpr auto N = 7;
  constexpr auto TOO_MUCH = 10;
  constexpr auto NEGATIVE = -1;

  auto array = make_int_array(N);

  try {
    auto buf = array->getRegion(TOO_MUCH, N);
    EXPECT(false);
  } catch (JniException& ex) {
  }

  try {
    auto buf = array->getRegion(NEGATIVE, N);
    EXPECT(false);
  } catch (JniException& ex) {
  }

  try {
    auto vec = std::vector<jint>(TOO_MUCH);
    array->setRegion(0, vec.size(), vec.data());
    EXPECT(false);
  } catch (JniException& ex) {
  }

  try {
    auto vec = std::vector<jint>(1);
    array->setRegion(NEGATIVE, vec.size(), vec.data());
    EXPECT(false);
  } catch (JniException& ex) {
  }

  return JNI_TRUE;
}

jboolean TestBooleanArrayIndexing(alias_ref<jobject> self, alias_ref<jbooleanArray> input, jint idx) {
  auto array = input->pin();
  jboolean value = array[idx];
  return value;
}

jint TestIntegerArrayIndexing(alias_ref<jobject> self, alias_ref<jintArray> input, jint idx) {
  auto array = input->pin();
  jint value = array[idx];
  return value;
}

jsize TestIntegerArraySize(alias_ref<jobject> self, alias_ref<jintArray> input) {
  auto array = input->pin();
  jsize size = array.size();
  return size;
}

alias_ref<jintArray> TestIntegerArrayIncrement(alias_ref<jobject> self, alias_ref<jintArray> input) {
  auto array = input->pin();
  for (size_t ii = 0; ii < array.size(); ii++) {
    array[ii]++;
  }
  return input;
}

void TestIntegerArrayMoveAssignment(alias_ref<jobject> self, alias_ref<jintArray> input) {
  auto array = input->pin();
  array[0] = 0;
  array.release();
}

jboolean isPinnedArrayACopy(alias_ref<jobject>, alias_ref<jintArray> input) {
  return input->pin().isCopy();
}

jboolean testCopiedPinnedArray(alias_ref<jobject>, alias_ref<jintArray> input) {
  EXPECT(input->size() > 0);
  input->pin()[0] = 100;
  auto pin = input->pin();
  EXPECT(pin.isCopy());
  EXPECT(pin[0] == input->pin()[0]);

  pin[0] = 200;
  EXPECT(input->pin()[0] == 100);

  pin.commit();
  EXPECT(input->pin()[0] == 200);
  pin[0] = 300;
  EXPECT(input->pin()[0] == 200);
  pin.abort();
  EXPECT(input->pin()[0] == 200);
  pin = input->pin();
  pin[0] = 400;
  pin.release();
  EXPECT(input->pin()[0] == 400);
  return JNI_TRUE;
}

jboolean testNonCopiedPinnedArray(alias_ref<jobject>, alias_ref<jintArray> input) {
  EXPECT(input->size() > 0);
  auto pin = input->pin();
  EXPECT(!pin.isCopy());
  EXPECT(pin[0] == input->pin()[0]);
  pin.commit();
  EXPECT(pin[0] == input->pin()[0]);
  pin[0] = 100;
  EXPECT(pin[0] == input->pin()[0]);
  input->pin()[0] = 200;
  EXPECT(pin[0] == input->pin()[0]);
  pin.abort();
  return JNI_TRUE;
}


void RegisterPrimitiveArrayTests() {
  registerNatives("com/facebook/jni/PrimitiveArrayTests", {
    makeNativeMethod("nativeTestMakeBooleanArray", testMakeBoolArray),
    makeNativeMethod("nativeTestMakeByteArray", testMakeByteArray),
    makeNativeMethod("nativeTestMakeCharArray", testMakeCharArray),
    makeNativeMethod("nativeTestMakeShortArray", testMakeShortArray),
    makeNativeMethod("nativeTestMakeIntArray", testMakeIntArray),
    makeNativeMethod("nativeTestMakeLongArray", testMakeLongArray),
    makeNativeMethod("nativeTestMakeFloatArray", testMakeFloatArray),
    makeNativeMethod("nativeTestMakeDoubleArray", testMakeDoubleArray),

    makeNativeMethod("nativeTestGetSetBooleanArray", testGetSetBooleanArray),
    makeNativeMethod("nativeTestGetSetByteArray", testGetSetArray<jbyteArray>),
    makeNativeMethod("nativeTestGetSetCharArray", testGetSetArray<jcharArray>),
    makeNativeMethod("nativeTestGetSetShortArray", testGetSetArray<jshortArray>),
    makeNativeMethod("nativeTestGetSetIntArray", testGetSetArray<jintArray>),
    makeNativeMethod("nativeTestGetSetLongArray", testGetSetArray<jlongArray>),
    makeNativeMethod("nativeTestGetSetFloatArray", testGetSetArray<jfloatArray>),
    makeNativeMethod("nativeTestGetSetDoubleArray", testGetSetArray<jdoubleArray>),

    makeNativeMethod("nativeTestPinBooleanArray", testPinBooleanArray),
    makeNativeMethod("nativeTestPinByteArray", testPinArray<jbyteArray>),
    makeNativeMethod("nativeTestPinCharArray", testPinArray<jcharArray>),
    makeNativeMethod("nativeTestPinShortArray", testPinArray<jshortArray>),
    makeNativeMethod("nativeTestPinIntArray", testPinArray<jintArray>),
    makeNativeMethod("nativeTestPinLongArray", testPinArray<jlongArray>),
    makeNativeMethod("nativeTestPinFloatArray", testPinArray<jfloatArray>),
    makeNativeMethod("nativeTestPinDoubleArray", testPinArray<jdoubleArray>),

    makeNativeMethod("nativeTestPinByteArrayRegion", testPinArrayRegion<jbyteArray>),
    makeNativeMethod("nativeTestPinCharArrayRegion", testPinArrayRegion<jcharArray>),
    makeNativeMethod("nativeTestPinShortArrayRegion", testPinArrayRegion<jshortArray>),
    makeNativeMethod("nativeTestPinIntArrayRegion", testPinArrayRegion<jintArray>),
    makeNativeMethod("nativeTestPinLongArrayRegion", testPinArrayRegion<jlongArray>),
    makeNativeMethod("nativeTestPinFloatArrayRegion", testPinArrayRegion<jfloatArray>),
    makeNativeMethod("nativeTestPinDoubleArrayRegion", testPinArrayRegion<jdoubleArray>),

    makeNativeMethod("nativeTestPinByteArrayCritical", testPinArrayCritical<jbyteArray>),
    makeNativeMethod("nativeTestPinCharArrayCritical", testPinArrayCritical<jcharArray>),
    makeNativeMethod("nativeTestPinShortArrayCritical", testPinArrayCritical<jshortArray>),
    makeNativeMethod("nativeTestPinIntArrayCritical", testPinArrayCritical<jintArray>),
    makeNativeMethod("nativeTestPinLongArrayCritical", testPinArrayCritical<jlongArray>),
    makeNativeMethod("nativeTestPinFloatArrayCritical", testPinArrayCritical<jfloatArray>),
    makeNativeMethod("nativeTestPinDoubleArrayCritical", testPinArrayCritical<jdoubleArray>),

    makeNativeMethod("nativeTestIndexOutOfBoundsInRegions", testIndexOutOfBoundsInRegions),

    makeNativeMethod("nativeTestBooleanArrayIndexing",
                     TestBooleanArrayIndexing),
    makeNativeMethod("nativeTestIntegerArrayIndexing",
                     TestIntegerArrayIndexing),
    makeNativeMethod("nativeTestIntegerArraySize",
                     TestIntegerArraySize),
    makeNativeMethod("nativeTestIntegerArrayIncrement",
                     TestIntegerArrayIncrement),
    makeNativeMethod("nativeTestIntegerArrayMoveAssignment",
                     TestIntegerArrayMoveAssignment),

    makeNativeMethod("nativeIsPinnedArrayACopy", isPinnedArrayACopy),
    makeNativeMethod("nativeTestCopiedPinnedArray",
                     testCopiedPinnedArray),
    makeNativeMethod("nativeTestNonCopiedPinnedArray",
                     testNonCopiedPinnedArray),
  });
}
