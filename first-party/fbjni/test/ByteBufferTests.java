// Copyright 2004-present Facebook. All Rights Reserved.

package com.facebook.jni;

import static org.fest.assertions.api.Assertions.assertThat;

import java.nio.Buffer;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import org.junit.Test;

public class ByteBufferTests extends BaseFBJniTests {
  @Test
  public void testDirectByteBuffer() {
    assertThat(nativeTestDirectByteBuffer()).isTrue();
  }

  public static native boolean nativeTestDirectByteBuffer();

  @Test
  public void testEmptyDirectByteBuffer() {
    assertThat(nativeTestEmptyDirectByteBuffer()).isTrue();
  }

  public static native boolean nativeTestEmptyDirectByteBuffer();

  @Test
  public void testRewindBuffer() {
    assertThat(nativeTestRewindBuffer()).isTrue();
  }

  public native boolean nativeTestRewindBuffer();

  @Test
  public void testAllocateDirect() {
    ByteBuffer buffer = nativeAllocateDirect(5);
    assertThat(buffer.isDirect()).isTrue();
    assertThat(buffer.capacity()).isEqualTo(5);
  }

  public native ByteBuffer nativeAllocateDirect(int size);

  // called from native
  public static void writeBytes(ByteBuffer dest, byte a, byte b, byte c, byte d) {
    dest.put(a).put(b).put(c).put(d);
  }

  @Test
  public void testFloatBuffer() {
    final int BUFFER_COUNT = 5;
    final int FLOAT_SIZE = 4;
    FloatBuffer buffer =
        ByteBuffer.allocateDirect(BUFFER_COUNT * FLOAT_SIZE)
            .order(ByteOrder.nativeOrder())
            .asFloatBuffer();
    buffer.put(1f);
    buffer.put(2f);
    buffer.put(2.5f);
    buffer.put(2.75f);
    buffer.put(3f);
    assertThat(nativeTestFloatBuffer(buffer)).isTrue();
  }

  public native boolean nativeTestFloatBuffer(Buffer buffer);
}
