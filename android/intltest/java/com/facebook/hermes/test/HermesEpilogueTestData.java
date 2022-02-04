/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.hermes.test;

import java.nio.charset.Charset;

public class HermesEpilogueTestData {
  private static final String SRC = "function f(){}; f()";
  private static final byte[] hermesBytecode = JSRuntime.compileJavaScript(SRC);

  public static byte[] getBytecode() {
    return hermesBytecode;
  }

  public static byte[] getBytecodeWithEpilogue(String epilogue) {
    byte[] epilogueAsBytes = epilogue.getBytes(Charset.forName("UTF-8"));
    byte[] hermesBytecodeWithEpilogue = new byte[hermesBytecode.length + epilogueAsBytes.length];
    System.arraycopy(hermesBytecode, 0, hermesBytecodeWithEpilogue, 0, hermesBytecode.length);
    System.arraycopy(
        epilogueAsBytes,
        0,
        hermesBytecodeWithEpilogue,
        hermesBytecode.length,
        epilogueAsBytes.length);
    return hermesBytecodeWithEpilogue;
  }
}
