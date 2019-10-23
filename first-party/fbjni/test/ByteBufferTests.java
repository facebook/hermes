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

  @Test
  public void testByteBufferOrder() {
    ByteBuffer buffer = nativeByteBufferOrder();
    assertThat(buffer.getInt()).isEqualTo(1);
  }

  public native ByteBuffer nativeByteBufferOrder();
}
