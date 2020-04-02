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

import java.nio.ByteBuffer;
import java.nio.channels.ReadableByteChannel;
import java.util.Arrays;
import org.junit.Test;

public class ReadableByteChannelTests extends BaseFBJniTests {
  private static final byte[] data = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
  };

  @Test
  public void testSmallRead() {
    byte[] testData = Arrays.copyOf(data, 8);
    ReadableByteChannel channel = new TestChannel(testData);
    assertThat(nativeTestSmallRead(channel, testData)).isTrue();
  }

  public static native boolean nativeTestSmallRead(ReadableByteChannel channel, byte[] data);

  @Test
  public void testReadToBufferCapacity() {
    ReadableByteChannel channel = new TestChannel(data);
    assertThat(nativeTestReadToBufferCapacity(channel, data)).isTrue();
  }

  public static native boolean nativeTestReadToBufferCapacity(
      ReadableByteChannel channel, byte[] data);

  @Test
  public void testConsumeChannel() {
    ReadableByteChannel channel = new TestChannel(data);
    assertThat(nativeTestConsumeChannel(channel, data)).isTrue();
  }

  public static native boolean nativeTestConsumeChannel(ReadableByteChannel channel, byte[] data);

  @Test
  public void testConsumeChannelIteratively() {
    ReadableByteChannel channel = new TestChannel(data);
    assertThat(nativeTestConsumeChannelIteratively(channel, data)).isTrue();
  }

  public static native boolean nativeTestConsumeChannelIteratively(
      ReadableByteChannel channel, byte[] data);

  private static class TestChannel implements ReadableByteChannel {
    private final byte[] data;
    private int offset = 0;

    TestChannel(byte[] data) {
      this.data = data;
    }

    @Override
    public int read(ByteBuffer buffer) {
      if (offset >= data.length) {
        return -1;
      }

      int n = Math.min(buffer.remaining(), data.length - offset);
      int start = offset;
      offset += n;
      buffer.put(Arrays.copyOfRange(data, start, offset));
      return n;
    }

    @Override
    public boolean isOpen() {
      return true;
    }

    @Override
    public void close() {}
  }
}
