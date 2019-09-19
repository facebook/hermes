// Copyright 2004-present Facebook. All Rights Reserved.

package com.facebook.jni;

import com.facebook.soloader.nativeloader.NativeLoader;
import com.facebook.soloader.nativeloader.SystemDelegate;
import org.junit.BeforeClass;
import org.junit.Rule;
import org.junit.rules.ExpectedException;

public class BaseFBJniTests {
  @Rule public ExpectedException thrown = ExpectedException.none();

  @BeforeClass
  public static void setup() {
    if (!NativeLoader.isInitialized()) {
      NativeLoader.init(new SystemDelegate());
    }
    // Explicitly load fbjni to ensure that its JNI_OnLoad is run.
    NativeLoader.loadLibrary("fbjni");
    NativeLoader.loadLibrary("fbjni-tests");
  }
}
