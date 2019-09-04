// Copyright 2004-present Facebook. All Rights Reserved.

package com.facebook.jni;

import com.facebook.soloader.SoLoader;
import org.junit.BeforeClass;
import org.junit.Rule;
import org.junit.rules.ExpectedException;

public class BaseFBJniTests {
  @Rule public ExpectedException thrown = ExpectedException.none();

  @BeforeClass
  public static void setup() {
    // Explicitly load fbjni to ensure that its JNI_OnLoad is run.
    SoLoader.loadLibrary("fbjni");
    SoLoader.loadLibrary("fbjni-tests");
  }
}
