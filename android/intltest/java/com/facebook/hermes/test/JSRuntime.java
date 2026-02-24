/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.hermes.test;

import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import androidx.test.InstrumentationRegistry;
import com.facebook.jni.HybridData;
import com.facebook.proguard.annotations.DoNotStrip;
import com.facebook.soloader.SoLoader;
import java.nio.charset.Charset;

// This class is only used by tests.  If they fail on older APIs, I
// don't care.
@TargetApi(19)
public class JSRuntime implements AutoCloseable {
  static {
    SoLoader.init(InstrumentationRegistry.getContext(), false);
    try {
      // The OSS build produces two .so files, so both need
      // to be loaded to initialize JNI context.
      SoLoader.loadLibrary("hermes");
    } catch (UnsatisfiedLinkError e) {
      // FB Internal build doesn't produce a separate libhermes,
      // so just ignore this failure.
    }
    SoLoader.loadLibrary("jsijni");
  }

  @DoNotStrip
  @SuppressLint("NotAccessedPrivateField")
  private final HybridData mHybridData;

  /**
   * Create a {@link JSRuntime} that evaluates JS using Hermes.
   *
   * @param shouldRecordGCStats indicates whether or not the runtime should collect GC statistics
   *     which can be accessed through calls to {@link getRecordedGCStats}. This parameter is
   *     optional, and defaults to {@code false}.
   */
  public static native JSRuntime makeHermesRuntime(boolean shouldRecordGCStats);

  public static JSRuntime makeHermesRuntime() {
    return makeHermesRuntime(/* shouldRecordGCStats */ false);
  }

  /**
   * Create a {@link JSRuntime} that evaluates JS using Hermes, with the given initial and maximum
   * heap sizes.
   *
   * @param shouldRecordGCStats indicates whether or not the runtime should collect GC statistics
   *     which can be accessed through calls to {@link getRecordedGCStats}. This parameter is
   *     optional, and defaults to {@code false}.
   */
  public static native JSRuntime makeHermesRuntimeWithHeapSpec(
      long initHeapSize, long maxHeapSize, boolean shouldRecordGCStats);

  public static JSRuntime makeHermesRuntimeWithHeapSpec(long initHeapSize, long maxHeapSize) {
    return makeHermesRuntimeWithHeapSpec(
        initHeapSize, maxHeapSize, /* shouldRecordGCStats */ false);
  }

  /** Compile JS source to bytecode. NOTE: This only creates hermes bytecode. */
  public static native byte[] compileJavaScript(byte[] code);

  public static byte[] compileJavaScript(String code) {
    return compileJavaScript(code.getBytes(Charset.forName("UTF-8")));
  }

  private JSRuntime(HybridData hybridData) {
    mHybridData = hybridData;
  }

  /**
   * Evaluate JS in the runtime managed by this object.
   *
   * @param code the JS source code to evaluate.
   */
  public native void evaluateJavaScript(byte[] code);

  /**
   * Evaluate JS in the runtime managed by this object.
   *
   * @param code the JS source code to evaluate, as a String.
   */
  public void evaluateJavaScript(String code) {
    evaluateJavaScript(code.getBytes(Charset.forName("UTF-8")));
  }

  /**
   * Evaluate JS in the runtime managed by this object.
   *
   * @param codeFile the path to the file containing JS source or bytecode.
   */
  public native void evaluateJavaScriptFile(String codeFile);

  /**
   * It is assumed that the property is an int.
   *
   * @param propertyName name of the property on the global object
   */
  public native int getGlobalNumberProperty(String propertyName);

  /**
   * Set a int type property.
   *
   * @param propertyName name of the property on the global object
   * @param val value of the property
   */
  public native void setGlobalNumberProperty(String propertyName, int val);

  /**
   * Set a String type property.
   *
   * @param propertyName name of the property on the global object
   * @param val value of the property
   */
  public native void setGlobalStringProperty(String propertyName, String val);

  /**
   * Call a Java Script function by name
   *
   * @param functionName the name of the function
   */
  public native void callFunction(String functionName);

  /**
   * It is assumed that the property is a String.
   *
   * @param propertyName name of the property on the global object
   */
  public native String getGlobalStringProperty(String propertyName);

  /**
   * (Hermes-only API)
   *
   * @return the GC statistics that have been collected so far, as a JSON-encoded string, if there
   *     are any.
   */
  public native String getRecordedGCStats();

  /**
   * Explicitly deallocate the Runtime. After a particular instance of the Hermes Runtime has been
   * closed, any other calls to it will result in a {@link NullPointerException}.
   */
  @Override
  public void close() {
    mHybridData.resetNative();
  }
}
