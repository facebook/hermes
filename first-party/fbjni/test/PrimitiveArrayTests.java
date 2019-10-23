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

package com.facebook.jni;

import static org.fest.assertions.api.Assertions.assertThat;
import static org.fest.assertions.api.Assertions.offset;
import static org.junit.Assume.assumeFalse;
import static org.junit.Assume.assumeTrue;

import org.junit.Ignore;
import org.junit.Test;

public class PrimitiveArrayTests extends BaseFBJniTests {

  private static final int MAGIC = 42;

  @Test
  public void testMakeArrays() {
    assertThat(nativeTestMakeBooleanArray(MAGIC).length).isEqualTo(MAGIC);
    assertThat(nativeTestMakeByteArray(MAGIC).length).isEqualTo(MAGIC);
    assertThat(nativeTestMakeCharArray(MAGIC).length).isEqualTo(MAGIC);
    assertThat(nativeTestMakeShortArray(MAGIC).length).isEqualTo(MAGIC);
    assertThat(nativeTestMakeIntArray(MAGIC).length).isEqualTo(MAGIC);
    assertThat(nativeTestMakeLongArray(MAGIC).length).isEqualTo(MAGIC);
    assertThat(nativeTestMakeFloatArray(MAGIC).length).isEqualTo(MAGIC);
    assertThat(nativeTestMakeDoubleArray(MAGIC).length).isEqualTo(MAGIC);
  }

  private static native boolean[] nativeTestMakeBooleanArray(int size);

  private static native byte[] nativeTestMakeByteArray(int size);

  private static native char[] nativeTestMakeCharArray(int size);

  private static native short[] nativeTestMakeShortArray(int size);

  private static native int[] nativeTestMakeIntArray(int size);

  private static native long[] nativeTestMakeLongArray(int size);

  private static native float[] nativeTestMakeFloatArray(int size);

  private static native double[] nativeTestMakeDoubleArray(int size);

  @Test
  public void testGetSetBooleanArray() {
    boolean[] array = {false, true};

    assertThat(nativeTestGetSetBooleanArray(array)).isTrue();
    assertThat(array).isEqualTo(new boolean[] {true, false});
  }

  private static native boolean nativeTestGetSetBooleanArray(boolean[] array);

  @Test
  public void testPinBooleanArray() {
    boolean[] array = {false, true};

    assertThat(nativeTestPinBooleanArray(array)).isTrue();
    assertThat(array).isEqualTo(new boolean[] {true, false});
  }

  private static native boolean nativeTestPinBooleanArray(boolean[] array);

  @Test
  public void testGetSetByteArray() {
    byte[] array = new byte[MAGIC];
    for (int i = 0; i < array.length; ++i) {
      array[i] = (byte) i;
    }

    assertThat(nativeTestGetSetByteArray(array)).isTrue();

    for (int i = 0; i < array.length; ++i) {
      assertThat(array[i]).isEqualTo((byte) (2 * i));
    }
  }

  private static native boolean nativeTestGetSetByteArray(byte[] array);

  @Test
  public void testGetSetCharArray() {
    char[] array = new char[MAGIC];
    for (int i = 0; i < array.length; ++i) {
      array[i] = (char) i;
    }

    assertThat(nativeTestGetSetCharArray(array)).isTrue();

    for (int i = 0; i < array.length; ++i) {
      assertThat(array[i]).isEqualTo((char) (2 * i));
    }
  }

  private static native boolean nativeTestGetSetCharArray(char[] array);

  @Test
  public void testGetSetShortArray() {
    short[] array = new short[MAGIC];
    for (int i = 0; i < array.length; ++i) {
      array[i] = (short) i;
    }

    assertThat(nativeTestGetSetShortArray(array)).isTrue();

    for (int i = 0; i < array.length; ++i) {
      assertThat(array[i]).isEqualTo((short) (2 * i));
    }
  }

  private static native boolean nativeTestGetSetShortArray(short[] array);

  @Test
  public void testGetSetIntArray() {
    int[] intArray = new int[MAGIC];
    for (int i = 0; i < intArray.length; ++i) {
      intArray[i] = i;
    }

    assertThat(nativeTestGetSetIntArray(intArray)).isTrue();

    for (int i = 0; i < intArray.length; ++i) {
      assertThat(intArray[i]).isEqualTo(2 * i);
    }
  }

  private static native boolean nativeTestGetSetIntArray(int[] array);

  @Test
  public void testGetSetLongArray() {
    long[] longArray = new long[MAGIC];
    for (int i = 0; i < longArray.length; ++i) {
      longArray[i] = (long) i;
    }

    assertThat(nativeTestGetSetLongArray(longArray)).isTrue();

    for (int i = 0; i < longArray.length; ++i) {
      assertThat(longArray[i]).isEqualTo(2 * i);
    }
  }

  private static native boolean nativeTestGetSetLongArray(long[] array);

  @Test
  public void testGetSetFloatArray() {
    float[] floatArray = new float[MAGIC];
    for (int i = 0; i < floatArray.length; ++i) {
      floatArray[i] = i;
    }

    assertThat(nativeTestGetSetFloatArray(floatArray)).isTrue();

    for (int i = 0; i < floatArray.length; ++i) {
      assertThat(floatArray[i]).isEqualTo(2 * i, offset(1e-3f));
    }
  }

  private static native boolean nativeTestGetSetFloatArray(float[] array);

  @Test
  public void testGetSetDoubleArray() {
    double[] doubleArray = new double[MAGIC];
    for (int i = 0; i < doubleArray.length; ++i) {
      doubleArray[i] = i;
    }

    assertThat(nativeTestGetSetDoubleArray(doubleArray)).isTrue();

    for (int i = 0; i < doubleArray.length; ++i) {
      assertThat(doubleArray[i]).isEqualTo(2 * i, offset(1e-3));
    }
  }

  private static native boolean nativeTestGetSetDoubleArray(double[] array);

  @Test
  public void testPinByteArray() {
    byte[] array = new byte[MAGIC];
    for (int i = 0; i < array.length; ++i) {
      array[i] = (byte) i;
    }

    assertThat(nativeTestPinByteArray(array)).isTrue();

    for (int i = 0; i < array.length; ++i) {
      assertThat(array[i]).isEqualTo((byte) (2 * i));
    }
  }

  private static native boolean nativeTestPinByteArray(byte[] array);

  @Test
  public void testPinCharArray() {
    char[] array = new char[MAGIC];
    for (int i = 0; i < array.length; ++i) {
      array[i] = (char) i;
    }

    assertThat(nativeTestPinCharArray(array)).isTrue();

    for (int i = 0; i < array.length; ++i) {
      assertThat(array[i]).isEqualTo((char) (2 * i));
    }
  }

  private static native boolean nativeTestPinCharArray(char[] array);

  @Test
  public void testPinShortArray() {
    short[] array = new short[MAGIC];
    for (int i = 0; i < array.length; ++i) {
      array[i] = (short) i;
    }

    assertThat(nativeTestPinShortArray(array)).isTrue();

    for (int i = 0; i < array.length; ++i) {
      assertThat(array[i]).isEqualTo((short) (2 * i));
    }
  }

  private static native boolean nativeTestPinShortArray(short[] array);

  @Test
  public void testPinIntArray() {
    int[] intArray = new int[MAGIC];
    for (int i = 0; i < intArray.length; ++i) {
      intArray[i] = i;
    }

    assertThat(nativeTestPinIntArray(intArray)).isTrue();

    for (int i = 0; i < intArray.length; ++i) {
      assertThat(intArray[i]).isEqualTo(2 * i);
    }
  }

  private static native boolean nativeTestPinIntArray(int[] array);

  @Test
  public void testPinLongArray() {
    long[] longArray = new long[MAGIC];
    for (int i = 0; i < longArray.length; ++i) {
      longArray[i] = (long) i;
    }

    assertThat(nativeTestPinLongArray(longArray)).isTrue();

    for (int i = 0; i < longArray.length; ++i) {
      assertThat(longArray[i]).isEqualTo(2 * i);
    }
  }

  private static native boolean nativeTestPinLongArray(long[] array);

  @Test
  public void testPinFloatArray() {
    float[] floatArray = new float[MAGIC];
    for (int i = 0; i < floatArray.length; ++i) {
      floatArray[i] = (long) i;
    }

    assertThat(nativeTestPinFloatArray(floatArray)).isTrue();

    for (int i = 0; i < floatArray.length; ++i) {
      assertThat(floatArray[i]).isEqualTo(2 * i, offset(1e-3f));
    }
  }

  private static native boolean nativeTestPinFloatArray(float[] array);

  @Test
  public void testPinDoubleArray() {
    double[] doubleArray = new double[MAGIC];
    for (int i = 0; i < doubleArray.length; ++i) {
      doubleArray[i] = (double) i;
    }

    assertThat(nativeTestPinDoubleArray(doubleArray)).isTrue();

    for (int i = 0; i < doubleArray.length; ++i) {
      assertThat(doubleArray[i]).isEqualTo(2 * i, offset(1e-3));
    }
  }

  private static native boolean nativeTestPinDoubleArray(double[] array);

  @Test
  public void testPinByteArrayRegion() {
    byte[] array = new byte[MAGIC];
    for (int i = 0; i < array.length; ++i) {
      array[i] = (byte) i;
    }

    assertThat(nativeTestPinByteArrayRegion(array)).isTrue();

    for (int i = 0; i < array.length; ++i) {
      assertThat(array[i]).isEqualTo((byte) (2 * i));
    }
  }

  private static native boolean nativeTestPinByteArrayRegion(byte[] array);

  @Test
  public void testPinCharArrayRegion() {
    char[] array = new char[MAGIC];
    for (int i = 0; i < array.length; ++i) {
      array[i] = (char) i;
    }

    assertThat(nativeTestPinCharArrayRegion(array)).isTrue();

    for (int i = 0; i < array.length; ++i) {
      assertThat(array[i]).isEqualTo((char) (2 * i));
    }
  }

  private static native boolean nativeTestPinCharArrayRegion(char[] array);

  @Test
  public void testPinShortArrayRegion() {
    short[] array = new short[MAGIC];
    for (int i = 0; i < array.length; ++i) {
      array[i] = (short) i;
    }

    assertThat(nativeTestPinShortArrayRegion(array)).isTrue();

    for (int i = 0; i < array.length; ++i) {
      assertThat(array[i]).isEqualTo((short) (2 * i));
    }
  }

  private static native boolean nativeTestPinShortArrayRegion(short[] array);

  @Test
  public void testPinIntArrayRegion() {
    int[] intArray = new int[MAGIC];
    for (int i = 0; i < intArray.length; ++i) {
      intArray[i] = i;
    }

    assertThat(nativeTestPinIntArrayRegion(intArray)).isTrue();

    for (int i = 0; i < intArray.length; ++i) {
      assertThat(intArray[i]).isEqualTo(2 * i);
    }
  }

  private static native boolean nativeTestPinIntArrayRegion(int[] array);

  @Test
  public void testPinLongArrayRegion() {
    long[] longArray = new long[MAGIC];
    for (int i = 0; i < longArray.length; ++i) {
      longArray[i] = (long) i;
    }

    assertThat(nativeTestPinLongArrayRegion(longArray)).isTrue();

    for (int i = 0; i < longArray.length; ++i) {
      assertThat(longArray[i]).isEqualTo(2 * i);
    }
  }

  private static native boolean nativeTestPinLongArrayRegion(long[] array);

  @Test
  public void testPinFloatArrayRegion() {
    float[] floatArray = new float[MAGIC];
    for (int i = 0; i < floatArray.length; ++i) {
      floatArray[i] = (long) i;
    }

    assertThat(nativeTestPinFloatArrayRegion(floatArray)).isTrue();

    for (int i = 0; i < floatArray.length; ++i) {
      assertThat(floatArray[i]).isEqualTo(2 * i, offset(1e-3f));
    }
  }

  private static native boolean nativeTestPinFloatArrayRegion(float[] array);

  @Test
  public void testPinDoubleArrayRegion() {
    double[] doubleArray = new double[MAGIC];
    for (int i = 0; i < doubleArray.length; ++i) {
      doubleArray[i] = (double) i;
    }

    assertThat(nativeTestPinDoubleArrayRegion(doubleArray)).isTrue();

    for (int i = 0; i < doubleArray.length; ++i) {
      assertThat(doubleArray[i]).isEqualTo(2 * i, offset(1e-3));
    }
  }

  private static native boolean nativeTestPinDoubleArrayRegion(double[] array);

  @Test
  public void testPinByteArrayCritical() {
    byte[] array = new byte[MAGIC];
    for (int i = 0; i < array.length; ++i) {
      array[i] = (byte) i;
    }

    assertThat(nativeTestPinByteArrayCritical(array)).isTrue();

    for (int i = 0; i < array.length; ++i) {
      assertThat(array[i]).isEqualTo((byte) (2 * i));
    }
  }

  private static native boolean nativeTestPinByteArrayCritical(byte[] array);

  @Test
  public void testPinCharArrayCritical() {
    char[] array = new char[MAGIC];
    for (int i = 0; i < array.length; ++i) {
      array[i] = (char) i;
    }

    assertThat(nativeTestPinCharArrayCritical(array)).isTrue();

    for (int i = 0; i < array.length; ++i) {
      assertThat(array[i]).isEqualTo((char) (2 * i));
    }
  }

  private static native boolean nativeTestPinCharArrayCritical(char[] array);

  @Test
  public void testPinShortArrayCritical() {
    short[] array = new short[MAGIC];
    for (int i = 0; i < array.length; ++i) {
      array[i] = (short) i;
    }

    assertThat(nativeTestPinShortArrayCritical(array)).isTrue();

    for (int i = 0; i < array.length; ++i) {
      assertThat(array[i]).isEqualTo((short) (2 * i));
    }
  }

  private static native boolean nativeTestPinShortArrayCritical(short[] array);

  @Test
  public void testPinIntArrayCritical() {
    int[] intArray = new int[MAGIC];
    for (int i = 0; i < intArray.length; ++i) {
      intArray[i] = i;
    }

    assertThat(nativeTestPinIntArrayCritical(intArray)).isTrue();

    for (int i = 0; i < intArray.length; ++i) {
      assertThat(intArray[i]).isEqualTo(2 * i);
    }
  }

  private static native boolean nativeTestPinIntArrayCritical(int[] array);

  @Test
  public void testPinLongArrayCritical() {
    long[] longArray = new long[MAGIC];
    for (int i = 0; i < longArray.length; ++i) {
      longArray[i] = (long) i;
    }

    assertThat(nativeTestPinLongArrayCritical(longArray)).isTrue();

    for (int i = 0; i < longArray.length; ++i) {
      assertThat(longArray[i]).isEqualTo(2 * i);
    }
  }

  private static native boolean nativeTestPinLongArrayCritical(long[] array);

  @Test
  public void testPinFloatArrayCritical() {
    float[] floatArray = new float[MAGIC];
    for (int i = 0; i < floatArray.length; ++i) {
      floatArray[i] = (long) i;
    }

    assertThat(nativeTestPinFloatArrayCritical(floatArray)).isTrue();

    for (int i = 0; i < floatArray.length; ++i) {
      assertThat(floatArray[i]).isEqualTo(2 * i, offset(1e-3f));
    }
  }

  private static native boolean nativeTestPinFloatArrayCritical(float[] array);

  @Test
  public void testPinDoubleArrayCritical() {
    double[] doubleArray = new double[MAGIC];
    for (int i = 0; i < doubleArray.length; ++i) {
      doubleArray[i] = (double) i;
    }

    assertThat(nativeTestPinDoubleArrayCritical(doubleArray)).isTrue();

    for (int i = 0; i < doubleArray.length; ++i) {
      assertThat(doubleArray[i]).isEqualTo(2 * i, offset(1e-3));
    }
  }

  private static native boolean nativeTestPinDoubleArrayCritical(double[] array);

  @Test
  public void testIndexOutOfBoundsInRegions() {
    assertThat(nativeTestIndexOutOfBoundsInRegions()).isTrue();
  }

  private static native boolean nativeTestIndexOutOfBoundsInRegions();

  @Test
  public void testBooleanArrayIndexing() {
    boolean[] array = {true, true, false, true, false};
    for (int ii = 0; ii < 5; ii++) {
      assertThat(nativeTestBooleanArrayIndexing(array, ii)).isEqualTo(array[ii]);
    }
  }

  private native boolean nativeTestBooleanArrayIndexing(boolean[] array, int idx);

  @Test
  public void testIntegerArrayIndexing() {
    int[] array = {0, 1, 2, 3, 4};
    for (int ii = 0; ii < 5; ii++) {
      assertThat(nativeTestIntegerArrayIndexing(array, ii)).isEqualTo(array[ii]);
    }
  }

  private native int nativeTestIntegerArrayIndexing(int[] array, int idx);

  @Test
  public void testIntegerArraySize() {
    int[] array = {0, 1, 2, 3, 4};
    assertThat(nativeTestIntegerArraySize(array)).isEqualTo(array.length);
  }

  private native int nativeTestIntegerArraySize(int[] array);

  @Test
  public void testIntegerArrayIncrement() {
    int[] array = {0, 1, 2, 3, 4};
    array = nativeTestIntegerArrayIncrement(array);
    for (int ii = 0; ii < 5; ii++) {
      assertThat(array[ii]).isEqualTo(ii + 1);
    }
  }

  private native int[] nativeTestIntegerArrayIncrement(int[] array);

  @Test
  public void testIntegerArrayMoveAssignment() {
    int[] array = {0, 1, 2, 3, 4};
    nativeTestIntegerArrayMoveAssignment(array);
    assertThat(array[0]).isEqualTo(0);
  }

  private native void nativeTestIntegerArrayMoveAssignment(int[] array);

  // On ART, a large array will be placed in the large heap. Arrays here are
  // non-movable and so the vm pins them in-place. A small array will be a
  // movable object and thus the pinned array will be a copy.
  // On Dalvik, all pinned arrays are in-place.

  @Test
  @Ignore("Flakey Test. See t8845133")
  public void testCopiedPinnedArray() {
    int[] array = new int[100];
    assumeTrue(nativeIsPinnedArrayACopy(array));
    assertThat(nativeTestCopiedPinnedArray(array)).isTrue();
  }

  @Test
  public void testNonCopiedPinnedArray() {
    int[] array = new int[1000000];
    assumeFalse(nativeIsPinnedArrayACopy(array));
    assertThat(nativeTestNonCopiedPinnedArray(array)).isTrue();
  }

  private native boolean nativeIsPinnedArrayACopy(int[] array);

  private native boolean nativeTestCopiedPinnedArray(int[] array);

  private native boolean nativeTestNonCopiedPinnedArray(int[] array);
}
