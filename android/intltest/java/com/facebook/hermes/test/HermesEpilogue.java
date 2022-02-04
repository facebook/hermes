/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.hermes.test;

import android.annotation.SuppressLint;
import com.facebook.jni.HybridData;
import com.facebook.proguard.annotations.DoNotStrip;
import com.facebook.soloader.SoLoader;

public class HermesEpilogue implements AutoCloseable {
  static {
    SoLoader.loadLibrary("jsijniepi");
  }

  @DoNotStrip
  @SuppressLint("NotAccessedPrivateField")
  private final HybridData mHybridData;

  /**
   * @param bytecode hermes bytecode, note that no validation is done.
   * @return Any byte traling the hermes bytecode data in bytes.
   */
  public static native byte[] getHermesBytecodeMetadata(byte[] bytecode);

  /** Same as getHermesBytecodeMetadata but starting from a file. */
  public static native byte[] getHermesBCFileMetadata(String filename);

  private HermesEpilogue(HybridData hybridData) {
    mHybridData = hybridData;
  }

  /**
   * Explicitly deallocate the Runtime. After a particular instance of the Hermes Runtime has been
   * closed, any other calls to it will result in a {@link NullPointerException}.
   */
  @Override
  public void close() {
    mHybridData.resetNative();
  }
}
